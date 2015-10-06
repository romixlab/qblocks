#include "block.h"
#include "block_p.h"
#include "blockinfo.h"
#include "blocksapplication.h"
#include <QQmlContext>

#include <QDebug>

using namespace Blocks;

Block::Block(QObject *parent) :
    QObject(parent), d(new BlockData)
{
    d->objects.insert("self", this);
}

Block::~Block()
{
    qDebug() << "DESTRUCTING BLOCK!!!" << d->info->version().name();
    delete d;
}

void Block::plugged(Blocks::Block *child)
{
    Q_UNUSED(child)
}

BlockInfo *Block::info() const
{
    return d->info;
}

QObject *Block::loadQml(const QUrl &url)
{
    if (!d->context) {
        d->context = new QQmlContext(blocksApp->qmlEngine(), this);
        d->context->setContextProperty("blocksApp", blocksApp);
        d->context->setContextProperty("block", this);
    }
    QQmlComponent *qmlComponent = new QQmlComponent(blocksApp->qmlEngine(), this);
    qmlComponent->loadUrl(url);

    QQmlContext *componentContext = new QQmlContext(d->context, this);
    QObject *qmlObject = qmlComponent->create(componentContext);
    componentContext->setParent(qmlObject);

    if (qmlComponent->isReady()) {
        //qDebug() << "Success loading of qml component";
        qmlComponent->deleteLater();
    } else if (qmlComponent->isError()) {
        qDebug() << "Error loading qml component" << qmlComponent->errorString();
        delete qmlObject;
        qmlComponent->deleteLater();
    } else {
        connect(qmlComponent, SIGNAL(statusChanged(QQmlComponent::Status)),
                this,         SLOT(componentStatusChanged(QQmlComponent::Status)));
    }

    //qDebug() << "loadQml" << qmlObject;
    return qmlObject;
}

void Block::addObject(const QString &name, QObject *object)
{
    if (d->objects.contains(name)) {
        qDebug() << "Block::addObject: already added, replacing" << name;
        d->objects_list.removeAll(d->objects[name]);
    }
    d->objects.insert(name, object);
    d->objects_list.push_back(object);
    if (object->parent() == 0) {
        object->setParent(this);
        qDebug() << "Adding parent to" << object;
    }
}

QObject *Block::object(const QString &name) const
{
    if (d->objects.contains(name))
        return d->objects[name];
    qWarning() << "Block" << info()->version().name() << "object" << name << "not found";
    return 0;
}

QList<QObject *> Block::objects() const
{
    return d->objects_list;
}

void Block::componentstatusChanged(QQmlComponent::Status status)
{
    qDebug() << "QuickBlock::componentstatusChanged()";
    QQmlComponent *component = qobject_cast<QQmlComponent*>(this->sender());
    if(component)
    {
        switch(status)
        {
        case QQmlComponent::Loading:
        case QQmlComponent::Null:
            break; // wait for the next update
        case QQmlComponent::Ready:
            qDebug() << "Success";
            component->deleteLater();
            break;
        case QQmlComponent::Error:
            qDebug() << "Fail" << component->errorString();
            component->deleteLater();
            break;
        }
    }
    else
        qDebug() << "Null component";
}
