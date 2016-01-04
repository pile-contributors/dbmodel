/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodelcol.h
  date         November 2015
  author       Nicu Tofan

  brief        Contains the definition for DbModelCol class.

*//*

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Please read COPYING and README files in root folder
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/* ------------------------------------------------------------------------- */
/* ========================================================================= */
#ifndef DBMODELCOL_H
#define DBMODELCOL_H
//
//
//
//
/*  INCLUDES    ------------------------------------------------------------ */

#include <dbmodel/dbmodel-config.h>
#include <dbstruct/dbstruct.h>
#include <dbstruct/dbcolumn.h>
#include <dbstruct/dbtaew.h>

/*  INCLUDES    ============================================================ */
//
//
//
//
/*  DEFINITIONS    --------------------------------------------------------- */

QT_BEGIN_NAMESPACE
class QComboBox;
class QVariant;
class QModelIndex;
class QCheckBox;
QT_END_NAMESPACE

class DbModel;
class DbModelTbl;

/*  DEFINITIONS    ========================================================= */
//
//
//
//
/*  CLASS    --------------------------------------------------------------- */

//! Model representation of a column.
class DBMODEL_EXPORT DbModelCol
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

public:

    int user_index_; /**< index of this column in `mapping_` */
    const DbModelTbl * table_; /**< the table that holds information that this column shows */
    int t_primary_; /**< column index in referenced table (-1 indicates this is a local column) of the key */
    int t_display_; /**< column index in referenced table (-1 indicates this is a local column) of the display */
    QString label_; /**< cached label for the header */
    DbColumn original_; /**< original column data*/

    /*  DATA    ============================================================ */
    //
    //
    //
    //
    /*  FUNCTIONS    ------------------------------------------------------- */

public:

    //! Default constructor creates a non-foreign key column.
    DbModelCol ();

    //! Default constructor creates a non-foreign key column.
    DbModelCol (
            const DbColumn & source,
            int user_index,
            const DbModelTbl & table);

    //! Copy constructor.
    DbModelCol (const DbModelCol & other);

    //! Assignment operator.
    DbModelCol & operator= (const DbModelCol & other);

    //! destructor
    ~DbModelCol() {}

    //! Tell if this column is foreign or not.
    bool
    isForeign () const {
        return t_primary_ != -1;
    }


    //! Set a combobox to work with this column's data.
    bool
    setCombo (
        QComboBox * control,
        const QVariant & key,
        bool b_delegate_enh = true) const;

    //! Retreives the value and saves it back in the model.
    bool
    getComboValue (
        const QModelIndex & index,
        DbModel * top_model,
        QComboBox * control) const;


    //! Get the result from a foreign key combo.
    QVariant
    comboResult (
            DbModel *top_model,
            QComboBox *control) const;

    QVariant
    comboInsert (
            DbModel *top_model,
            const QString &value) const;



    //! Retreives the value and saves it back in the model.
    bool
    getTristateValue (
        const QModelIndex & index,
        DbModel * top_model,
        QCheckBox * control) const;

    //! Prepare a checkbox to handle tristaed values.
    bool
    setTristate (
        QCheckBox *control,
        const QVariant & value,
        bool b_delegate_enh) const;


    int
    mainTableRealIndex () const {
        return original_.real_col_id_;
    }

    int
    mainTableVirtualIndex () const {
        return original_.col_id_;
    }


    /*  FUNCTIONS    ======================================================= */
    //
    //
    //
    //
}; /* class DbModelCol */

/*  CLASS    =============================================================== */
//
//
//
//


#endif // DBMODELCOL_H
/* ------------------------------------------------------------------------- */
/* ========================================================================= */


