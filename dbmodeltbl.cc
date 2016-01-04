/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodeltbl.cc
  date         November 2015
  author       Nicu Tofan

  brief        Contains the implementation for DbModelTbl class.

*//*

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Please read COPYING and README files in root folder
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/* ------------------------------------------------------------------------- */
/* ========================================================================= */
//
//
//
//
/*  INCLUDES    ------------------------------------------------------------ */

#include "dbmodelprivate.h"
#include "dbmodeltbl.h"
#include <QSqlTableModel>

/*  INCLUDES    ============================================================ */
//
//
//
//
/*  DEFINITIONS    --------------------------------------------------------- */

/*  DEFINITIONS    ========================================================= */
//
//
//
//
/*  CLASS    --------------------------------------------------------------- */

/**
 * @class DbModelTbl
 *
 * Each table that is actually used by this model gets an entry of this
 * kind in `tables_`, with the first one always being the main table.
 *
 * The instance may be invalid if the referenced table is not found
 * (by name) in the database at initialization time (`table()`).
 */

/* ------------------------------------------------------------------------- */
DbModelTbl::DbModelTbl (
        DbTaew * meta_part, QSqlTableModel * model_part) :
    meta_(meta_part),
    columns_(),
    model_(model_part)
{

}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelTbl::setColumnCallback (int column_index, DbColumn::Callback value)
{
    bool b_ret = false;
    for (;;) {

        if (meta_ == NULL) {
            DBMODEL_DEBUGM("Can't set callback for column; no metadata\n");
            break;
        }

        if ((column_index < 0) || (column_index >= columns_.count())) {
            DBMODEL_DEBUGM("Can't set callback for column; index %d "
                           "is out of valid range [0, %d) for columns\n",
                           column_index, columns_.count());
            break;
        }

        DbColumn & col = columns_[column_index];
        if (!col.isDynamic ()) {
            DBMODEL_DEBUGM("Can't set callback for column %d; not dynamic\n",
                           column_index);
            break;
        }

        col.format_.callback_ = value;

        b_ret = true;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbColumn::Callback DbModelTbl::columnCallback (int column_index) const
{
    DbColumn::Callback b_ret = NULL;
    for (;;) {

        if (meta_ == NULL) {
            DBMODEL_DEBUGM("Can't get callback for column; no metadata\n");
            break;
        }

        if ((column_index < 0) || (column_index >= columns_.count())) {
            DBMODEL_DEBUGM("Can't get callback for column; index %d "
                           "is out of valid range [0, %d) for columns\n",
                           column_index, columns_.count());
            break;
        }

        const DbColumn & col = columns_.at(column_index);
        if (!col.isDynamic ()) {
            DBMODEL_DEBUGM("Can't get callback for column %d; not dynamic\n",
                           column_index);
            break;
        }

        b_ret = col.format_.callback_;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbModelTbl::rowCount() const
{
    if (model_ == NULL)
        return 0;
    return model_->rowCount ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelTbl::destroy()
{
    if (meta_ != NULL) {
        delete meta_;
        meta_ = NULL;
    }
    if (model_ != NULL) {
        model_->deleteLater ();
        model_ = NULL;
    }
    columns_.clear();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModelTbl::data (int row, int column, int role) const
{
    if (model_ == NULL)
        return QVariant();
    return model_->data (
                model_->index (
                    row, column), role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbModelTbl::toRealIndex (int value) const
{
    if (meta_ == NULL) {
        return NULL;
    }
    return meta_->toRealIndex (value);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelTbl::constructColumns ()
{
    columns_.clear();
    if (meta_ == NULL)
        return;

    int i_max = meta_->columnCount ();
    for (int i = 0; i < i_max; ++i) {
        columns_.append (meta_->columnCtor (i));
    }
}
/* ========================================================================= */

/*  CLASS    =============================================================== */
//
//
//
//

/* ------------------------------------------------------------------------- */
/* ========================================================================= */

