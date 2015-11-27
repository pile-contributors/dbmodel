/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodelprivate.h
  date         November 2015
  author       Nicu Tofan

  brief        Contains the definition for DbModelPrivate class.

*//*

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Please read COPYING and README files in root folder
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/* ------------------------------------------------------------------------- */
/* ========================================================================= */
#ifndef DBMODELPRIVATE_H
#define DBMODELPRIVATE_H
//
//
//
//
/*  INCLUDES    ------------------------------------------------------------ */

#include <QAbstractTableModel>
#include <QList>

#include <dbmodel/dbmodel-config.h>
#include <dbmodel/dbmodelcol.h>
#include <dbmodel/dbmodeltbl.h>
#include <dbstruct/dbstruct.h>
#include <dbstruct/dbtaew.h>

/*  INCLUDES    ============================================================ */
//
//
//
//
/*  DEFINITIONS    --------------------------------------------------------- */

#if 1
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

/*  DEFINITIONS    ========================================================= */
//
//
//
//
/*  CLASS    --------------------------------------------------------------- */

//! Internal model.
class DbModelPrivate : public QAbstractTableModel {
    Q_OBJECT

    /** @todo Think about:
     *
     * model->setEditStrategy (QSqlTableModel::OnManualSubmit);
     * model->setEditStrategy (QSqlTableModel::OnFieldChange);
     * model->setJoinMode(QSqlRelationalTableModel::LeftJoin);
     */

    //
    //
    //
    //
    /*  DEFINITIONS    ----------------------------------------------------- */

    /*  DEFINITIONS    ===================================================== */
    //
    //
    //
    //
    /*  DATA    ------------------------------------------------------------ */

private:

    DbStruct * db_; /**< The database we're connecting to. */
    QList<DbModelCol> mapping_; /**< one entry for each column mapping between
                             user-indices and internal models */
    QList<DbModelTbl> tables_; /**< the list of tables referenced by this model */
    int row_highlite_; /**< the row for the cell to highlite */
    int col_highlite_; /**< the column for the cell to highlite */

    /*  DATA    ============================================================ */
    //
    //
    //
    //
    /*  FUNCTIONS    ------------------------------------------------------- */

public:

    //! Create a new model from a table or view definition and a database.
    DbModelPrivate(
            DbStruct * db,
            DbTaew * meta,
            QObject * parent = NULL);

    //! Create a new model from a database and an index.
    DbModelPrivate (
            DbStruct * db,
            int component,
            QObject * parent = NULL);

    Q_DISABLE_COPY(DbModelPrivate)

    //! destructor
    virtual ~DbModelPrivate();

    //! Is this a valid model (with a table set) or not?
    bool
    isValid() const {
        return  (db_ != NULL) &&
                (tables_.count() > 0) &&
                tables_.at (0).isValid();
    }

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
    inline DbTaew *
    takeMeta () {
        if (tables_.count() == 0)
            return NULL;
        DbTaew * result = tables_.first().meta;
        tables_.first().meta = NULL;
        terminateMeta ();
        col_highlite_ = -1;
        row_highlite_ = -1;
        return result;
    }

    //! Get a pointer to meta object; ownership stays with this instance.
    inline DbTaew *
    metaTaew () const {
        if (tables_.count() == 0)
            return NULL;
        return tables_.at (0).meta;
    }

    //! Get a pointer to internal main model object.
    inline QSqlTableModel *
    mainModel () const {
        if (tables_.count() == 0)
            return NULL;
        return tables_.at (0).model;
    }

    //! Select the model (retreive information using options).
    bool
    selectMe ();

    //! Number of rows.
    int
    rowCount () const;

    //! Number of columns.
    int
    columnCount () const;

    //! Retreive the database. Ownership of returned pointer stays
    //! with this instance.
    inline DbStruct *
        database () const {

        return db_;
    }

    //! Set the database; also invalidates the table.
    void
    setDatabase (
        DbStruct * value);

    //! Give away the pointer and remove it from internal storage.
    inline DbStruct *
    takeDatabase () {
        DbStruct * result = db_;
        db_ = NULL;
        setMeta (NULL);
        col_highlite_ = -1;
        row_highlite_ = -1;
        return result;
    }

    //! Get the model data regarding a column; index is a real index.
    const DbModelCol &
    columnData (
        int index) const {

        return mapping_.at (index);
    }

    //! Get the model data regarding a table.
    const DbModelTbl &
    tableData (
        int table_index) const {

        return tables_.at (table_index);
    }

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
            const QString & table) const;

    //! Get a record for a row
    QSqlRecord
    record (
            int row) const;

    //! Read the labels again (possibly in a different language).
    void
    reloadHeaders ();


    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
    /** @name QSqlTableModel
    *  Reimplemented methods from QSqlTableModel.
    */
    ///@{

public:

    Qt::ItemFlags
    flags (
        const QModelIndex &index) const;

    QVariant
    data (
        const QModelIndex &idx,
        int role = Qt::DisplayRole) const;

    QVariant
    headerData (
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;

    bool
    setHeaderData (
        int section,
        Qt::Orientation orientation,
        const QVariant & value,
        int role = Qt::EditRole);

    bool
    setData (
        const QModelIndex &index,
        const QVariant &value,
        int role = Qt::EditRole);

    //! Tell if an index is valid for this model.
    bool
    validateIndex (
            const QModelIndex &idx) const;

    //! Tell if an index is valid for this model.
    void
    sort (
        int column,
        Qt::SortOrder order = Qt::AscendingOrder) {

        setOrder (column, order);
    }

    //! Convert the data for the user.
    static QVariant
    formattedData (
            const DbColumn &colorig,
            const QVariant &original_value);

    //! Remove a number of rows
    bool
    removeRows (
        int row,
        int count,
        const QModelIndex &parent);


protected:

    int
    rowCount (
        const QModelIndex &parent) const;

    int
    columnCount (
        const QModelIndex &parent) const;


    /*/! Prevent the user from using this method - use `setMeta()` instead.
    virtual void
    setTable (
            const QString & value) {
        QSqlRelationalTableModel::setTable (value);
    } */

    ///@}
    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */


    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
    /** @name Marker
    * A cell may be highlited in a different color and with
    * a special icon; the model only stores the coordinates
    * of that cell.
    */
    ///@{

public:

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


private:

    //! Destroys meta object.
    void
    terminateMeta ();

    //! Loads meta object.
    bool
    loadMeta (
        DbTaew * meta);

    //! Find a table by name.
    const DbModelTbl &
    table (
        const QString &name);

    //! Remove all tables (and all columns).
    void
    clearTables ();

    //! Create all entries for foreign keys and add them to the list.
    void
    addForeignKeyColumn (
        const DbColumn & col,
        int & col_idx);



    /*  FUNCTIONS    ======================================================= */
    //
    //
    //
    //
}; /* class DbModelPrivate */

/*  CLASS    =============================================================== */
//
//
//
//


#endif // DBMODELPRIVATE_H
/* ------------------------------------------------------------------------- */
/* ========================================================================= */


