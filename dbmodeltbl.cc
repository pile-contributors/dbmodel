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
#include <QSqlRecord>

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
DbModelTbl::DbModelTbl ()  :
    meta_(NULL),
    model_(NULL),
    mapping_()
{
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelTbl::DbModelTbl (
        DbTaew * meta_part, QSqlTableModel * model_part) :
    meta_(meta_part),
    model_(model_part),
    mapping_()
{
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
const DbColumn &DbModelTbl::column(int colidx) const
{
    assert((colidx >= 0) && (colidx < columnCount ()));
    return mapping_.at (colidx).original_;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
const QString & DbModelTbl::columnLabel (int colidx) const
{
    assert((colidx >= 0) && (colidx < columnCount ()));
    return mapping_.at (colidx).original_.columnLabel ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelTbl::retrieveLabels ()
{
    int i_max = meta_->columnCount ();
    for (int i = 0; i < i_max; ++i) {
        mapping_[i].original_.col_label_ = meta_->columnLabel (i);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelTbl::setColumnCallback (int column_index, DbColKb value)
{
    bool b_ret = false;
    for (;;) {

        if (meta_ == NULL) {
            DBMODEL_DEBUGM("Can't set callback for column; no metadata\n");
            break;
        }

        if ((column_index < 0) || (column_index >= columnCount ())) {
            DBMODEL_DEBUGM("Can't set callback for column; index %d "
                           "is out of valid range [0, %d) for columns\n",
                           column_index, columnCount());
            break;
        }

        b_ret = mapping_[column_index].setColumnCallback (value);
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbColKb DbModelTbl::columnCallback (int column_index) const
{
    DbColKb b_ret = NULL;
    for (;;) {

        if (meta_ == NULL) {
            DBMODEL_DEBUGM("Can't get callback for column; no metadata\n");
            break;
        }

        if ((column_index < 0) || (column_index >= columnCount ())) {
            DBMODEL_DEBUGM("Can't get callback for column; index %d "
                           "is out of valid range [0, %d) for columns\n",
                           column_index, columnCount ());
            break;
        }

        b_ret = mapping_.at (column_index).columnCallback ();
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * @return number of columns
 */
int DbModelTbl::columnCount () const
{
    return mapping_.count ();
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
void DbModelTbl::destroy ()
{
    if (meta_ != NULL) {
        delete meta_;
        meta_ = NULL;
    }
    if (model_ != NULL) {
        model_->deleteLater ();
        model_ = NULL;
    }
    mapping_.clear();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelTbl::setHeaderData (int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section)
        return false;
    if (role != Qt::DisplayRole)
        if (role != Qt::EditRole)
            return false;

    DbModelCol & col = mapping_[section];
    col.label_ = value.toString ();
    return true;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModelTbl::data (
        const DbModelPrivate* mp, int row, int col, int role) const
{
#ifdef DBMODEL_DEBUG
    if (col == 6) {
        if (role == Qt::DisplayRole) {
            int kkk = 0;
        }
    }

#endif

    QVariant result;
    for (;;) {
        if (!isColIndexValid (col)) {
            break;
        }

        // Degenerate case.
        if (model_ == NULL) {
            break;
        }

        const DbModelCol & column = mapping_.at (col);
        const DbColumn & col_meta = column.original_;

        if (col_meta.isDynamic ()) {
            QSqlRecord rec = model_->record (row);
            result = column.original_.kbData (*meta_,
                                            rec,
                                            role,
                                            mp->parentDbModel ());
            break;
        } else {
            // We only service these roles from hereon now.
            if (role != Qt::DisplayRole)
                if (role != Qt::EditRole)
                    break;

            if (col_meta.isVirtual ()) {

                // if this is a virtual column we need the index of the original column
                assert(col_meta.virtrefcol_ >= 0);
                assert(col_meta.virtrefcol_ < columnCount ());

                // get the key in foreign table
                const DbModelCol & ref_col = columnData (col_meta.virtrefcol_);
                result = model_->index (
                            row, ref_col.mainTableRealIndex ())
                        .data (Qt::EditRole);
            } else {
                // Get the value stored on this column (may be actual
                // value or the key in a foreign table.
                result = model_->index (
                            row, column.mainTableRealIndex ())
                        .data (Qt::EditRole);
            }
        }

        // For edit role we return raw data.
        if (role == Qt::EditRole) {
            break;
        }

        // If the column is not foreign we have the result.
        if (!column.isForeign ()) {
            result = col_meta.formattedData (result);
            break;
        }

        // we have a value that is an index in another table
        QSqlTableModel * model = column.table_->sqlModel ();
        if (model == NULL)
            break;

        int i_max = model->rowCount ();
        for (int i = 0; i < i_max; ++i) {
            // loop-get the key
            QVariant iter_key =
                    model->index (i, column.t_primary_)
                    .data (Qt::EditRole);
            // did we found it? (inefficient)
            if (iter_key == result) {
                result = column.table_->data (
                            mp, i, column.t_display_, Qt::EditRole);
//                result = model->index (i, column.t_display_)
//                        .data (Qt::DisplayRole);

                // format the data according to the rules for source column
                result = column.table_->column (column.t_display_).formattedData (
                            result);
                break;
            }
        }

        break;
    }
    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
const DbModelCol &DbModelTbl::columnData(int index) const
{
    return mapping_.at (index);
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
void DbModelTbl::constructColumns (DbModelPrivate* mp)
{
    mapping_.clear();
    if (meta_ == NULL)
        return;

    int i_max = meta_->columnCount ();

    // There's going to be at least this many columns.
    mapping_.reserve (i_max);

    // create the columns as we go
    int col_idx = 0;
    for (int i = 0; i < i_max; ++i) {
        DbColumn col = meta_->columnCtor (i);
        if (col.isForeignKey ()) {
            addForeignKeyColumn (col, col_idx, mp);
        } else {
            DbModelCol loc_col (col, col_idx, *this);
            loc_col.t_display_ = col.columnId ();
//            loc_col.t_display_ = col.columnRealId();
            if (loc_col.t_display_  == -1) {
                DBMODEL_DEBUGM("Cannot use virtual column as display "
                               "(column %s - %d)\n",
                               TMP_A(col.columnName()), i);
                continue;
            }
            loc_col.label_ = loc_col.original_.columnLabel ();
            //setHeaderData(col_idx, Qt::Vertical, QString("X %1").arg (col_idx));
            mapping_.append (loc_col);
            ++col_idx;
        }
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelTbl::addForeignKeyColumn (
        const DbColumn & col, int & col_idx, DbModelPrivate* mp)
{
    DBMODEL_TRACE_ENTRY;

    // attempt to locate the secondary table in the database
    const DbModelTbl & secondary = mp->table (col.foreign_table_);
    int key_col = -1;
    if (secondary.isValid()) {
        key_col = secondary.metadata()->realColumnIndex (col.foreign_key_);
        if (key_col == -1) {
            DBMODEL_DEBUGM("Key column %s was not "
                           "found in table %s\n",
                           TMP_A(col.foreign_key_),
                           TMP_A(secondary.metadata()->tableName()));
        }
    }

    // new column to be added to mapping_ array
    DbModelCol loc_col (col, col_idx, secondary);
    if (secondary.isValid() && (key_col != -1)) {
        loc_col.t_primary_ = key_col;
//        loc_col.t_display_ = secondary.metadata()->realColumnIndex (
//                    col.foreign_ref_);
        loc_col.t_display_ = secondary.metadata()->columnIndex (
                    col.foreign_ref_);
        if (loc_col.t_display_ == -1) {
            DBMODEL_DEBUGM("Display column %s was not "
                           "found in table %s or is virtual\n",
                           TMP_A(col.foreign_ref_),
                           TMP_A(secondary.metadata()->tableName()));
            loc_col.t_display_ = key_col;
        }
    } else {
        // If secondary is not valid or key column was not
        // found the id column will be presented to the user.
        // No further intervention is required.
    }

    loc_col.label_ = metadata()->columnLabel (
                loc_col.mainTableVirtualIndex ());

    mapping_.append (loc_col);
    //setHeaderData(col_idx, Qt::Vertical, QString("Y %1").arg (col_idx));
    ++col_idx;

    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/*  CLASS    =============================================================== */
//
//
//
//
void DbModelTbl::anchorVtable() const {}
/* ------------------------------------------------------------------------- */
/* ========================================================================= */

