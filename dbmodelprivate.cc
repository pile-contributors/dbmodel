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
#include "dbmodelmanager.h"
#include "dbmodel.h"

#include <dbstruct/dbtable.h>
#include <dbstruct/dbview.h>
#include <dbstruct/dbstruct.h>
#include <dbstruct/dbtaew.h>
#include <dbmodel/dbmodel-config.h>
#include <dbmodel/dbmodelcol.h>
#include <dbmodel/dbmodeltbl.h>

#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QVector>
#include <QSortFilterProxyModel>
#include <QCoreApplication>

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
DbModelPrivate::DbModelPrivate(DbStruct * db, DbTaew * meta, DbModel * parent) :
    QAbstractTableModel(parent),
    db_(db),
    tables_(),
    row_highlite_(-1),
    col_highlite_(-1),
    user_data_(NULL)
{
    DBMODEL_TRACE_ENTRY;
    loadMeta (meta);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This constructor variant uses the database to retrieve the main table.
 * The `component` must be a valid index for `DbStructMeta::taew()`.
 *
 * \param db the database to use (NULL can be used to allow later initialization
 * \param component table to be loaded (-1 can be used to allow later initialization
 * \param parent the parent QObject
 */
DbModelPrivate::DbModelPrivate(DbStruct * db, int component, DbModel * parent) :
    QAbstractTableModel(parent),
    db_(db),
    tables_(),
    row_highlite_(-1),
    col_highlite_(-1),
    user_data_(NULL)
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
DbModel * DbModelPrivate::parentDbModel() const
{
    DbModel * result = qobject_cast<DbModel *>(parent ());
    assert(result != NULL);
    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * A call to this function amounts to a complete change in the data that is
 * displayed. It is a shortcut for destroying the model and creating a new
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
        col_highlite_ = -1;
        row_highlite_ = -1;
    }
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * A call to this functionammounts to a complete change in the data that is
 * displayed. It is a shortcut for destroying the model and creating a new
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
    col_highlite_ = -1;
    row_highlite_ = -1;
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelPrivate::selectMe ()
{
    DBMODEL_TRACE_ENTRY;

    if (!isValid()) {
        DBMODEL_DEBUGM("Attempt to select invalid model\n");
        return false;
    }
    beginResetModel ();
    bool b_ret = true;
    foreach(const DbModelTbl & tbl, tables_) {
        QSqlTableModel * model = tbl.sqlModel ();
        if (model == NULL) {
            b_ret = false;
        } else {
            bool loc_b_ret = model->select ();
            if (!loc_b_ret) {
                DBMODEL_DEBUGM("model->select failed: %s\n",
                             TMP_A(model->lastError().text()));
                DBMODEL_DEBUGM("    query: %s\n",
                             TMP_A(model->query().lastQuery()));
            }
#           ifdef DBMODEL_DEBUG
            else {
                DBMODEL_DEBUGM("        model->select query: %s\n",
                             TMP_A(model->query().lastQuery()));
            }
#           endif
            b_ret = b_ret && loc_b_ret;
        }
    }
    endResetModel ();
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
    return tableData (0).columnCount ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This method exists because, for table-only models, we will never have
 * a parent.
 *
 * @return number of columns
 */
void DbModelPrivate::setDatabase (DbStruct * value)
{
    DbStruct * old_db = takeDatabase();
    if (old_db != NULL) {
        delete old_db;
    }
    db_ = value;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
const DbModelCol &DbModelPrivate::columnData(int index) const
{
    return tableData (0).columnData (index);
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
            DBMODEL_DEBUGM("%d is out of bounds for tables [0, %d)\n",
                           table_index, tables_.count());
            break;
        }

        QSqlTableModel * model = tables_[table_index].sqlModel ();
        if (model == NULL) {
            DBMODEL_DEBUGM("Table %d is invalid\n",
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
            DBMODEL_DEBUGM("This model does not contain a table called %s\n",
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
            DBMODEL_DEBUGM("%d is out of bounds for tables [0, %d)\n",
                           table_index, tables_.count());
            break;
        }

        QSqlTableModel * model = tables_[table_index].sqlModel ();
        if (model == NULL) {
            DBMODEL_DEBUGM("Table %d is invalid\n",
                           table_index);
            break;
        }

        if ((column < 0) && (column >= columnCount ())) {
            DBMODEL_DEBUGM("%d is out of bounds for columns [0, %d)\n",
                           column, columnCount());
            break;
        }

        // if this is a regular column then is easy
        const DbModelCol & c = columnData (column);
        if (!c.isForeign()) {
            model->sort (c.mainTableRealIndex(), order);
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
            DBMODEL_DEBUGM("This model does not contain a table called %s\n",
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
int DbModelPrivate::findTable (const QString &table) const
{
    int i_max = tables_.count();
    for (int i = 0; i < i_max; ++i) {
        if (!tables_.at(i).metadata()->tableName().compare (table, Qt::CaseInsensitive)) {
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
        const DbModelCol & column = columnData (idx.column());

        // Only allow editing if this is allowed in the model
        if (!column.original_.readOnly ()) {
            result = result | Qt::ItemIsEditable;
        }

        break;
    }

    return result;
}
/* ========================================================================= */
#if 0

/* ------------------------------------------------------------------------- */
QString tristateToString (
        int value, const QString & s_true,
        const QString & s_false, const QString & s_undef = QString())
{
    if (value == Qt::Unchecked) return s_false;
    else if (value == Qt::Checked) return s_true;
    else return s_undef;
}
/* ========================================================================= */
#endif
/* ------------------------------------------------------------------------- */
QVariant DbModelPrivate::formattedData (
        const DbColumn & col_meta, const QVariant & original_value)
{
    return col_meta.formattedData (original_value);
#if 0
    QVariant result = original_value;
    switch (col_meta.columnType ()) {
    case DbDataType::DTY_DATE: {
        // see [here](http://doc.qt.io/qt-5/qdatetime.html#toString)
        result = result.toDate().toString(
                    QCoreApplication::translate("UserTime", "yyyy-MMM-dd"));
        break; }
    case DbDataType::DTY_TIME: {
        // see [here](http://doc.qt.io/qt-5/qdatetime.html#toString)
        result = result.toTime().toString(
            QCoreApplication::translate(
                "UserTime", "h:mm:ss"));
        break; }
    case DbDataType::DTY_DATETIME: {
        // see [here](http://doc.qt.io/qt-5/qdatetime.html#toString)
        result = result.toDateTime().toString(
                QCoreApplication::translate(
                    "UserTime", "yyyy-MMM-dd h:mm:ss"));
        break; }
    case DbDataType::DTY_SMALLINT:
    case DbDataType::DTY_BIGINT:
    case DbDataType::DTY_TINYINT:
    case DbDataType::DTY_INTEGER: {
        if (!col_meta.original_format_.isEmpty()) {
            result = QString("%1").arg(
                        result.toLongLong(),
                        col_meta.format_.width_,
                        col_meta.precision_,
                        col_meta.fill_char_);
        }
        break; }
    case DbDataType::DTY_REAL:
    case DbDataType::DTY_MONEY:
    case DbDataType::DTY_SMALLMONEY:
    case DbDataType::DTY_NUMERIC:
    case DbDataType::DTY_NUMERICSCALE:
    case DbDataType::DTY_FLOAT:
    case DbDataType::DTY_DECIMALSCALE:
    case DbDataType::DTY_DECIMAL: {
        if (!col_meta.original_format_.isEmpty()) {
            result = QString("%1").arg(
                        result.toReal(),
                        col_meta.format_.width_,
                        col_meta.nr_format_,
                        col_meta.precision_,
                        col_meta.fill_char_);
        }
        break;}
    case DbDataType::DTY_BIT: {
        switch (col_meta.format_.bit_) {
        case DbColumn::BF_YES_CAMEL: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "Yes") :
                        QCoreApplication::translate("DbModel", "No");
            break; }
        case DbColumn::BF_YES_LOWER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "yes") :
                        QCoreApplication::translate("DbModel", "no");
            break; }
        case DbColumn::BF_YES_UPPER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "YES") :
                        QCoreApplication::translate("DbModel", "NO");
            break; }
        case DbColumn::BF_ON_CAMEL: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "On") :
                        QCoreApplication::translate("DbModel", "Off");
            break; }
        case DbColumn::BF_ON_LOWER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "on") :
                        QCoreApplication::translate("DbModel", "off");
            break; }
        case DbColumn::BF_ON_UPPER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "ON") :
                        QCoreApplication::translate("DbModel", "OFF");
            break; }
        case DbColumn::BF_TRUE_CAMEL: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "True") :
                        QCoreApplication::translate("DbModel", "False");
            break; }
        case DbColumn::BF_TRUE_LOWER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "true") :
                        QCoreApplication::translate("DbModel", "false");
            break; }
        case DbColumn::BF_TRUE_UPPER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "TRUE") :
                        QCoreApplication::translate("DbModel", "FALSE");
            break; }
        case DbColumn::BF_Y_UPPER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "Y") :
                        QCoreApplication::translate("DbModel", "N");
            break; }
        case DbColumn::BF_T_UPPER: {
            result = result.toBool() ?
                        QCoreApplication::translate("DbModel", "T") :
                        QCoreApplication::translate("DbModel", "F");
            break; }
        default: // DbColumn::BF_STRING_ON
            result = result.toBool() ?
                        col_meta.original_format_ :
                        QString();
        }

        break;}
    case DbDataType::DTY_TRISTATE: {
        switch (col_meta.format_.bit_) {
        case DbColumn::BF_YES_CAMEL: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "Yes"),
                        QCoreApplication::translate("DbModel", "No"));
            break; }
        case DbColumn::BF_YES_LOWER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "yes"),
                        QCoreApplication::translate("DbModel", "no"));
            break; }
        case DbColumn::BF_YES_UPPER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "YES"),
                        QCoreApplication::translate("DbModel", "NO"));
            break; }
        case DbColumn::BF_ON_CAMEL: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "On"),
                        QCoreApplication::translate("DbModel", "Off"));
            break; }
        case DbColumn::BF_ON_LOWER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "on"),
                        QCoreApplication::translate("DbModel", "off"));
            break; }
        case DbColumn::BF_ON_UPPER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "ON"),
                        QCoreApplication::translate("DbModel", "OFF"));
            break; }
        case DbColumn::BF_TRUE_CAMEL: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "True"),
                        QCoreApplication::translate("DbModel", "False"));
            break; }
        case DbColumn::BF_TRUE_LOWER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "true"),
                        QCoreApplication::translate("DbModel", "false"));
            break; }
        case DbColumn::BF_TRUE_UPPER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "TRUE"),
                        QCoreApplication::translate("DbModel", "FALSE"));
            break; }
        case DbColumn::BF_Y_UPPER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "Y"),
                        QCoreApplication::translate("DbModel", "N"));
            break; }
        case DbColumn::BF_T_UPPER: {
            result = tristateToString (
                        result.toInt(),
                        QCoreApplication::translate("DbModel", "T"),
                        QCoreApplication::translate("DbModel", "F"));
            break; }
        default: // DbColumn::BF_STRING_ON
            result = tristateToString (
                        result.toInt(),
                        col_meta.original_format_,
                        QString());
        }

        break;}
    default: break;
    };

    return result;
