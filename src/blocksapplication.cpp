#include "blocksapplication.h"
#include "block.h"

#include <QPluginLoader>

#include <QDebug>


class Blocks::ApplicationData {
public:
    QHash<QString, QObject *> objects;
    QList<QObject *> objects_list;
    QQmlEngine *engine;
};

using namespace Blocks;

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
    qDebug() << "loader.load" << loader.load() << loader.errorString();
    qDebug() << loader.instance();
    Block *block = qobject_cast<Block *>(loader.instance());
    if (!block)
        return 0;
    block->plugin();
    return block;
}

void Application::addObject(const QString &path, QObject *object)
{
    if (d->objects.contains(path)) {
        qDebug() << "addObject: already has" << path;
        return;
    }
    d->objects.insert(path, object);
    d->objects_list.push_back(object);
}

QObject *Application::object(const QString &path) const
{
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
