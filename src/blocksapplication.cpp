#include "blocksapplication.h"
#include "block.h"
#include "blockinfo.h"
#include "blockinfo_p.h"

#include <QDir>

#include <QDebug>


class Blocks::ApplicationData {
public:
    void checkAndAppendToLoadQueue(BlockInfo *blockInfo);
    QList<BlockInfo *> load_queue;

    QHash<QString, QObject *> objects;
    QList<QObject *> objects_list;
    QQmlEngine *engine;
};

using namespace Blocks;


void ApplicationData::checkAndAppendToLoadQueue(BlockInfo *blockInfo)
{
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
    load_queue.push_back(blockInfo);
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

    qDebug() << pluginFiles;
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
        blockInfo->d->load();
    }
}

void Application::addObject(const QString &path, QObject *object)
{
    if (d->objects.contains(path)) {
        qDebug() << "addObject: already has" << path;
        d->objects_list.removeAll(d->objects[path]);
    }
    d->objects.insert(path, object);
    d->objects_list.push_back(object);

    //qDebug() << "Objects" << d->objects_list << d->objects;
}

QObject *Application::object(const QString &path) const
{
    //qDebug() << "requested object, path:" << path << d->objects.value(path, 0);
    return d->objects.value(path, 0);
}

QList<QObject *> Application::objects() const
{
    return d->objects_list;
}

void Application::setQmlEngine(QQmlEngine *engine)
{
    d->engine = engine;
}

QQmlEngine *Application::qmlEngine() const
{
    return d->engine;
}


