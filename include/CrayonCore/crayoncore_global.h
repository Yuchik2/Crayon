#ifndef CRCORE_GLOBAL_H
#define CRCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CRAYON_CORE_LIBRARY)
#  define CRAYON_CORE_EXPORT Q_DECL_EXPORT
#else
#  define CRAYON_CORE_EXPORT Q_DECL_IMPORT
#endif


#endif // CRCORE_GLOBAL_H
