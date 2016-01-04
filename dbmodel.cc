/**
 * @file dbmodel.cc
 * @brief Definitions for DbModel class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "dbmodel.h"
#include "dbmodelprivate.h"

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
    QSortFilterProxyModel(parent),
    impl(new DbModelPrivate (db, meta, this)),
    filter_()
{
    DBMODEL_TRACE_ENTRY;
    setSourceModel(impl);
    setSortCaseSensitivity (Qt::CaseInsensitive);
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
    QSortFilterProxyModel(parent),
    impl(new DbModelPrivate (db, component, this)),
    filter_()
{
    DBMODEL_TRACE_ENTRY;
    setSourceModel(impl);
    setSortCaseSensitivity (Qt::CaseInsensitive);
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
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::isValid() const
{
    return impl->isValid ();
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
    impl->setMeta (meta);
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
    impl->setMeta (database, meta);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbTaew * DbModel::takeMeta ()
{
    return impl->takeMeta();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbTaew * DbModel::metaTaew () const
{
    return impl->metaTaew();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QSqlTableModel * DbModel::mainModel () const
{
    return impl->mainModel();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::selectMe ()
{
    return impl->selectMe();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This method exists because, for table-only models, we will never have
 * a parent.
 *
 * @return number of rows
 */
int DbModel::rowCount () const
{
    // DBMODEL_DEBUGM("%d rows\n", impl->rowCount());
    return impl->rowCount();
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
    if (impl == NULL) return 0;
    return impl->columnCount ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbStruct * DbModel::database () const
{
    if (impl == NULL) return NULL;
    return impl->database();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The model will NOT be valid after this call because the metadata for the
 * table is not set.
 *
 * @param value the database to use; this instance takes ownership of
 * the pointer and delete will be called in the destructor.
 */
void DbModel::setDatabase (DbStruct * value)
{
    if (impl != NULL)
        impl->setDatabase (value);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbStruct * DbModel::takeDatabase ()
{
    if (impl == NULL) return NULL;
    return impl->takeDatabase();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
const DbModelCol & DbModel::columnData (int index) const
{
    return impl->columnData (index);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
const DbModelTbl & DbModel::tableData (int table_index) const
{
    return impl->tableData (table_index);
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
bool DbModel::setFilter (const QString & filter, int table_index)
{
    if (table_index == 0) {
        filter_ = filter;
    }
    return impl->setFilter (filter, table_index);
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
bool DbModel::setFilter (const QString & filter, const QString & table)
{
    int table_index = impl->findTable (table);
    if (table_index == -1) {
        DBMODEL_DEBUGM("This model does not contain a table called %s\n",
                       TMP_A(table));
        return false;
    }

    return setFilter (filter, table_index);
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
bool DbModel::setOrder (int column, Qt::SortOrder order, int table_index)
{
    return impl->setOrder (column, order, table_index);
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
bool DbModel::setOrder (int column, Qt::SortOrder order, const QString & table)
{
    return impl->setOrder (column, order, table);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method iterates internal list in search for the name.
 *
 * @param table name of the table to search for
 * @return -1 if it was not found, 0 based index otherwise
 */
int DbModel::findTable (const QString &table) const
{
    return impl->findTable (table);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QSqlRecord DbModel::record(int row) const
{
    QModelIndex inimpl = mapToSource (index(row, 0));
    return impl->record (inimpl.row());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModel::reloadHeaders ()
{
    return impl->reloadHeaders ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::setColumnCallback (
        int table_index, int column_index,
        DbColumn::Callback value, void * user_data)
{
    return impl->setColumnCallback (
                table_index, column_index, value, user_data);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbColumn::Callback DbModel::columnCallback (
        int table_index, int column_index)
{
    return impl->columnCallback (table_index, column_index);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void * DbModel::columnCallbackData ()
{
    return impl->columnCallbackData ();
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
bool DbModel::setCurrentMarker (int row, int column)
{
    return impl->setCurrentMarker (row, column);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::lessThan (const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if (leftData.type() == QVariant::DateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
    } else if (leftData.type() == QVariant::Date) {
        return leftData.toDate() < rightData.toDate();
    } else if (leftData.type() == QVariant::Time) {
        return leftData.toTime() < rightData.toTime();
    } else {
        return QSortFilterProxyModel::lessThan (left, right);
        /*
        static QRegExp emailPattern("[\\w\\.]*@[\\w\\.]*)");

        QString leftString = leftData.toString();
        if(left.column() == 1 && emailPattern.indexIn(leftString) != -1)
            leftString = emailPattern.cap(1);

        QString rightString = rightData.toString();
        if(right.column() == 1 && emailPattern.indexIn(rightString) != -1)
            rightString = emailPattern.cap(1);

        return QString::localeAwareCompare(leftString, rightString) < 0;
        */
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbModel::getMarkerRow () const
{
    return impl->getMarkerRow ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbModel::getMarkerCol () const
{
    return impl->getMarkerCol ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModel::hasMarkerCell () const
{
    return impl->hasMarkerCell ();
}
/* ========================================================================= */
