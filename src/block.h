#ifndef BLOCK_H
#define BLOCK_H

#include "blocks_global.h"

#include <QObject>
#include <QUrl>
#include <QQmlComponent>

namespace Blocks {

class BlockInfo;
class BlockInfoData;
class BlockData;
class BLOCKSSHARED_EXPORT Block : public QObject
{
    Q_OBJECT
public:
    Block(QObject *parent = 0);
    virtual ~Block();

    /**
     * @brief plugin called after all of this Block dependencies are plugged in (plugin called).
     */
    virtual void plugin() = 0;
    /**
     * @brief plugout called in reverse order with respect to plugin.
     */
    virtual void plugout() = 0;
    /**
     * @brief plugged called when child Block are plugged in (plugin called).
     * @param child
     */
    virtual void plugged(Block *child);
    /**
     * @brief info returns this Block info (dependencies, childs, version and other info).
     * @return @see BlockInfo
     */
    BlockInfo *info() const;
    /**
     * @brief loadQml loads qml file (local or remote), setts `block` context property and returns root object.
     * @param url url of a qml file, can be remote path
     * @return qml root object
     */
    QObject *loadQml(const QUrl &url);
    /**
     * @brief addObject shares object with other Blocks
     * @param name name of an object, unique within Block
     * @param object QObject child
     */
    Q_INVOKABLE void addObject(const QString &name, QObject *object);
    /**
     * @brief object returns previously added object
     * @param name name of an object
     * @return
     */
    Q_INVOKABLE QObject *object(const QString &name) const;
    /**
     * @brief objects returns all object that where previously added
     * @return
     */
    Q_INVOKABLE QList<QObject *> objects() const;

    friend class BlockInfoData;
private slots:
    void componentstatusChanged(QQmlComponent::Status status);
private:
    BlockData *d;
};

} // Blocks

#endif // BLOCK_H
