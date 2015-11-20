/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodelprivate.cc
  date         November 2015
  author       Nicu Tofan

  brief        Contains the implementation for DbModelPrivate class.

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
#include "dbmodel.h"

#include <dbstruct/dbtable.h>
#include <dbstruct/dbview.h>
#include <dbstruct/dbstruct.h>
#include <dbstruct/dbtaew.h>
#include <dbmodel/dbmodel-config.h>
#include <dbmodel/dbmodelcol.h>
#include <dbmodel/dbmodeltbl.h>

#include <QSql>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QVector>
#include <QSortFilterProxyModel>

#include <assert.h>

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


/* ------------------------------------------------------------------------- */
/**
 * This constructor variant uses table metadata provided by the user and it
 * takes it on faith that the table belongs to the `db` database.
 *
 * \param db the database to use (NULL can be used to allow later initialization
 * \param meta table to be loaded (NULL can be used to allow later initialization
 * \param parent the parent QObject
 */
DbModelPrivate::DbModelPrivate(DbStruct * db, DbTaew * meta, QObject * parent) :
    QAbstractTableModel(parent),
    db_(db),
    mapping_(),
    tables_()
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
DbModelPrivate::DbModelPrivate(DbStruct * db, int component, QObject * parent) :
    QAbstractTableModel(parent),
    db_(db),
    mapping_(),
    tables_()
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
DbModelPrivate::~DbModelPrivate()
{
    DBMODEL_TRACE_ENTRY;
    terminateMeta ();
    setDatabase(NULL);
    DBMODEL_TRACE_EXIT;
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
void DbModelPrivate::setMeta (DbTaew * meta)
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
void DbModelPrivate::setMeta (DbStruct * database, DbTaew * meta)
{
    DBMODEL_TRACE_ENTRY;
    setDatabase (database);
    setMeta (meta);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelPrivate::selectMe ()
{
    DBMODEL_TRACE_ENTRY;

    if (!isValid()) {
        DBMODEL_DEBUGM("Attempt to select invalid model");
        return false;
    }

    bool b_ret = true;
    foreach(const DbModelTbl & tbl, tables_) {
        QSqlTableModel * model = tbl.model;
        if (model == NULL) {
            b_ret = false;
        } else {
            bool loc_b_ret = model->select ();
            if (!loc_b_ret) {
                DBMODEL_DEBUGM("model->select failed: %s",
                             TMP_A(model->lastError().text()));
                DBMODEL_DEBUGM("    query: %s",
                             TMP_A(model->query().lastQuery()));
            }
#           ifdef DBMODEL_DEBUG
            else {
                DBMODEL_DEBUGM("model->select query: %s",
                             TMP_A(model->query().lastQuery()));
            }
#           endif
            b_ret = b_ret && loc_b_ret;
        }
    }

    // model->setJoinMode (QSqlRelationalTableModel::LeftJoin);

    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This method exists because, for table-only models, we will never have
 * a parent.
 *
 * @return number of rows
 */
int DbModelPrivate::rowCount () const
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
int DbModelPrivate::columnCount () const
{
    return mapping_.count ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * To set the filter for this model's main table call the method without the
 * \b table_index parameter.
 *
 * @param filter The filter to apply
 * @param table_index the index of the table
 * @return false if the index is out of bounds or the model does not exist
 */
bool DbModelPrivate::setFilter (const QString & filter, int table_index)
{
    bool b_ret = false;
    beginResetModel();
    for (;;) {
        if ((table_index < 0) || (table_index >= tables_.count())) {
            DBMODEL_DEBUGM("%d is out of bounds for tables [0, %d)",
                           table_index, tables_.count());
            break;
        }

        QSqlTableModel * model = tables_[table_index].model;
        if (model == NULL) {
            DBMODEL_DEBUGM("Table %d is invalid",
                           table_index);
            break;
        }

        model->setFilter (filter);
        // ! Not calling model->select (); !

        b_ret = true;
        break;
    }
    endResetModel();
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This overload searches for the name in internal list to identify the
 * index, then calls the base method.
 *
 * @param filter The filter to apply
 * @param table
 * @return false if the name was not found or the model is invalid
 */
bool DbModelPrivate::setFilter (const QString & filter, const QString & table)
{
    bool b_ret = false;
    for (;;) {
        int table_index = findTable (table);
        if (table_index == -1) {
            DBMODEL_DEBUGM("This model does not contain a table called %s",
                           TMP_A(table));
            break;
        }

        b_ret = setFilter (filter, table_index);
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * To set the sorting order for this model's main table call
 * the method without the \b table_index parameter.
 *
 * @param column the column to use for sorting;
 * @param order the order to apply to sid column
 * @param table_index the index of the table
 * @return false if the index is out of bounds or the model does not exist
 */
bool DbModelPrivate::setOrder (int column, Qt::SortOrder order, int table_index)
{
    bool b_ret = false;
    beginResetModel();
    for (;;) {
        if ((table_index < 0) || (table_index >= tables_.count())) {
            DBMODEL_DEBUGM("%d is out of bounds for tables [0, %d)",
                           table_index, tables_.count());
            break;
        }

        QSqlTableModel * model = tables_[table_index].model;
        if (model == NULL) {
            DBMODEL_DEBUGM("Table %d is invalid",
                           table_index);
            break;
        }

        if ((column < 0) && (column >= mapping_.count())) {
            DBMODEL_DEBUGM("%d is out of bounds for columns [0, %d)",
                           column, mapping_.count());
            break;
        }

        // if this is a regular column then is easy
        const DbModelCol & c = mapping_.at (column);
        if (!c.isForeign()) {
            model->sort (c.table_index_, order);
        }

    /** @todo */

        b_ret = true;
        break;
    }
    endResetModel();
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This overload searches for the name in internal list to identify the
 * index, then calls the base method.
 *
 * @param filter The filter to apply
 * @param table
 * @return false if the name was not found or the model is invalid
 */
bool DbModelPrivate::setOrder (int column, Qt::SortOrder order, const QString & table)
{
    bool b_ret = false;
    for (;;) {
        int table_index = findTable (table);
        if (table_index == -1) {
            DBMODEL_DEBUGM("This model does not contain a table called %s",
                           TMP_A(table));
            break;
        }

        b_ret = setOrder (column, order, table_index);
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method iterates internal list in search for the name.
 *
 * @param table name of the table to search for
 * @return -1 if it was not found, 0 based index otherwise
 */
int DbModelPrivate::findTable (const QString &table)
{
    int i_max = tables_.count();
    for (int i = 0; i < i_max; ++i) {
        if (!tables_.at(i).meta->tableName().compare (table, Qt::CaseInsensitive)) {
            return i;
        }
    }
    return -1;
}
/* ========================================================================= */






















/* ------------------------------------------------------------------------- */
Qt::ItemFlags DbModelPrivate::flags (const QModelIndex &idx) const
{
    Qt::ItemFlags result = QAbstractTableModel::flags (idx);
    for (;;) {
        if (!validateIndex (idx))
            break;

        // data for this column in main table
        DbModelCol column = mapping_.at (idx.column());

        // Only allow editing if this is allowed in the model
        if (!column.original_.read_only_) {
            result = result | Qt::ItemIsEditable;
        }

        break;
    }

    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModelPrivate::data (const QModelIndex & idx, int role) const
{
    for (;;) {
        if (role != Qt::DisplayRole)
            if (role != Qt::EditRole)
                break;

        if (!validateIndex (idx))
            break;

        // data for this column in main table
        DbModelCol column = mapping_.at(idx.column());
        QSqlTableModel * main_model = tables_.first().model;
        if (main_model == NULL)
            break;
        QVariant main_value = main_model->data (
                    main_model->index(idx.row(), column.table_index_));

        // for simple cases this is it
        if ((role == Qt::EditRole) || !column.isForeign()) {
            return main_value;
        }

        // we have a value that is an index in another table
        QSqlTableModel * model = column.table_->model;
        if (model == NULL)
            break;

        //! @todo column.t_display_;
        int i_max = model->rowCount();
        for (int i = 0; i < i_max; ++i) {
            if (model->data(model->index(i, column.t_primary_), Qt::EditRole) == main_value) {
                main_value = model->data(model->index(i, column.t_display_), Qt::DisplayRole);
                break;
            }
        }


        // return model->data (model->index(row, idx_mcol));
        return main_value;
    }
    return QVariant ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModelPrivate::headerData (
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
        DbModelCol & col = ((DbModelPrivate*)this)->mapping_[section];
        if (col.label_.isEmpty()) {
            if (!col.table_->isValid())
                break;

            col.label_ = col.table_->meta->columnLabel (col.t_display_);
        }
*/
        const DbModelCol & col = mapping_[section];
        return col.label_;
    }
    return QAbstractTableModel::headerData (section, orientation, role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelPrivate::setData (
        const QModelIndex & idx, const QVariant &value, int role)
{
    for (;;) {
        if (!isValid())
            break;

        if (!validateIndex (idx))
            break;

        if (!(flags (idx) & Qt::ItemIsEditable))
            return false;

        const DbModelCol & col = mapping_.at (idx.column());

        QSqlTableModel * model = mainModel();
        if (model == NULL)
            break;

        bool b_ret = model->setData (
                    model->index (idx.row(), col.table_index_),
                    value, role);
        if (b_ret) {
#           ifdef DBMODEL_DEBUG
            DBMODEL_DEBUGM("model->save: %s",
                         TMP_A(model->query().lastQuery()));
#           endif
            model->submit();
            emit dataChanged (idx, idx);
            return true;
        } else {
            DBMODEL_DEBUGM("model->save failed: %s",
                         TMP_A(model->lastError().text()));
            DBMODEL_DEBUGM("    query: %s",
                         TMP_A(model->query().lastQuery()));
        }
        break;
    }
    return QAbstractTableModel::setData (idx, value, role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbModelPrivate::rowCount (const QModelIndex &) const
{
    return rowCount ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbModelPrivate::columnCount (const QModelIndex &) const
{
    return columnCount ();
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
bool DbModelPrivate::setHeaderData (
        int section, Qt::Orientation orientation,
        const QVariant & value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section)
        return false;
    if (role != Qt::DisplayRole)
        if (role != Qt::EditRole)
            return false;

    DbModelCol & col = mapping_[section];
    col.label_ = value.toString ();
    emit headerDataChanged (orientation, section, section);
    return true;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelPrivate::validateIndex (const QModelIndex & idx) const
{
    bool b_ret = false;
    for (;;) {

        if (idx.parent().isValid())
            break;

        int row = idx.row();
        if ((row < 0) || (row >= rowCount()))
            break;

        int col = idx.column();
        if ((col < 0) || (col >= columnCount()))
            break;

        b_ret = true;
        break;
    }
    return b_ret;
}
/* ========================================================================= */


/* ------------------------------------------------------------------------- */
/**
 * Removes the metadata information and any associated resources.
 */
void DbModelPrivate::terminateMeta ()
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
bool DbModelPrivate::loadMeta (DbTaew * meta)
{
    DBMODEL_TRACE_ENTRY;
    bool b_ret = false;
    beginResetModel ();
    if ((meta != NULL) && (db_ != NULL)) {

        // There's going to be at least this many columns.
        mapping_.reserve (meta->columnCount ());

        // inform underlyng table about the table we're gonna use
        QSqlTableModel * main = new QSqlTableModel (this, db_->database());
        main->setTable (meta->tableName());
        main->setEditStrategy (QSqlTableModel::OnFieldChange);

        // our table is always at position 0
        assert(tables_.count() == 0);
        DbModelTbl this_table;
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
                DbModelCol loc_col (col, col_idx, i, tables_.first());
                loc_col.t_display_ = i;
                loc_col.label_ = loc_col.table_->meta->columnLabel (loc_col.t_display_);
                //setHeaderData(col_idx, Qt::Vertical, QString("X %1").arg (col_idx));
                mapping_.append (loc_col);
                ++col_idx;
            }
        }
        b_ret = true;
    }
    endResetModel ();
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
const DbModelTbl & DbModelPrivate::table (const QString & name)
{
    DBMODEL_TRACE_ENTRY;
    assert(db_ != NULL);

    // attempt to locate it
    int i = 0;
    foreach(const DbModelTbl & titer, tables_) {
        if (!titer.meta->tableName().compare(name)) {
            assert(i != 0);
            // The method is used to find related tables.
            // A table should not have a relation with itself (is unnatural :).
            return titer;
        }
        ++i;
    }

    // not found so add a new one
    DbModelTbl new_tbl;
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
void DbModelPrivate::clearTables ()
{
    DBMODEL_TRACE_ENTRY;
    int i_max = tables_.count();
    for (int i = 0; i < i_max; ++i) {
        DbModelTbl & crt = tables_[i];
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
void DbModelPrivate::addForeignKeyColumn (
        const DbColumn & col, int idx, int & col_idx)
{
    DBMODEL_TRACE_ENTRY;

    // attempt to locate the secondary table in the database
    const DbModelTbl & secondary = table (col.foreign_table_);
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
        DbModelCol loc_col (col, col_idx, idx, secondary);
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

        loc_col.label_ = loc_col.table_->meta->columnLabel (loc_col.t_display_);
        mapping_.append (loc_col);
        //setHeaderData(col_idx, Qt::Vertical, QString("Y %1").arg (col_idx));
        ++col_idx;
    }

    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */


/*  CLASS    =============================================================== */
//
//
//
//

/* ------------------------------------------------------------------------- */
/* ========================================================================= */
