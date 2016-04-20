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
#include <QFont>
#include <QApplication>

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
    check_column_(check_column),
    checks_ (),
    has_all_ (false)
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

void DbCheckProxy::clearCheckMark (int row_idx)
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

void DbCheckProxy::setSourceModel (QAbstractItemModel *sourceModel)
{
    check_column_ = 0;
    checks_.clear ();
    QIdentityProxyModel::setSourceModel (sourceModel);
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
    if (has_all_) {
        if (proxyIndex.row () == 0) {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                return QObject::tr ("All");
            case Qt::FontRole: {
                QFont f = QApplication::font ();
                f.setBold (true);
                return f;
            }
            };
            return QVariant ();
        } else {
            return /*QIdentityProxyModel::*/index (
                        proxyIndex.row () - 1,
                        proxyIndex.column ()).data (role);
        }
    } else {
        return QIdentityProxyModel::data (proxyIndex, role);
    }
}

Qt::ItemFlags DbCheckProxy::flags (const QModelIndex &proxyIndex) const
{
    QModelIndex idx = proxyIndex;
    for (;;) {
        if (!has_all_)
            break;
        if (idx.row() == 0) {
            /// @Todo
        }
    }
    for (;;) {
        if (proxyIndex.column() != check_column_)
            break;
        return QIdentityProxyModel::flags (proxyIndex) | Qt::ItemIsUserCheckable;
    }
    return QIdentityProxyModel::flags (proxyIndex);
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
    return QIdentityProxyModel::setData (proxyIndex, value, role);
}
