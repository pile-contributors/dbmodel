/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodelmanager.cc
  date         Oct 2015
  author

  brief        Contains the implementation for DbModelManager class.

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

#include "dbmodelmanager.h"

#include <QApplication>
#include <QStyle>

/*  INCLUDES    ============================================================ */
//
//
//
//
/*  DEFINITIONS    --------------------------------------------------------- */

DbModelManager * DbModelManager::uniq_ = NULL;

/*  DEFINITIONS    ========================================================= */
//
//
//
//
/*  CLASS    --------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
bool DbModelManager::init()
{
    bool b_ret = false;
    for (;;) {
        if (uniq_ != NULL) {
            b_ret = true;
            break;
        }
        uniq_ = new DbModelManager ();


        b_ret = true;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbModelManager::end ()
{
    for (;;) {
        if (uniq_ == NULL) {
            break;
        }


        delete uniq_;
        break;
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelManager::DbModelManager() :
    crt_icon_marker_(QApplication::style()->standardIcon(QStyle::SP_MediaPlay)),
    crt_color_marker_(QColor (255, 255, 153))
{
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelManager::~DbModelManager()
{

}
/* ========================================================================= */

/*  CLASS    =============================================================== */
//
//
//
//
/* ------------------------------------------------------------------------- */
/* ========================================================================= */