#endif
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelPrivate::removeRows (int row, int count, const QModelIndex &)
{
    if (!isValid())
        return false;
    if (tables_.first().sqlModel()->removeRows (row, count)) {
        DBMODEL_DEBUGM ("%d row(s) removed starting at %d\n", count, row);
        return true;
    } else {
        DBMODEL_DEBUGM ("%d row(s) starting at %d could not be removed\n",
                        count, row);
        return false;
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModelPrivate::data (const QModelIndex & idx, int role) const
{
    for (;;) {

        // Deal with the highlite cell to get that out of the way.
        if ((idx.row() == row_highlite_) && (idx.column() == col_highlite_)) {
            if (role == Qt::DecorationRole) {
                return QVariant (DbModelManager::getIcon ());
            } else if (role == Qt::BackgroundColorRole) {
                return QVariant (DbModelManager::getColor ());
            }
        }

        // Read-only columns have distinctive colors; otherwise we
        // use default processing for these characteristics.
        if (role == Qt::TextColorRole) {
            // read-only items are shown in a lighter font
            Qt::ItemFlags flgs = flags (idx);
            bool beditable = flgs & Qt::ItemIsEditable;
            if (beditable) {
                break;
            } else {
                return QColor(Qt::darkGray);
            }
        } else if (role == Qt::BackgroundColorRole) {
            QVariant editdata = data(idx, Qt::EditRole);
            if (editdata.isNull()) {
                // return QColor(224, 235, 235); // bluish
                return QColor(255, 242, 229); // reddish
            } else {
                break;
            }
        }

        // Make sure the index is within legal boundaries.
        if (!validateIndex (idx))
            break;

        const DbModelTbl & main_table = tables_.first();
        QVariant result = main_table.data (
                    this, idx.row (), idx.column (), role);

        // return model->data (model->index(row, idx_mcol));
        // DBMODEL_DEBUGM("DbModelPrivate::data = %s\n", TMP_A(result.toString()));
        return result;
    }
    return QVariant ();
}
/* ========================================================================= */


#if 0
        // Get the metadata about this column from main table.
        const DbModelCol & column = columnData (idx.column());
        const DbColumn & col_meta = column.original_;

        // Get main table model and its sql backend.
        const DbModelTbl & main_table = tables_.first();
        QSqlTableModel * main_model = main_table.sqlModel ();

        // Degenerate case.
        QVariant result;
        if (main_model == NULL)
            break;

        if (col_meta.isDynamic ()) {
            // for now we can only use dynamic columns on first level
            // but the user may set the callback for related tables.
            QSqlRecord rec = main_model->record (idx.row());
            return column.original_.kbData (*main_table.metadata(),
                                            rec,
                                            role,
                                            parentDbModel ());
        } else {
            // We only service these roles from hereon now.
            if (role != Qt::DisplayRole)
                if (role != Qt::EditRole)
                    break;

            if (col_meta.isVirtual ()) {

                // if this is a virtual column we need the index of the original column
                assert(col_meta.virtrefcol_ >= 0);
                assert(col_meta.virtrefcol_ < columnCount());

                // get the key in foreign table
                const DbModelCol & ref_col = columnData(col_meta.virtrefcol_);
                result = main_model->data (
                            main_model->index (
                                idx.row(),
                                ref_col.mainTableRealIndex ()));
            } else {
                // Get the value stored on this column (may be actual
                // value or the key in a foreign table.
                result = main_model->data (
                            main_model->index (
                                idx.row(),
                                column.mainTableRealIndex ()));
            }
        }

        // For edit role we return raw data.
        if (role == Qt::EditRole) {
            return result;
        }

        // If the column is not foreign we have the result.
        if (!column.isForeign ()) {
            return formattedData (col_meta, result);
        }

        // we have a value that is an index in another table
        QSqlTableModel * model = column.table_->sqlModel();
        if (model == NULL)
            break;
        // DbTaew * meta = column.table_->meta;

        int i_max = model->rowCount();
        for (int i = 0; i < i_max; ++i) {
            // loop-get the key
            QVariant iter_key = model->data (
                        model->index(i, column.t_primary_),
                        Qt::EditRole);
            // did we found it? (inefficient)
            if (iter_key == result) {

                result = model->data (
                            model->index(i, column.t_display_),
                            Qt::DisplayRole);
#               if 0
                // format the data according to the rules for our column
                // we are unable to do this now because the data generated by
                // python script does not merge the overrides in virtual column
                // with original values from the referenced column
                //
                // We need to perform two passes in python,
                // - once collecting data
                // - once for generating content
                result = formattedData (
                            col_meta, result);
#               else
                // format the data according to the rules for source column
                result = formattedData (
                            column.table_->column (column.t_display_),
                            result);
#               endif
                break;
            }
        }
#endif

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
        const DbModelCol & col = columnData(section);
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

        const DbModelCol & col = columnData (idx.column());

        QSqlTableModel * model = mainModel();
        if (model == NULL)
            break;

        bool b_ret = model->setData (
                    model->index (idx.row(), col.mainTableRealIndex()),
                    value, role);
        if (b_ret) {
#           ifdef DBMODEL_DEBUG
            DBMODEL_DEBUGM("model->save: %s\n",
                         TMP_A(model->query().lastQuery()));
#           endif
            model->submit();
            emit dataChanged (idx, idx);
            return true;
        } else {
            DBMODEL_DEBUGM("model->save failed: %s\n",
                         TMP_A(model->lastError().text()));
            DBMODEL_DEBUGM("    query: %s\n",
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
    if (tables_[0].setHeaderData (
            section, orientation, value, role)) {
        emit headerDataChanged (orientation, section, section);
        return true;
    } else {
        return false;
    }
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

        // inform underlying table about the table we're gonna use
        QSqlTableModel * main = new QSqlTableModel (this, db_->database());
        main->setTable (meta->tableName ());
        main->setEditStrategy (QSqlTableModel::OnFieldChange);

        // our table is always at position 0
        assert(tables_.count() == 0);

        tables_.append (DbModelTbl ());

        DbModelTbl & this_table = tables_[0];
        this_table.setSqlModel (main);
        this_table.setMetadata (meta);
        this_table.constructColumns (this);


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
        if (!titer.tableName().compare(name)) {
            assert(i != 0);
            // The method is used to find related tables.
            // A table should not have a relation with itself (is unnatural :).
            return titer;
        }
        ++i;
    }

    // not found so add a new one
    tables_.append (DbModelTbl());
    DbModelTbl & new_tbl = tables_.last();
    DbTaew * intermed = db_->metaDatabase ()->taew (name);
    if (intermed == NULL) {
        new_tbl.setSqlModel (NULL);
        DBMODEL_DEBUGM("The database does not contain a table called %s\n",
                       TMP_A(name));
    } else {
        new_tbl.setSqlModel (intermed->sqlModel (db_->database(), this));
    }
    new_tbl.setMetadata (intermed);
    new_tbl.constructColumns (this);

    DBMODEL_TRACE_EXIT;
    return new_tbl;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelPrivate::clearTables ()
{
    DBMODEL_TRACE_ENTRY;
    int i_max = tables_.count();
    for (int i = 0; i < i_max; ++i) {
        DbModelTbl & crt = tables_[i];
        crt.destroy();
    }
    tables_.clear();
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QSqlRecord DbModelPrivate::record (int row) const
{
    if (!isValid()) {
        return QSqlRecord();
    }
    return mainModel()->record (row);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelPrivate::reloadHeaders ()
{
    beginResetModel();
    tables_[0].retrieveLabels ();
    endResetModel();
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
bool DbModelPrivate::setCurrentMarker (int row, int column)
{
    bool b_ret = false;
    for (;;) {

        if (!isValid()) {
            DBMODEL_DEBUGM("The model is in invalid state\n");
            break;
        }

//        if ((column < 0) || (column >= columnCount())) {
//            DBMODEL_DEBUGM("column %d outside valid range [0, %d)\n",
//                         column, columnCount());
//            break;
//        }

//        if ((row < 0) || (row >= columnCount())) {
//            DBMODEL_DEBUGM("row %d outside valid range [0, %d)\n",
//                         row, rowCount());
//            break;
//        }

        // save the values
        col_highlite_ = column;
        row_highlite_ = row;

        b_ret = true;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelPrivate::setColumnCallback (
        int table_index, int column_index, DbColKb value,
        void * user_data)
{
    bool b_ret = false;
    for (;;) {
        if ((table_index < 0) || (table_index >= tables_.count())) {
            DBMODEL_DEBUGM("Can't set callback for column; index %d "
                           "is out of valid range [0, %d) for tables\n",
                           table_index, tables_.count());
            break;
        }

        DbModelTbl & tbl = tables_[table_index];
        b_ret = tbl.setColumnCallback (column_index, value);

        user_data_ = user_data;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbColKb DbModelPrivate::columnCallback (
        int table_index, int column_index)
{
    DbColKb b_ret = NULL;
    for (;;) {

        if ((table_index < 0) || (table_index >= tables_.count())) {
            DBMODEL_DEBUGM("Can't get callback for column; index %d "
                           "is out of valid range [0, %d) for tables\n",
                           table_index, tables_.count());
            break;
        }

        const DbModelTbl & tbl = tables_.at (table_index);
        b_ret = tbl.columnCallback (column_index);

        break;
    }
    return b_ret;
}
/* ========================================================================= */

/*  CLASS    =============================================================== */
//
//
//
//
void DbModelPrivate::anchorVtable() const {}
/* ------------------------------------------------------------------------- */
/* ========================================================================= */

