#include "block.h"
#include "blocksapplication.h"
#include <QQmlContext>

#include <QDebug>

class Blocks::BlockData
{
public:
    BlockData() :
        context(0)
    { }
    QQmlContext *context;
};

using namespace Blocks;

Block::Block(QObject *parent) :
    QObject(parent), d(new BlockData)
{ }

Block::~Block()
{
    delete d;
}

void Block::plugged(Blocks::Block *child)
{
    Q_UNUSED(child)
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
        qDebug() << "Success loading of qml component";
        qmlComponent->deleteLater();
    } else if (qmlComponent->isError()) {
        qDebug() << "Error loading qml component" << qmlComponent->errorString();
        delete qmlObject;
        qmlComponent->deleteLater();
    } else {
        connect(qmlComponent, SIGNAL(statusChanged(QQmlComponent::Status)),
                this,         SLOT(componentStatusChanged(QQmlComponent::Status)));
    }

    qDebug() << "loadQml" << qmlObject;
    return qmlObject;
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
