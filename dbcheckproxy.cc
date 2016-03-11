/**
 * @file dbcheckproxy.cc
 * @brief Definitions for DbModel class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2016 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "dbcheckproxy.h"
#include "dbmodelprivate.h"

#include <dbstruct/dbtable.h>
#include <dbstruct/dbview.h>
#include <dbstruct/dbstruct.h>

#include <QSql>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QAbstractItemView>
#include <QItemSelectionModel>

#include <assert.h>

typedef QMap<int,bool> CheckMap;
typedef QMap<int,bool>::iterator CheckMapIter;
typedef QMap<int,bool>::const_iterator CheckMapCIter;

/**
 * @class DbCheckProxy
 *
 * .
 */

DbCheckProxy::DbCheckProxy (int check_column, QObject *parent) :
    QIdentityProxyModel(parent),
    check_column_(check_column)
{

}

bool DbCheckProxy::isChecked (int row_idx) const
{
    CheckMapCIter cmi = checks_.find (row_idx);
    if (cmi == checks_.end ())
        return false;
    else
        return cmi.value ();
}

void DbCheckProxy::clearAllCheckMarks()
{
    checks_.clear ();
}

void DbCheckProxy::setAllCheckMarks()
{
    int i_max = rowCount ();
    for (int i = 0; i < i_max; ++i) {
        checks_.insert (i, true);
    }
}

void DbCheckProxy::clearCheckMark(int row_idx)
{
    CheckMapIter cmi = checks_.find (row_idx);
    if (cmi != checks_.end ())
        checks_.erase (cmi);
}

void DbCheckProxy::setCheckMark (int row_idx, bool b_checked)
{
    if (b_checked)
        checks_.insert (row_idx, true);
    else
        clearCheckMark (row_idx);
}

void DbCheckProxy::setSourceModel(QAbstractItemModel *sourceModel)
{
    check_column_ = 0;
    checks_.clear ();
    QAbstractProxyModel::setSourceModel (sourceModel);
}

QVariant DbCheckProxy::data (const QModelIndex &proxyIndex, int role) const
{
    for (;;) {
        if (role != Qt::CheckStateRole)
            break;
        if (proxyIndex.column() != check_column_)
            break;
        return isChecked (proxyIndex.row()) ? Qt::Checked : Qt::Unchecked;
    }
    return QAbstractProxyModel::data (proxyIndex, role);
}

Qt::ItemFlags DbCheckProxy::flags(const QModelIndex &proxyIndex) const
{
    for (;;) {
        if (proxyIndex.column() != check_column_)
            break;
        return QAbstractProxyModel::flags (proxyIndex) | Qt::ItemIsUserCheckable;
    }
    return QAbstractProxyModel::flags (proxyIndex);
}

bool DbCheckProxy::setData (const QModelIndex &proxyIndex, const QVariant &value, int role)
{
    for (;;) {
        if (role != Qt::CheckStateRole)
            break;
        if (proxyIndex.column() != check_column_)
            break;
        setCheckMark (proxyIndex.row(), value.toInt () == Qt::Checked);
        return true;
    }
    return QAbstractProxyModel::setData (proxyIndex, value, role);
}
