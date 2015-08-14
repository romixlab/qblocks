#include "blockinfo.h"
#include "blockinfo_p.h"
#include "block.h"
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

    block->plugin();
    state = BlockInfo::LOADED;
    return true;
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
{ }

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




