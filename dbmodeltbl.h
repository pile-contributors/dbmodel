/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodeltbl.h
  date         November 2015
  author       Nicu Tofan

  brief        Contains the definition for DbModelTbl class.

*//*

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Please read COPYING and README files in root folder
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/* ------------------------------------------------------------------------- */
/* ========================================================================= */
#ifndef DBMODELTBL_H
#define DBMODELTBL_H
//
//
//
//
/*  INCLUDES    ------------------------------------------------------------ */

#include <dbmodel/dbmodel-config.h>
#include <dbmodel/dbmodelcol.h>
#include <dbstruct/dbtaew.h>
#include <dbstruct/dbcolumn.h>
#if DBSTRUCT_MAJOR_VERSION >= 1
#include <dbstruct/dbdatatype.h>
#endif
#include <assert.h>

/*  INCLUDES    ============================================================ */
//
//
//
//
/*  DEFINITIONS    --------------------------------------------------------- */

class DbModelPrivate;

/*  DEFINITIONS    ========================================================= */
//
//
//
//
/*  CLASS    --------------------------------------------------------------- */

//! Table data is stored only once with multiple references from columns.
class DBMODEL_EXPORT DbModelTbl {
    //
    //
    //
    //
    /*  DEFINITIONS    ----------------------------------------------------- */

    friend class DbModelPrivate;

    /*  DEFINITIONS    ===================================================== */
    //
    //
    //
    //
    /*  DATA    ------------------------------------------------------------ */

    DbTaew * meta_; /**< metadata about main table or view */
    QSqlTableModel * model_; /**< the underlying model */
    QList<DbModelCol> mapping_; /**< one entry for each column mapping between
                             user-indexes and internal models */

    /*  DATA    ============================================================ */
    //
    //
    //
    //
    /*  FUNCTIONS    ------------------------------------------------------- */

public:

    //! Constructor.
    DbModelTbl();

    //! Constructor.
    DbModelTbl (
            DbTaew * meta_part,
            QSqlTableModel * model_part);

    //! destructor
    ~DbModelTbl() {}

    //! Tell if this instance is valid (found in database).
    bool
    isValid () const {
        return model_ != NULL;
    }

    //! Tell if an index is valid.
    bool
    isColIndexValid (
            int idx) const {
        return ((idx >= 0) && (idx <mapping_.count ()));
    }

    //! Metadata about main table or view
    DbTaew * metadata () const {
        return meta_; }

    //! Set metadata about main table or view.
    void setMetadata (DbTaew * value) {
        meta_ = value;
    }

    //! The underlying model.
    QSqlTableModel * sqlModel () const {
        return model_; }

    //! Set the underlying model.
    void setSqlModel (QSqlTableModel * value) {
        model_ = value;
    }

    //! Get the column for a particular index.
    const DbColumn & column (int colidx) const;

    //! Get the column for a particular index.
    const QString & columnLabel (int colidx) const;

    //! Re-acquire the labels (maybe in the new language).
    void
    retrieveLabels ();

    //! Set callback for a column.
    bool
    setColumnCallback (
            int column_index,
            DbColKb value);

    //! Get callback for a column.
    DbColKb
    columnCallback (
            int column_index) const;

    //! Get the column for a particular index.
    QString tableName () const {
        if (meta_ == NULL) return QString ();
        return meta_->tableName();
    }

    //! Total number of columns (including virtual).
    int
    columnCount () const;

    //! Number of rows in the sql model.
    int
    rowCount () const;

    //! Data from the sql model.
    QVariant
    data (
            const DbModelPrivate* mp,
            int row,
            int column,
            int role = Qt::DisplayRole) const;

    //! Get the model data regarding a column; index is a real index.
    const DbModelCol &
    columnData (
        int index) const;

    //! Convert the index of a column to real index.
    int
    toRealIndex (
            int value) const;

    //! Clear internal components.
    void
    destroy ();

    //! Set the label for a column.
    bool
    setHeaderData (
            int section,
            Qt::Orientation orientation,
            const QVariant & value,
            int role);

    //! Get data.
    QVariant
    data (
            int row,
            int col,
            int role = Qt::EditRole);

protected:

    //! Create the list of columns.
    void
    constructColumns (
            DbModelPrivate* mp);

    //! Create all entries for foreign keys and add them to the list.
    void
    addForeignKeyColumn (
            const DbColumn & col,
            int & col_idx,
            DbModelPrivate* mp);

    /*  FUNCTIONS    ======================================================= */
    //
    //
    //
    //
public: virtual void anchorVtable() const;
}; /* class DbModelTbl */

/*  CLASS    =============================================================== */
//
//
//
//


#endif // DBMODELTBL_H
/* ------------------------------------------------------------------------- */
/* ========================================================================= */
