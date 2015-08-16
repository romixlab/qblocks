#ifndef BLOCK_P_H
#define BLOCK_P_H

#include "block.h"

namespace Blocks {

class BlockInfo;
class BlockData
{
public:
    BlockData() :
        context(0)
    { }
    QQmlContext *context;
    BlockInfo *info;
    QHash<QString, QObject *> objects;
    QList<QObject *> objects_list;
};

}

#endif // BLOCK_P_H

