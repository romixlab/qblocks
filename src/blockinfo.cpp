#include "blockinfo.h"
#include "blockinfo_p.h"
#include "block.h"
#include "block_p.h"
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>

using namespace Blocks;

bool BlockInfoData::setFileName(const QString &fileName)
{
    loader.setFileName(fileName);
    if (loader.fileName().isEmpty()) {
        qWarning() << "Blocks cannot find" << fileName << loader.errorString();
        return false;
    }
    if (readMetaData(loader.metaData())) {
        state = BlockInfo::METADATA_READED;
        return true;
    }
    return false;
}

bool BlockInfoData::readMetaData(const QJsonObject &metaData)
{
    QJsonValue value;
    value = metaData.value("IID");
    if (!value.isString()) {
        qWarning() << "Not a plugin (no IID found)";
        return false;
    }

    value = metaData.value("MetaData");
    if (!value.isObject()) {
        qWarning() << "Plugin meta data not found";
        return false;
    }
    QJsonObject pluginInfo = value.toObject();

    value = pluginInfo.value("provides");
    if (!value.isString()) {
        qWarning() << "Plugin doesn't provide anything (check \"provides\" field)";
        return false;
    }
    provides = BlockVersion(value.toString());

    if (!provides.isValid()) {
        qWarning() << "Plugin provides invalid version";
        return false;
    }

    value = pluginInfo.value("provides_compat");
    if (!value.isString()) {
        qWarning() << "Plugin doesn't provide compatibility version "
                      "(check \"provides_compat\" field)";
        return false;
    }
    provides_compat = BlockVersion(value.toString());

    if (!provides_compat.isValid()) {
        qWarning() << "Plugin provides invalid version";
        return false;
    }

    value = pluginInfo.value("needs");
    if (value.isArray()) {
        QJsonArray deps = value.toArray();
        foreach(const QJsonValue &d, deps) {
            if (d.isString()) {
                BlockVersion version(d.toString());
                if (version.isValid())
                    needs.append(version);
            }
        }
    }

    value = pluginInfo.value("after");
    if (value.isString()) {
        after << value.toString();
    } else {
        QJsonArray afters = value.toArray();
        foreach(const QJsonValue &a, afters)
            if (a.isString())
                after << a.toString();
    }
    category = pluginInfo.value("category").toString();

    foreach (const BlockVersion &v, needs) {
        qDebug() << v.name() << v.versionMajor() << v.versionMinor() << v.versionRevision();
    }
    return true;
}

bool BlockInfoData::load()
{
    if (!loader.load()) {
        qWarning() << "Cannot load Block" << loader.errorString();
        state = BlockInfo::FAILED;
        return false;
    }

    block = qobject_cast<Block *>(loader.instance());
    if (!block) {
        qWarning() << "Plugin must derive from Blocks::Block";
        state = BlockInfo::FAILED;
        return false;
    }

    block->d->info = q;
    block->plugin();
    foreach (BlockInfo *dependency, dependency_blocks)
        dependency->block()->plugged(block);

    state = BlockInfo::LOADED;
    return true;
}

void BlockInfoData::addChildBlock(BlockInfo *blockInfo)
{
    //qDebug() << "addChildBlock to" << q->version().name() << blockInfo->version().name() << blockInfo->category() << blockInfo->after();
    category_t *category_node = categories.value(blockInfo->category(), 0);
    if (category_node) { // category exists
        category_node->blocks.append(blockInfo);
        foreach (QString after, blockInfo->after())
            if (!category_node->after.contains(after))
                category_node->after.append(after);
    } else {
        category_node = new category_t;
        category_node->resolved = false;
        category_node->after = blockInfo->after();
        category_node->blocks.append(blockInfo);
        categories.insert(blockInfo->category(), category_node);
    }
}

