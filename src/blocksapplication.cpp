#include "blocksapplication.h"
#include "block.h"
#include "block_p.h"
#include "blockinfo.h"
#include "blockinfo_p.h"

#include <QDir>

#include <QDebug>


class Blocks::ApplicationData {
public:
    void checkAndAppendToLoadQueue(BlockInfo *blockInfo);
    QList<BlockInfo *> load_queue;
    QList<BlockInfo *> blocks;

    QHash<QString, QObject *> objects;
    QList<QObject *> objects_list;
    QQmlEngine *engine;
};

using namespace Blocks;


void ApplicationData::checkAndAppendToLoadQueue(BlockInfo *blockInfo)
{
    qDebug() << "Check and append to queue" << blockInfo->version().name();
    if (load_queue.contains(blockInfo))
        return;
    foreach (BlockInfo *need, blockInfo->needs()) {
        if (need->state() != BlockInfo::RESOLVED) {
            qWarning() << "Dependency of ... not resolved";
            return;
        }
        if (!load_queue.contains(need))
            checkAndAppendToLoadQueue(need);
    }
    if (blockInfo->state() != BlockInfo::RESOLVED) {
        qWarning() << "Not resolved ...";
        return;
    }
    // search for last parent of this block in queue and insert after
    int lastParentIdx = 0;
    foreach (BlockInfo *parent, blockInfo->needs()) {
        int idx = load_queue.indexOf(parent);
        if (idx > lastParentIdx)
            lastParentIdx = idx;
    }
    load_queue.insert(lastParentIdx + 1, blockInfo);
}

Application *Application::self = 0;

Application::Application(int &argc, char **argv) :
    QApplication(argc, argv), d(new ApplicationData)
{
    self = this;
}

Application::~Application()
{
    delete d;
}


Block *Application::loadBlock(const QString &name)
{
    QPluginLoader loader(name);
    loader.load();
    if (!loader.isLoaded())
        qWarning() << "Plugin load failed" << loader.errorString();
    Block *block = qobject_cast<Block *>(loader.instance());
    if (!block)
        return 0;
    block->plugin();
    return block;
}

void Application::loadBlocks(const QString &path)
{
    QStringList pluginFiles;
    const QDir dir(path);
    const QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
    foreach (QFileInfo file, files) {
        const QString filePath = file.absoluteFilePath();
        if (QLibrary::isLibrary(filePath))
            pluginFiles.append(filePath);
    }

    QList<BlockInfo *> blocks;
    foreach (const QString &pluginFile, pluginFiles) {
        BlockInfo *blockInfo = new BlockInfo;
        if (!blockInfo->d->setFileName(pluginFile)) {
            delete blockInfo;
            continue;
        }
        blocks.append(blockInfo);
    }

    foreach(BlockInfo *blockInfo, blocks)
        blockInfo->resolveDependencies(blocks);
    foreach(BlockInfo *blockInfo, blocks)
        d->checkAndAppendToLoadQueue(blockInfo);
    foreach(BlockInfo *blockInfo, d->load_queue) {
        qDebug() << "Loading" << blockInfo->version().name();
        if (blockInfo->d->load())
            d->blocks.append(blockInfo);
    }
    d->load_queue.clear();
}

QObject *Application::object(const QString &path) const
{
    foreach (BlockInfo *blockInfo, d->blocks) {
        QObject *object = blockInfo->block()->object(path);
        if (object)
            return object;
    }
    return 0;
}

QList<QObject *> Application::objects() const
{
    QList<QObject *> objects;
    foreach (BlockInfo *blockInfo, d->blocks)
        objects.append(blockInfo->block()->objects());
    return objects;
}

void Application::setQmlEngine(QQmlEngine *engine)
{
    d->engine = engine;
}

QQmlEngine *Application::qmlEngine() const
{
    return d->engine;
}


