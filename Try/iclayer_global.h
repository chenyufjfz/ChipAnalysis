#ifndef ICLAYER_GLOBAL_H
#define ICLAYER_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef ICLAYER_LIB
# define ICLAYER_EXPORT Q_DECL_EXPORT
#else
# define ICLAYER_EXPORT Q_DECL_IMPORT
#endif

#endif // ICLAYER_GLOBAL_H
