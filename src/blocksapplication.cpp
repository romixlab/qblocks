#include "blocksapplication.h"
#include "block.h"
#include "block_p.h"
#include "blockinfo.h"
#include "blockinfo_p.h"

#include <QDir>

#include <QDebug>


class Blocks::ApplicationData {
public:
    void load_blocks(const QList<BlockInfo *> &blocks);
    BlockInfo * root_block;
    QList<BlockInfo *> blocks;

    QHash<QString, QObject *> objects;
    QList<QObject *> objects_list;
    QQmlEngine *engine;
};

using namespace Blocks;


void ApplicationData::load_blocks(const QList<BlockInfo *> &blocks)
{
    foreach (BlockInfo *blockInfo, blocks) {
        if (blockInfo->d->load()) {
            qDebug() << blockInfo->version().name() << "loaded, loading childs...";
            load_blocks(blockInfo->d->resolvedChildBlocks());
            this->blocks.append(blockInfo);
        }
    }
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
    d->root_block = new BlockInfo;
    foreach(BlockInfo *blockInfo, blocks)
        if (blockInfo->d->dependency_blocks.length() == 0)
            d->root_block->d->addChildBlock(blockInfo);
    d->load_blocks(d->root_block->d->resolvedChildBlocks());
}

QObject *Application::object(const QString &path) const
{
    BlockInfo *currentBlock = d->root_block;
    QStringList pathSplitted = path.split(".");
    for (int i = 0; i < pathSplitted.length() - 1; ++i) {
        QString blockName = pathSplitted[i];
        foreach (BlockInfo *childBlock, currentBlock->d->resolvedChildBlocks()) {
            if (childBlock->version().name() == blockName) {
                currentBlock = childBlock;
                break;
            }
            qDebug() << "Application::object" << path << "not found:" << blockName;
            return 0;
        }
    }

    return currentBlock->block()->object(pathSplitted.last());
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




