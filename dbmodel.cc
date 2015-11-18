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
#include <dbstruct/dbstruct.h>

#include <QSql>
#include <QSqlError>
#include <QSqlQuery>

#include <assert.h>

/**
 * @class DbModel
 *
 * .
 */

/* ------------------------------------------------------------------------- */
/**
 * This constructor variant uses table metadata provided by the user and it
 * takes it on faith that the table belongs to the `db` database.
 *
 * \param db the database to use (NULL can be used to allow later initialization
 * \param meta table to be loaded (NULL can be used to allow later initialization
 * \param parent the parent QObject
 */
DbModel::DbModel(DbStruct * db, DbTaew * meta, QObject * parent) :
    QAbstractTableModel(parent),
    db_(db),
    mapping_(),
    tables_(),
    col_highlite_(-1),
    row_highlite_(-1)
{
    DBMODEL_TRACE_ENTRY;
    loadMeta (meta);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This constructor variant uses the database to retreive the main table.
 * The `component` must be a valid index for `DbStructMeta::taew()`.
 *
 * \param db the database to use (NULL can be used to allow later initialization
 * \param component table to be loaded (-1 can be used to allow later initialization
 * \param parent the parent QObject
 */
DbModel::DbModel(DbStruct * db, int component, QObject * parent) :
    QAbstractTableModel(parent),
    db_(db),
    mapping_(),
    tables_(),
    col_highlite_(-1),
    row_highlite_(-1)
{
    DBMODEL_TRACE_ENTRY;
    DbTaew * meta = NULL;
    if (db != NULL) {
        meta = db->metaDatabase()->taew (component);
    }
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
    setDatabase(NULL);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The model will NOT be valid after this call because the metadata for the
 * table is not set.
 *
 *
 *
 * @param value the database to use; this instance takes ownership of
 * the pointer and delete will be called in the destructor.
 */
void DbModel::setDatabase (DbStruct * value)
{
    if (db_ == value)
        return;
    if (db_ != NULL) {
        delete db_;
    }
    db_ = value;
    setMeta (NULL);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * A call to this functionammounts to a complete change in the data that is
 * displayied. It is a shortcut for destroying the model and creating a new
 * one for another table.
 *
 * The database is assumed to stay the same. The function fails if there is
 * no database present in this instance.
 *
 * @param meta the table to load; may be NULL to bring the model in
 * invalid state. A future call with a non-NULL value will make it valid again.
 */
void DbModel::setMeta (DbTaew * meta)
{
    DBMODEL_TRACE_ENTRY;
    if ((db_ != NULL) && (meta != metaTaew ())) {
        terminateMeta ();
        loadMeta (meta);
    }
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * A call to this functionammounts to a complete change in the data that is
 * displayied. It is a shortcut for destroying the model and creating a new
 * one for another table.
 *
 * The database if first changed. If the database is valid we proceed to
 * changing the table.
 *
 * Calling this function is a shortcut for:
 * @code
 * model->setDatabase (database);
 * model->setMeta (meta);
 * @endcode
 *
 * @param meta the table to load; may be NULL to bring the model in
 * invalid state. A future call with a non-NULL value will make it valid again.
 */
void DbModel::setMeta (DbStruct * database, DbTaew * meta)
{
    DBMODEL_TRACE_ENTRY;
    setDatabase (database);
    setMeta (meta);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::selectMe ()
{
    DBMODEL_TRACE_ENTRY;

    if (!isValid()) {
        DBMODEL_DEBUGM("Attempt to select invalid model");
        return false;
    }

    // main->setJoinMode (QSqlRelationalTableModel::LeftJoin);
    QSqlTableModel * main = mainModel ();
    bool b_ret = main->select ();
    if (!b_ret) {
        DBMODEL_DEBUGM("model->select failed: %s",
                     TMP_A(main->lastError().text()));
        DBMODEL_DEBUGM("    query: %s",
                     TMP_A(main->query().lastQuery()));
    }
#ifdef DBMODEL_DEBUG
    else {
        DBMODEL_DEBUGM("model->select query: %s",
                     TMP_A(main->query().lastQuery()));
    }
#endif

    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method checks the input against current valid range and stores the
 * coordinates of the cell to highlite with a different color and icon.
 *
 * @param column the column of the cell to highlite
 * @param row the row of the cell to highlite
 * @return true if the checks passed
 */
bool DbModel::setCurrentMarker (int column, int row)
{
    bool b_ret = false;
    for (;;) {

        if (!isValid()) {
            DBMODEL_DEBUGM("The model is in invalid state");
            break;
        }

        if ((column < 0) || (column >= columnCount())) {
            DBMODEL_DEBUGM("column %d outside valid range [0, %d)",
                         column, columnCount());
            break;
        }

        if ((row < 0) || (row >= columnCount())) {
            DBMODEL_DEBUGM("row %d outside valid range [0, %d)",
                         row, rowCount());
            break;
        }

        // save the values
        col_highlite_ = row;
        row_highlite_ = column;

        b_ret = true;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
Qt::ItemFlags DbModel::flags (const QModelIndex &index) const
{
    return Qt::NoItemFlags;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModel::data (const QModelIndex &idx, int role) const
{
    return QVariant ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModel::headerData (
        int section, Qt::Orientation orientation, int role) const
{
    for (;;) {
        // we only handle text
        if (role != Qt::DisplayRole)
            break;

        // we only handle horizontal orientation
        if (orientation != Qt::Horizontal)
            break;

        // column index should be in valid interval
        if ((section < 0) || (section > columnCount()))
            break;
/*
        Col & col = ((DbModel*)this)->mapping_[section];
        if (col.label_.isEmpty()) {
            if (!col.table_->isValid())
                break;

            col.label_ = col.table_->meta->columnLabel (col.t_display_);
        }
*/
        const Col & col = mapping_[section];
        return col.label_;
    }
    return QAbstractTableModel::headerData (section, orientation, role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}
/* ------------------------------------------------------------------------- */

/* ========================================================================= */
int DbModel::rowCount (const QModelIndex &) const
{
    return rowCount ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbModel::columnCount (const QModelIndex &) const
{
    return columnCount ();
}
/* ========================================================================= */

/* ========================================================================= */
/**
 * This method exists because, for table-only models, we will never have
 * a parent.
 *
 * @return number of rows
 */
int DbModel::rowCount () const
{
    if (!isValid())
        return 0;
    return mainModel ()->rowCount();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This method exists because, for table-only models, we will never have
 * a parent.
 *
 * @return number of columns
 */
int DbModel::columnCount () const
{
    return mapping_.count ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Sets the caption for a horizontal header for the specified \a role to
 * \a value. This is useful if the model is used to
 * display data in a view (e.g., QTableView).
 *
 * Returns \c true if \a orientation is Qt::Horizontal and
 * the \a section refers to a valid section; otherwise returns
 * false.
 *
 * Note that this function cannot be used to modify values in the
 * database since the model is read-only.
 *
 */
bool DbModel::setHeaderData (
        int section, Qt::Orientation orientation,
        const QVariant & value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section)
        return false;
    if (role != Qt::DisplayRole)
        if (role != Qt::EditRole)
            return false;

    Col & col = mapping_[section];
    col.label_ = value.toString ();
    emit headerDataChanged (orientation, section, section);
    return true;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Removes the metadata information and any associated resources.
 */
void DbModel::terminateMeta ()
{
    DBMODEL_TRACE_ENTRY;
    clearTables ();
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This is a low level method. Use `setMeta()` to change current table.
 *
 * The internal workings of the model (like marked cell) will be
 * altered even if the method returns false (for example if
 * the \b meta parameter is NULL).
 *
 * @param meta the table to load; may be NULL to bring the model in
 * invalid state. A future call with a non-NULL value will make it valid again.
 *
 * @return true if the model was loaded
 */
bool DbModel::loadMeta (DbTaew * meta)
{
    DBMODEL_TRACE_ENTRY;
    bool b_ret = false;
    col_highlite_ = -1;
    row_highlite_ = -1;
    if ((meta != NULL) && (db_ != NULL)) {

        // There's going to be at least this many columns.
        mapping_.reserve (meta->columnCount ());

        // inform underlyng table about the table we're gonna use
        QSqlTableModel * main = new QSqlTableModel (this, db_->database());
        main->setTable (meta->tableName());

        // our table is always at position 0
        assert(tables_.count() == 0);
        Tbl this_table;
        this_table.model = main;
        this_table.meta = meta;
        tables_.append (this_table);

        // create the columns as we go
        int i_max = meta->columnCount();
        int col_idx = 0;
        for (int i = 0; i < i_max; ++i) {
            DbColumn col = meta->columnCtor (i);
            if (col.isForeignKey ()) {
                addForeignKeyColumn (col, i, col_idx);
            } else {
                Col loc_col (col_idx, i, tables_.first());
                loc_col.t_display_ = i;
                mapping_.append (loc_col);
                loc_col.label_ = loc_col.table_->meta->columnLabel (loc_col.t_display_);
                //setHeaderData(col_idx, Qt::Vertical, QString("X %1").arg (col_idx));
                ++col_idx;
            }
        }

        b_ret = true;
    }
    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This is a low level method.
 *
 * It should not be entered if the database is not valid.
 *
 * Locates a table by name; if not found in internal cache it creates a new
 * one and attempts to initialize it from the database. If the database
 * does not contain a table with this name returned instance will be invalid
 * but it will exist nonetheless.
 *
 * @return a reference inside `tables_`
 */
const DbModel::Tbl & DbModel::table (const QString & name)
{
    DBMODEL_TRACE_ENTRY;
    assert(db_ != NULL);

    // attempt to locate it
    int i = 0;
    foreach(const Tbl & titer, tables_) {
        if (!titer.meta->tableName().compare(name)) {
            assert(i != 0);
            // The method is used to find related tables.
            // A table should not have a relation with itself (is unnatural :).
            return titer;
        }
        ++i;
    }

    // not found so add a new one
    Tbl new_tbl;
    new_tbl.meta = db_->metaDatabase ()->taew (name);
    if (new_tbl.meta == NULL) {
        new_tbl.model = NULL;
        DBMODEL_DEBUGM("The database does not contain a table called %s",
                       TMP_A(name));
    } else {
        new_tbl.model = new_tbl.meta->sqlModel (db_->database(), this);
    }
    tables_.append (new_tbl);

    DBMODEL_TRACE_EXIT;
    return tables_.last();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModel::clearTables ()
{
    DBMODEL_TRACE_ENTRY;
    int i_max = tables_.count();
    for (int i = 0; i < i_max; ++i) {
        Tbl & crt = tables_[i];
        if (crt.meta != NULL) {
            delete crt.meta;
            crt.meta = NULL;
        }
        if (crt.model != NULL) {
            crt.model->deleteLater ();
            crt.model = NULL;
        }
    }
    tables_.clear();
    mapping_.clear();
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModel::addForeignKeyColumn (
        const DbColumn & col, int index, int & col_idx)
{
    DBMODEL_TRACE_ENTRY;

    // attempt to locate the secondary table in the database
    const DbModel::Tbl & secondary = table (col.foreign_table_);
    int key_col = -1;
    if (secondary.isValid()) {
        key_col = secondary.meta->columnIndex (col.foreign_key_);
        if (key_col == -1) {
            DBMODEL_DEBUGM("Key column %s was not "
                           "found in table %s",
                           TMP_A(col.foreign_key_),
                           TMP_A(secondary.meta->tableName()));
        }
    }

    // all display columns will belong to this table
    foreach(const QString & s_iter, col.foreign_ref_) {

        // new column to be added to mapping_ array
        Col loc_col (col_idx, index, secondary);
        if (secondary.isValid() && (key_col != -1)) {
            loc_col.t_primary_ = key_col;
            loc_col.t_display_ = secondary.meta->columnIndex (
                        s_iter);
            if (loc_col.t_display_ == -1) {
                DBMODEL_DEBUGM("Display column %s was not "
                               "found in table %s",
                               TMP_A(s_iter),
                               TMP_A(secondary.meta->tableName()));
                loc_col.t_display_ = key_col;
            }
        } else {
            // If secondary is not valid or key column was not
            // found the id column will be presented to the user.
            // No further intervention is required.
        }

        mapping_.append (loc_col);
        loc_col.label_ = loc_col.table_->meta->columnLabel (loc_col.t_display_);
        //setHeaderData(col_idx, Qt::Vertical, QString("Y %1").arg (col_idx));
        ++col_idx;
    }

    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */
