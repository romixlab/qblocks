#ifndef BLOCKINFO_H
#define BLOCKINFO_H

#include "blocks_global.h"
#include <QString>

namespace Blocks {

class BLOCKSSHARED_EXPORT BlockVersion {
public:
    BlockVersion();
    BlockVersion(const QString &version);

    bool isValid() const;
    QString name() const;
    quint8 versionMajor() const;
    quint8 versionMinor() const;
    quint8 versionRevision() const;
    bool operator==(const BlockVersion &other) const;
    bool operator<(const BlockVersion &other) const;

private:
    QString m_name;
    quint8 m_major, m_minor, m_revision;
};

class Application;
class ApplicationData;
class Block;
class BlockInfoData;
class BLOCKSSHARED_EXPORT BlockInfo
{
public:
    BlockInfo();
    ~BlockInfo();
    enum State {INVALID, METADATA_READED, RESOLVED, LOADED, FAILED};

    Block *block() const;
    bool provides(const BlockVersion &version) const;
    bool resolveDependencies(const QList<BlockInfo *> &blocks);
    QList<BlockInfo *> needs() const;
    State state() const;
    BlockVersion version() const;
    QStringList after() const;
    QString category() const;

    friend class Application;
    friend class ApplicationData;
private:
    BlockInfoData *d;
};

} // Blocks

#endif // BLOCKINFO_H
