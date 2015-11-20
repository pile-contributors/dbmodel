/* ========================================================================= */
/* ------------------------------------------------------------------------- */
/*!
  file         dbmodelcol.cc
  date         November 2015
  author       Nicu Tofan

  brief        Contains the implementation for DbModelCol class.

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

#include "dbmodel.h"
#include "dbmodelcol.h"
#include "dbmodelprivate.h"

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

/**
 * @class DbModelCol
 *
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


/* ------------------------------------------------------------------------- */
DbModelCol::DbModelCol() :
    user_index_(-1),
    table_index_(-1),
    table_(NULL),
    t_primary_(-1),
    t_display_(-1),
    label_(),
    original_()
{
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelCol::DbModelCol(
        const DbColumn & source, int user_index, int table_index,
        const DbModelTbl & table) :
    user_index_(user_index),
    table_index_(table_index),
    table_(&table),
    t_primary_(-1),
    t_display_(-1),
    label_(),
    original_(source)
{
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelCol::DbModelCol (const DbModelCol & other) :
    user_index_(other.user_index_),
    table_index_(other.table_index_),
    table_(other.table_),
    t_primary_(other.t_primary_),
    t_display_(other.t_display_),
    label_(other.label_),
    original_(other.original_)
{
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelCol & DbModelCol::operator= (const DbModelCol & other)
{
    user_index_ = other.user_index_;
    table_index_ = other.table_index_;
    table_ = other.table_;
    t_primary_ = other.t_primary_;
    t_display_ = other.t_display_;
    label_ = other.label_;
    original_ = other.original_;
    return *this;
}
/* ========================================================================= */

/*  CLASS    =============================================================== */
//
//
//
//

/* ------------------------------------------------------------------------- */
/* ========================================================================= */

