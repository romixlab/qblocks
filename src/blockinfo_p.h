#ifndef BLOCKINFO_P_H
#define BLOCKINFO_P_H

#include "blockinfo.h"
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
    BlockVersion provides;
    BlockVersion provides_compat;
    QList<BlockVersion> needs;
    Block *block;
    QList<BlockInfo *> dependency_blocks;

    friend class BlocksApplication;
};

}

#endif // BLOCKINFO_P_H

