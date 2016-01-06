/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodelmanager.h
  date         Oct 2015
  author

  brief        Contains the definition for DbModelManager class.

*//*

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Please read COPYING and README files in root folder
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
/* ------------------------------------------------------------------------- */
/* ========================================================================= */
#ifndef DBMODELMANAGER_H
#define DBMODELMANAGER_H
//
//
//
//
/*  INCLUDES    ------------------------------------------------------------ */

#include <dbmodel/dbmodel-config.h>

#include <QIcon>
#include <QColor>

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

//! .
class DBMODEL_EXPORT DbModelManager {
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

    QIcon crt_icon_marker_; /**< Icon used to indicate current items */
    QColor crt_color_marker_; /**< Background used to indicate current items */
    static DbModelManager * uniq_; /**< The one and only instance */

    /*  DATA    ============================================================ */
    //
    //
    //
    //
    /*  FUNCTIONS    ------------------------------------------------------- */

public:

    //! Initialize the manager
    static bool
    init ();

    //! Terminate the manager.
    static void
    end ();

    //! Retrieve icon marker.
    static const QIcon &
    getIcon () {
        return uniq_->crt_icon_marker_;
    }

    //! Set icon marker.
    static void
    setIcon (const QIcon & value) {
        uniq_->crt_icon_marker_ = value;
    }

    //! Retrieve color marker.
    static const QColor &
    getColor () {
        return uniq_->crt_color_marker_;
    }

    //! Set color marker.
    static void
    setColor (const QColor & value) {
        uniq_->crt_color_marker_ = value;
    }


protected:

    //! Constructor.
    DbModelManager();

    //! destructor
    virtual ~DbModelManager();

private:

    /*  FUNCTIONS    ======================================================= */
    //
    //
    //
    //
}; /* class DbModelManager */

/*  CLASS    =============================================================== */
//
//
//
//



#endif // DBMODELMANAGER_H
/* ------------------------------------------------------------------------- */
/* ========================================================================= */


