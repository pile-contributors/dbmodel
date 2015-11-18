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
#include <dbstruct/dbstruct.h>
#include <dbstruct/dbtaew.h>

#include <QSql>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QVector>

//! A Qt model capable of representing sql tables.
class DBMODEL_EXPORT DbModel : public QAbstractTableModel {
    Q_OBJECT

private:

    /** @todo Think about:
     model->setEditStrategy (QSqlTableModel::OnManualSubmit);
    model->setEditStrategy (QSqlTableModel::OnFieldChange);
    model->setJoinMode(QSqlRelationalTableModel::LeftJoin);
    */

    //! Table data is stored only once with multiple references from columns
    /**
     * Each table that is actually used by this model gets an entry of this
     * kind in `tables_`, with the first one always being the main table.
     *
     * The instance may be invalid if the referenced table is not found
     * (by name) in the database at initialization time (`table()`).
     */
    struct Tbl {
        DbTaew * meta; /**< metadata about main table or view */
        QSqlTableModel * model; /**< the undelying model */

        //! Tell if this instance is valid (found in database).
        bool
        isValid () const {
            return model != NULL;
        }
    };

    //! Internal representation for a column */
    /**
     * One such structure is created for each column that the user sees.
     * At load time the table is inspected, each column is iterated and one or
     * more such structures are created.
     *
     * If the column is not a foreign key then only `user_index_`,
     * `table_index_` and `table_` are used, with `t_primary_` set to -1 to
     * indicate that this is not a foreign column.
     *
     * If the column is a foreign on then the value in main table is to
     * be searched in secondary table (`t_display_`),
     * on `t_primary_` column. Once found, the user should
     * be presented with the value stored in column
     * `t_display_`.
     *
     * This system enables the user to present any number of columns from
     * a secondary table while using a single foreign key column in
     * main table.
     */
    struct Col {
        //! Default constructor creates a non-foreign key column.
        Col () :
            user_index_(-1),
            table_index_(-1),
            table_(NULL),
            t_primary_(-1),
            t_display_(-1),
            label_()
        {}

        //! Default constructor creates a non-foreign key column.
        Col (int user_index, int table_index, const Tbl & table) :
            user_index_(user_index),
            table_index_(table_index),
            table_(&table),
            t_primary_(-1),
            t_display_(-1),
            label_()
        {}

        //! copy constructor
        Col (const Col & other) :
            user_index_(other.user_index_),
            table_index_(other.table_index_),
            table_(other.table_),
            t_primary_(other.t_primary_),
            t_display_(other.t_display_),
            label_(other.label_)
        {}

        //! assignment operator
        Col& operator=( const Col & other) {
            user_index_ = other.user_index_;
            table_index_ = other.table_index_;
            table_ = other.table_;
            t_primary_ = other.t_primary_;
            t_display_ = other.t_display_;
            label_ = other.label_;
            return *this;
        }


        int user_index_; /**< index of this column in `mapping_` */
        int table_index_; /**< index of this column in main table */
        const Tbl * table_; /**< the table that holds information that this column shows */
        int t_primary_; /**< column index in referenced table (-1 indicates this is a local column) of the key */
        int t_display_; /**< column index in referenced table (-1 indicates this is a local column) of the display */
        QString label_; /**< cached label for the header */

        //! Tell f this column is foreign or not.
        bool
        isForeign () {
            return t_primary_ != -1;
        }
    };

    DbStruct * db_; /**< The database we're connecting to. */
    QVector<Col> mapping_; /**< one entry for each column mapping between
                             user-indices and internal models */
    QVector<Tbl> tables_; /**< the list of tables referenced by this model */
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

    //! Destructor.
    virtual ~DbModel();

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
        return result;
    }


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



private:

    //! Destroys meta object.
    void
    terminateMeta ();

    //! Loads meta object.
    bool
    loadMeta (
        DbTaew * meta);

    //! Find a table by name.
    const DbModel::Tbl &
    table (
        const QString &name);

    //! Remove all tables (and all columns).
    void
    clearTables ();

    //! Create all entries for foreign keys and add them to the list.
    void
    addForeignKeyColumn (
        const DbColumn & col,
        int index,
        int & col_idx);

};

#endif // GUARD_DBMODEL_H_INCLUDE
