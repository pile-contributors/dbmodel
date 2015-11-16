/**
 * @file dbmodel-private.h
 * @brief Declarations for DbModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_DBMODEL_PRIVATE_H_INCLUDE
#define GUARD_DBMODEL_PRIVATE_H_INCLUDE

#include <dbmodel/dbmodel-config.h>

#if 0
#    define DBMODEL_DEBUGM printf
#else
#    define DBMODEL_DEBUGM black_hole
#endif

#if 0
#    define DBMODEL_TRACE_ENTRY printf("DBMODEL ENTRY %s in %s[%d]\n", __func__, __FILE__, __LINE__)
#else
#    define DBMODEL_TRACE_ENTRY
#endif

#if 0
#    define DBMODEL_TRACE_EXIT printf("DBMODEL EXIT %s in %s[%d]\n", __func__, __FILE__, __LINE__)
#else
#    define DBMODEL_TRACE_EXIT
#endif

#ifndef TMP_A
#   define TMP_A(__s__) __s__.toLatin1 ().constData ()
#endif

static inline void black_hole (...)
{}

#endif // GUARD_DBMODEL_PRIVATE_H_INCLUDE
