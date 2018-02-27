#ifndef SUGOILIB_GLOBAL_H
#define SUGOILIB_GLOBAL_H

//Compiler error: No such file or directory
//Any header file urls that start with "QtCore/" will cause this error
//I don't know why
//#include <QtCore/qglobal.h>
#include "C:/Qt/Qt5.10.1/5.10.1/msvc2017_64_static/include/QtCore/qglobal.h"

#if defined(_STATIC_BUILD)
#  define SUGOILIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SUGOILIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SUGOILIB_GLOBAL_H
