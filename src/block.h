#ifndef BLOCK_H
#define BLOCK_H

#include "blocks_global.h"

#include <QObject>
#include <QUrl>
#include <QQmlComponent>

namespace Blocks {

class BlockData;
class BLOCKSSHARED_EXPORT Block : public QObject
{
    Q_OBJECT
public:
    Block(QObject *parent = 0);
    ~Block();

    virtual void plugin() = 0;
    virtual void plugout() = 0;
    virtual void plugged(Block *child);

    QObject *loadQml(const QUrl &url);

private slots:
    void componentstatusChanged(QQmlComponent::Status status);
private:
    BlockData *d;
};

} // Blocks

#endif // BLOCK_H
