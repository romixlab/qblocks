#ifndef BLOCKS_GLOBAL_H
#define BLOCKS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BLOCKS_LIBRARY)
#  define BLOCKSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define BLOCKSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // BLOCKS_GLOBAL_H
