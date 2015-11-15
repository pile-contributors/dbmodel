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
#include <dbstruct/dbstruct-config.h>
#include <dbstruct/dbtaew.h>

#include <QSql>
#include <QSqlRelationalTableModel>
#include <QSqlDatabase>

//! brief description
class DBMODEL_EXPORT DbModel : public QSqlRelationalTableModel {

    DbTaew * meta_; /**< the table or view */

public:

    //! Create a new model from a table or view definition and a database.
    DbModel (
            QSqlDatabase & db,
            DbTaew * meta,
            QObject * parent = NULL);

    //! Destructor.
    virtual ~DbModel();

    //! Set the table or view; old instance is deleted;
    //! ovnership of table is assumed.
    void
    setMeta (
            DbTaew * meta);

    //! Give away the pointer and remove it from internal storage.
    DbTaew *
    takeMeta () {
        DbTaew * result = meta_;
        loadMeta (NULL);
        return result;
    }

protected:

private:

    //! Destroys meta object.
    void
    terminateMeta ();

    //! Loads meta object.
    bool
    loadMeta (
            DbTaew * meta);
};

#endif // GUARD_DBMODEL_H_INCLUDE
