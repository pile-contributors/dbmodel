/**
 * @file dbmodel.cc
 * @brief Definitions for DbModel class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "dbmodel.h"
#include "dbmodel-private.h"

#include <dbstruct/dbtable.h>
#include <dbstruct/dbview.h>


/**
 * @class DbModel
 *
 * Detailed description.
 */

/* ------------------------------------------------------------------------- */
/**
 * .
 */
DbModel::DbModel(QSqlDatabase & db, DbTaew * meta, QObject * parent) :
    QSqlRelationalTableModel(parent, db),
    meta_(meta)
{
    DBMODEL_TRACE_ENTRY;
    loadMeta (meta);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Detailed description for destructor.
 */
DbModel::~DbModel()
{
    DBMODEL_TRACE_ENTRY;
    terminateMeta ();
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModel::setMeta (DbTaew * meta)
{
    DBMODEL_TRACE_ENTRY;
    if (meta != meta_) {
        terminateMeta ();
        loadMeta (meta);
    }
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModel::terminateMeta ()
{
    DBMODEL_TRACE_ENTRY;
    if (meta_ != NULL) {
        delete meta_;
        meta_ = NULL;
    }
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::loadMeta (DbTaew * meta)
{
    DBMODEL_TRACE_ENTRY;
    bool b_ret = false;
    meta_ = meta;
    if (meta != NULL) {
        setTable (meta->tableName());
        b_ret = true;
    }
    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

