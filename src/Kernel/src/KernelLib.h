#ifndef KERNELLIB_H
#define KERNELLIB_H

#include <QtGlobal>

#ifdef KERNEL_EXPORT
# define KERNEL_API Q_DECL_EXPORT
#else
# define KERNEL_API Q_DECL_IMPORT
#endif

#endif // KERNELLIB_H