QList<BlockInfo *> BlockInfoData::resolvedChildBlocks()
{
    if (!resolved_childs.isEmpty())
        return resolved_childs;

    QHashIterator<QString, category_t *> it(categories);
    while (it.hasNext()) { // populate child_categories of each category
        it.next();
        category_t *category_node = it.value();
        foreach(const QString &after, category_node->after) {
            if (!categories.contains(after)) {
                qWarning() << "Category" << after << "required by following Blocks, doesn't exist.";
                qWarning() << "Try to disable one of these Blocks, so that the rest can be loaded.";
                QDebug dbg = qWarning();
                foreach (BlockInfo *blockInfo, category_node->blocks)
                    dbg << blockInfo->version().name();
            } else {
                categories[after]->child_categories.append(it.key());
            }
        }
    }

    it.toFront();
    while (it.hasNext()) { // recursively resolve dependencies
        it.next();
        resolve_category(it.key());
    }

//    qDebug() << "--------------";
//    qDebug() << "resolved_childs of" << q->version().name();
//    foreach (BlockInfo *blockInfo, resolved_childs)
//        qDebug() << blockInfo->version().name();
//    qDebug() << "--------------";

    return resolved_childs;
}

void BlockInfoData::resolve_category(const QString &category)
{
    category_t *category_node = categories[category];
    foreach (const QString &after, category_node->after)
        if (!categories[after]->resolved)
            resolve_category(after);

    category_node->resolved = true;
    foreach (BlockInfo *child, category_node->blocks)
        if (!resolved_childs.contains(child)) // only unique
            resolved_childs.append(child);

    foreach (const QString &child, category_node->child_categories)
        if (!categories[child]->resolved)
            resolve_category(child);
}

BlockVersion::BlockVersion()
{ }

BlockVersion::BlockVersion(const QString &versionString)
{
    QRegExp rx("(\\w+)-(\\d+).(\\d+).(\\d+)");
    if (rx.indexIn(versionString) != -1) {
        m_name = rx.cap(1);
        bool ok = true, result;
        m_major = rx.cap(2).toInt(&result);
        ok &= result;
        m_minor = rx.cap(3).toInt(&result);
        ok &= result;
        m_revision = rx.cap(4).toInt(&result);
        ok &= result;
        if (!ok)
            m_name = QString();
    }
}

bool BlockVersion::isValid() const
{
    return !m_name.isEmpty();
}

QString BlockVersion::name() const
{
    return m_name;
}

quint8 BlockVersion::versionMajor() const
{
    return m_major;
}

quint8 BlockVersion::versionMinor() const
{
    return m_minor;
}

quint8 BlockVersion::versionRevision() const
{
    return m_revision;
}

bool BlockVersion::operator==(const BlockVersion &other) const
{
    return ( m_major    == other.m_major) &&
            (m_minor    == other.m_minor) &&
            (m_revision == other.m_revision);
}

bool BlockVersion::operator <(const BlockVersion &other) const
{
    if (m_major < other.m_major)
        return true;
    if (m_minor < other.m_minor)
        return true;
    if (m_revision < other.m_revision)
        return true;
    return false;
}

BlockInfo::BlockInfo() :
    d(new BlockInfoData)
{
    d->q = this;
}

BlockInfo::~BlockInfo()
{
    delete d;
}

Block *BlockInfo::block() const
{
    return d->block;
}

bool BlockInfo::provides(const BlockVersion &version) const
{
    if (version.name() != d->provides.name())
        return false;
    return (version < d->provides || version == d->provides)
            && !(version < d->provides_compat);
}

bool BlockInfo::resolveDependencies(const QList<BlockInfo *> &blocks)
{
    d->state = BlockInfo::RESOLVED;
    foreach (const BlockVersion &dependency, d->needs) {
        BlockInfo *found = 0;
        foreach (BlockInfo *block, blocks) {
            if (block->provides(dependency)) {
                found = block;
                break;
            }
        }
        if (found == 0) {
            d->state = BlockInfo::FAILED;
            return false;
        }
        d->dependency_blocks.push_back(found);
        found->d->addChildBlock(this);
    }

    return true;
}

QList<BlockInfo *> BlockInfo::needs() const
{
    return d->dependency_blocks;
}

BlockInfo::State BlockInfo::state() const
{
    return d->state;
}

BlockVersion BlockInfo::version() const
{
    return d->provides;
}

QStringList BlockInfo::after() const
{
    return d->after;
}

QString BlockInfo::category() const
{
    return d->category;
}




