/**
 * @file dbmodel.h
 * @brief Declarations for DbModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_DBMODEL_H_INCLUDE
#define GUARD_DBMODEL_H_INCLUDE

#include <dbmodel/dbmodel-config.h>
#include <dbmodel/dbmodelcol.h>
#include <dbmodel/dbmodeltbl.h>

#include <QSql>
#include <QSqlRecord>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QVector>

class DbModelPrivate;

//! A Qt model capable of representing sql tables.
class DBMODEL_EXPORT DbModel : public QSortFilterProxyModel  {
    Q_OBJECT

    DbModelPrivate * impl;
    int row_highlite_; /**< the row for the cell to highlite */
    int col_highlite_; /**< the column for the cell to highlite */

public:

    //! Create a new model from a table or view definition and a database.
    DbModel (
            DbStruct * db,
            DbTaew * meta,
            QObject * parent = NULL);

    //! Create a new model from a database and an index.
    DbModel (
            DbStruct * db,
            int component,
            QObject * parent = NULL);

    Q_DISABLE_COPY(DbModel)

    //! Destructor.
    virtual ~DbModel();

    //! Is this a valid model (with a table set) or not?
    bool
    isValid() const;

    //! Set the table or view; old instance is deleted;
    //! ovnership of table is assumed.
    void
    setMeta (
            DbTaew * meta);

    //! Set the database and table or view; old instance is deleted;
    //! ovnership of table is assumed.
    void
    setMeta (
            DbStruct * database,
            DbTaew * meta);

    //! Give away the pointer and remove it from internal storage.
    DbTaew *
    takeMeta ();

    //! Get a pointer to meta object; ownership stays with this instance.
    DbTaew *
    metaTaew () const;

    //! Get a pointer to internal main model object.
    QSqlTableModel *
    mainModel () const;

    //! Select the model (retreive information using options).
    bool
    selectMe ();

    //! Number of rows.
    int
    rowCount () const;

    //! Number of columns.
    int
    columnCount () const;

    //! Custom sorting.
    bool
    lessThan (
        const QModelIndex &left,
        const QModelIndex &right) const;


    //! Retreive the database. Ownership of returned pointer stays
    //! with this instance.
    DbStruct *
        database () const;

    //! Set the database; also invalidates the table.
    void
    setDatabase (
        DbStruct * value);

    //! Give away the pointer and remove it from internal storage.
    DbStruct *
    takeDatabase ();

    //! Get the model data regarding a column; index is a virtual index.
    const DbModelCol &
    columnData (
        int index) const;

    //! Get the model data regarding a table.
    const DbModelTbl &
    tableData (
        int table_index) const;

    //! Set a filter on one of the internal models identified by its index.
    bool
    setFilter (
            const QString & filter,
            int table_index = 0);

    //! Set a filter on one of the internal models identified by its name.
    bool
    setFilter (
            const QString & filter,
            const QString & table);

    //! Set a filter on one of the internal models identified by its index.
    bool
    setOrder (
            int column,
            Qt::SortOrder order,
            int table_index = 0);

    //! Set a filter on one of the internal models identified by its name.
    bool
    setOrder (
            int column,
            Qt::SortOrder order,
            const QString & table);


    //! Find the index of a model identified by its name.
    int
    findTable (
            const QString & table);

    //! Get a record for a row
    QSqlRecord
    record (
            int row) const;

    //! Read the labels again (possibly in a different language).
    void
    reloadHeaders ();


    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
    /** @name Marker
    * A cell may be highlited in a different color and with
    * a special icon; the model only stores the coordinates
    * of that cell.
    */
    ///@{

    //! Set the indicated .
    bool
    setCurrentMarker (
        int column,
        int row);

    //! Retreive highlite row.
    inline int
    getMarkerRow () const {
        return row_highlite_;
    }

    //! Retreive highlite column.
    inline int
    getMarkerCol () const {
        return col_highlite_;
    }

    //! Tell if we have a highlite cell set.
    inline bool
    hasMarkerCell () const {
        return (col_highlite_ > -1) && (row_highlite_ > -1);
    }

    ///@}
    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */

};

#endif // GUARD_DBMODEL_H_INCLUDE
