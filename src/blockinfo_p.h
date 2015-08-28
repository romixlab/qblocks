#ifndef BLOCKINFO_P_H
#define BLOCKINFO_P_H

#include "blockinfo.h"

#include <QStringList>
#include <QPluginLoader>
#include <QJsonObject>

namespace Blocks {
    class BlocksApplication;
    class Block;

class BlockInfoData {
public:
    BlockInfoData() :
        state(BlockInfo::INVALID) { }

    BlockInfo *q;

    bool setFileName(const QString &fileName);
    bool readMetaData(const QJsonObject &metaData);
    bool load();

    BlockInfo::State state;
    QPluginLoader loader;

    // binary dependencies
    BlockVersion provides;
    BlockVersion provides_compat;
    QList<BlockVersion> needs;
    Block *block;
    QList<BlockInfo *> dependency_blocks;

    // category dependencies
    QStringList after;
    QString category;
    typedef struct {
        QStringList child_categories;
        QStringList after;
        QList<BlockInfo *> blocks;
        bool resolved;
    } category_t;
    QHash<QString, category_t *> categories;
    void addChildBlock(BlockInfo *blockInfo);
    QList<BlockInfo *> resolved_childs;
    QList<BlockInfo *> resolvedChildBlocks();
    void resolve_category(const QString &category);

    friend class BlocksApplication;
};

}

#endif // BLOCKINFO_P_H

