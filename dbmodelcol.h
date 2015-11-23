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
QT_END_NAMESPACE

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
    int table_index_; /**< index of this column in main table */
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
            int table_index,
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
        const QVariant & key) const;


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


