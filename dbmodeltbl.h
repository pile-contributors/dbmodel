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

public:

    DbTaew * meta; /**< metadata about main table or view */
    QSqlTableModel * model; /**< the undelying model */

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
        return model != NULL;
    }

    /*  FUNCTIONS    ======================================================= */
    //
    //
    //
    //
}; /* class DbModelTbl */

/*  CLASS    =============================================================== */
//
//
//
//


#endif // DBMODELTBL_H
/* ------------------------------------------------------------------------- */
/* ========================================================================= */


