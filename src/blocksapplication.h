#ifndef BLOCKSAPPLICATION_H
#define BLOCKSAPPLICATION_H

#include <QApplication>
#include "blocks_global.h"

class QQmlEngine;

namespace Blocks {

class Block;
class ApplicationData;
class BLOCKSSHARED_EXPORT Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    ~Application();

    Block *loadBlock(const QString &name);
    void loadBlocks(const QString &path);

    Q_INVOKABLE QObject *object(const QString &path) const;
    Q_INVOKABLE QList<QObject *> objects() const;

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

    template <class T>
    QList<T *> findAll()
    {
        QList<T *> objectsFound;
        foreach (QObject *object, objects()) {
            T *casted = qobject_cast<T *>(object);
            if (casted)
                objectsFound.append(casted);
        }
        return objectsFound;
    }

    void setQmlEngine(QQmlEngine *engine);
    QQmlEngine *qmlEngine() const;

    static Application *instance() { return self; }

private:
    static Application *self;
    ApplicationData *d;
    friend class ApplicationData;
};

} // Blocks

#define blocksApp Blocks::Application::instance()

#endif // BLOCKSAPPLICATION_H
