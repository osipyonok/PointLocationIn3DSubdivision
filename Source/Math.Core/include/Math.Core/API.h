#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(MATH_CORE_LIB)
#  define MATH_CORE_LIB_EXPORT Q_DECL_EXPORT
# else
#  define MATH_CORE_LIB_EXPORT Q_DECL_IMPORT
# endif
#else
# define MATH_CORE_LIB_EXPORT
#endif
