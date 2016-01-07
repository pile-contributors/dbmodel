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

#include <QSql>
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
 * A call to this functionammounts to a complete change in the data that is
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
    return mapping_.count ();
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

        if ((column < 0) && (column >= mapping_.count())) {
            DBMODEL_DEBUGM("%d is out of bounds for columns [0, %d)\n",
                           column, mapping_.count());
            break;
        }

        // if this is a regular column then is easy
        const DbModelCol & c = mapping_.at (column);
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
        const DbColumn & colorig, const QVariant & original_value)
{
    return colorig.formattedData (original_value);
#if 0
    QVariant result = original_value;
    switch (colorig.datatype_) {
    case DbColumn::DTY_DATE: {
        // see [here](http://doc.qt.io/qt-5/qdatetime.html#toString)
        result = result.toDate().toString(
                    QCoreApplication::translate("UserTime", "yyyy-MMM-dd"));
        break; }
    case DbColumn::DTY_TIME: {
        // see [here](http://doc.qt.io/qt-5/qdatetime.html#toString)
        result = result.toTime().toString(
            QCoreApplication::translate(
                "UserTime", "h:mm:ss"));
        break; }
    case DbColumn::DTY_DATETIME: {
        // see [here](http://doc.qt.io/qt-5/qdatetime.html#toString)
        result = result.toDateTime().toString(
                QCoreApplication::translate(
                    "UserTime", "yyyy-MMM-dd h:mm:ss"));
        break; }
    case DbColumn::DTY_SMALLINT:
    case DbColumn::DTY_BIGINT:
    case DbColumn::DTY_TINYINT:
    case DbColumn::DTY_INTEGER: {
        if (!colorig.original_format_.isEmpty()) {
            result = QString("%1").arg(
                        result.toLongLong(),
                        colorig.format_.width_,
                        colorig.precision_,
                        colorig.fill_char_);
        }
        break; }
    case DbColumn::DTY_REAL:
    case DbColumn::DTY_MONEY:
    case DbColumn::DTY_SMALLMONEY:
    case DbColumn::DTY_NUMERIC:
    case DbColumn::DTY_NUMERICSCALE:
    case DbColumn::DTY_FLOAT:
    case DbColumn::DTY_DECIMALSCALE:
    case DbColumn::DTY_DECIMAL: {
        if (!colorig.original_format_.isEmpty()) {
            result = QString("%1").arg(
                        result.toReal(),
                        colorig.format_.width_,
                        colorig.nr_format_,
                        colorig.precision_,
                        colorig.fill_char_);
        }
        break;}
    case DbColumn::DTY_BIT: {
        switch (colorig.format_.bit_) {
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
                        colorig.original_format_ :
                        QString();
        }

        break;}
    case DbColumn::DTY_TRISTATE: {
        switch (colorig.format_.bit_) {
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
                        colorig.original_format_,
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

        if ((idx.row() == row_highlite_) && (idx.column() == col_highlite_)) {
            if (role == Qt::DecorationRole) {
                return QVariant (DbModelManager::getIcon ());
            } else if (role == Qt::BackgroundColorRole) {
                return QVariant (DbModelManager::getColor ());
            }
        }

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
                return QColor(255, 242, 229); // redish
            } else {
                break;
            }
        }


        if (!validateIndex (idx))
            break;

        // data for this column in main table
        const DbModelCol & column = mapping_.at(idx.column());
        const DbColumn & colorig = column.original_;
        QVariant main_value;
        const DbModelTbl & main_table = tables_.first();
        QSqlTableModel * main_model = main_table.sqlModel();
        if (main_model == NULL)
            break;

        if (colorig.isDynamic ()) {
            // for now we can only use dynamic columns on first level
            // but the user may set the callback for related tables.
            QSqlRecord rec = main_model->record (idx.row());
            return column.original_.kbData (*main_table.metadata(),
                                            rec,
                                            role,
                                            parentDbModel ());
        } else if (colorig.isVirtual()) {
            if (role != Qt::DisplayRole)
                if (role != Qt::EditRole)
                    break;
            // if this is a virtual column we need the index of the original column
            assert(colorig.virtrefcol_ >= 0);
            assert(colorig.virtrefcol_ < mapping_.count());

            // get the key in foreign table
            const DbModelCol & ref_col = mapping_.at(colorig.virtrefcol_);
            main_value = main_model->data (
                        main_model->index (
                            idx.row(),
                            ref_col.mainTableRealIndex ()));
        } else {
            if (role != Qt::DisplayRole)
                if (role != Qt::EditRole)
                    break;
            // Get the value stored on this column (may be actual
            // value or the key in a foreign table.
            main_value = main_model->data (
                        main_model->index (
                            idx.row(),
                            column.mainTableRealIndex ()));
        }

        // for simple cases this is it
        if (role == Qt::EditRole) {
            return main_value;
        }

        if (!column.isForeign()) {
            return formattedData (colorig, main_value);
        }

        // we have a value that is an index in another table
        QSqlTableModel * model = column.table_->sqlModel();
        if (model == NULL)
            break;
        // DbTaew * meta = column.table_->meta;

        //! @todo column.t_display_;
        int i_max = model->rowCount();
        for (int i = 0; i < i_max; ++i) {
            // loop-get the key
            QVariant iter_key = model->data (
                        model->index(i, column.t_primary_),
                        Qt::EditRole);
            // did we found it? (ineficient)
            if (iter_key == main_value) {

                main_value = model->data (
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
                main_value = formattedData (
                            colorig, main_value);
#               else
                // format the data according to the rules for source column
                main_value = formattedData (
                            column.table_->column (column.t_display_),
                            main_value);
#               endif
                break;
            }
        }

        // return model->data (model->index(row, idx_mcol));
        // DBMODEL_DEBUGM("DbModelPrivate::data = %s\n", TMP_A(main_value.toString()));
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
        const DbModelCol & col = mapping_.at(section);
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
        this_table.setSqlModel (main);
        this_table.setMetadata (meta);
        tables_.append (this_table);

        // create the columns as we go
        int i_max = meta->columnCount();
        int col_idx = 0;
        for (int i = 0; i < i_max; ++i) {
            DbColumn col = meta->columnCtor (i);
            if (col.isForeignKey ()) {
                addForeignKeyColumn (col, col_idx);
            } else {
                DbModelCol loc_col (col, col_idx, tables_.first());
                loc_col.t_display_ = col.real_col_id_;
                if (loc_col.t_display_  == -1) {
                    DBMODEL_DEBUGM("Cannot use virtual column as display "
                                   "(column %s - %d)\n", TMP_A(col.col_name_), i);
                    continue;
                }
                loc_col.label_ = loc_col.table_->columnLabel (i);
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
        if (!titer.tableName().compare(name)) {
            assert(i != 0);
            // The method is used to find related tables.
            // A table should not have a relation with itself (is unnatural :).
            return titer;
        }
        ++i;
    }

    // not found so add a new one
    DbModelTbl new_tbl;
    DbTaew * intermed = db_->metaDatabase ()->taew (name);
    if (intermed == NULL) {
        new_tbl.setSqlModel (NULL);
        DBMODEL_DEBUGM("The database does not contain a table called %s\n",
                       TMP_A(name));
    } else {
        new_tbl.setSqlModel (intermed->sqlModel (db_->database(), this));
    }
    new_tbl.setMetadata (intermed);
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
        crt.destroy();
    }
    tables_.clear();
    mapping_.clear();
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelPrivate::addForeignKeyColumn (
        const DbColumn & col, int & col_idx)
{
    DBMODEL_TRACE_ENTRY;

    // attempt to locate the secondary table in the database
    const DbModelTbl & secondary = table (col.foreign_table_);
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
        loc_col.t_display_ = secondary.metadata()->realColumnIndex (
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

    loc_col.label_ = tables_.at (0).metadata()->columnLabel (
                loc_col.mainTableVirtualIndex ());

    mapping_.append (loc_col);
    //setHeaderData(col_idx, Qt::Vertical, QString("Y %1").arg (col_idx));
    ++col_idx;

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
    int i_max = mapping_.count();
    for (int i = 0; i < i_max; ++i) {
        DbModelCol & coldata = mapping_[i];
        // const DbColumn & col = coldata.original_;

        coldata.label_ = tables_.at (0).columnLabel (
                        coldata.mainTableVirtualIndex());
    }
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
        int table_index, int column_index, DbColumn::Callback value,
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

        /// @todo the cells in main table are also stored locally;
        /// this is a design flaw as we should only store a reference
        /// in DbModelCol.
        if (b_ret && (table_index == 0)) {
            mapping_[column_index].original_.format_.callback_ = value;
        }

        user_data_ = user_data;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbColumn::Callback DbModelPrivate::columnCallback (
        int table_index, int column_index)
{
    DbColumn::Callback b_ret = NULL;
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

