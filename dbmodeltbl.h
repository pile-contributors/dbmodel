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
#include <dbstruct/dbtaew.h>
#include <dbstruct/dbcolumn.h>
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

//! Table data is stored only once with multiple references from columns.
class DBMODEL_EXPORT DbModelTbl
{
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

    DbTaew * meta_; /**< metadata about main table or view */
    QList<DbColumn> columns_; /**< cached list of columns */
    QSqlTableModel * model_; /**< the undelying model */

    /*  DATA    ============================================================ */
    //
    //
    //
    //
    /*  FUNCTIONS    ------------------------------------------------------- */

public:

    //! Constructor.
    DbModelTbl() {}

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

    //! Metadata about main table or view
    DbTaew * metadata () const {
        return meta_; }

    //! Set metadata about main table or view.
    void setMetadata (DbTaew * value) {
        meta_ = value;
        constructColumns ();
    }

    //! The undelying model.
    QSqlTableModel * sqlModel () const {
        return model_; }

    //! Set the undelying model.
    void setSqlModel (QSqlTableModel * value) {
        model_ = value;
    }

    //! Get the column for a particular index.
    const DbColumn & column (int colidx) const {
        assert((colidx >= 0) && (colidx < columns_.count()));
        return columns_.at (colidx);
    }

    //! Get the column for a particular index.
    const QString & columnLabel (int colidx) const {
        assert((colidx >= 0) && (colidx < columns_.count()));
        return columns_.at (colidx).col_label_;
    }

    //! Set callback for a column.
    bool
    setColumnCallback (
            int column_index,
            DbColumn::Callback value);

    //! Get callback for a column.
    DbColumn::Callback
    columnCallback (
            int column_index) const;

    //! Get the column for a particular index.
    QString tableName () const {
        if (meta_ == NULL) return QString ();
        return meta_->tableName();
    }

    //! Number of rows in the sql model.
    int
    rowCount () const;

    //! Data from the sql model.
    QVariant
    data (
            int row,
            int column,
            int role = Qt::DisplayRole) const;

    //! Convert the index of a column to real index.
    int
    toRealIndex (
            int value) const;

    //! Clear internal components.
    void
    destroy ();

private:

    //! Create the list of columns.
    void
    constructColumns ();

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
