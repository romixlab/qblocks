#ifndef BLOCKSAPPLICATION_H
#define BLOCKSAPPLICATION_H

#include <QApplication>

class QQmlEngine;

namespace Blocks {

class Block;
class ApplicationData;
class Application : public QApplication
{
public:
    Application(int &argc, char **argv);
    ~Application();

    Block *loadBlock(const QString &name);

    void addObject(const QString &path, QObject *object);
    QObject *object(const QString &path) const;
    QList<QObject *> objects() const;

    template <class T>
    T *findFirst()
    {
        foreach (QObject *object, objects()) {
            T *casted = qobject_cast<T *>(object);
            if (casted)
                return casted;
        }
        return 0;
    }

    void setQmlEngine(QQmlEngine *engine);
    QQmlEngine *qmlEngine() const;

    static Application *instance() { return self; }

private:
    static Application *self;
    ApplicationData *d;
};

} // Blocks

#define blocksApp Blocks::Application::instance()

#endif // BLOCKSAPPLICATION_H
