#ifndef SUGOILIB_GLOBAL_H
#define SUGOILIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(_STATIC_BUILD)
#  define SUGOILIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SUGOILIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SUGOILIB_GLOBAL_H
