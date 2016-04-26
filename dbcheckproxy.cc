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

/* ------------------------------------------------------------------------- */
DbCheckProxy::DbCheckProxy (int check_column, QObject *parent) :
    QAbstractItemModel (parent),
    check_column_(check_column),
    checks_ (),
    has_all_ (false),
    src_model_(NULL)
{
    DBMODEL_TRACE_ENTRY;
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbCheckProxy::isChecked (int row_idx) const
{
    CheckMapCIter cmi = checks_.find (row_idx);
    if (cmi == checks_.end ())
        return false;
    else
        return cmi.value ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::clearAllCheckMarks()
{
    DBMODEL_TRACE_ENTRY;
    checks_.clear ();
    allChanged ();
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::setAllCheckMarks ()
{
    DBMODEL_TRACE_ENTRY;
    int i_max = rowCount ();
    QList<int> ilst;
    for (int i = 0; i < i_max; ++i) {
        checks_.insert (i, true);
        ilst.append (i);
    }
    emit checkChange (ilst);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::clearCheckMark (int row_idx)
{
    DBMODEL_TRACE_ENTRY;
    CheckMapIter cmi = checks_.find (row_idx);
    if (cmi != checks_.end ()) {
        checks_.erase (cmi);
        oneChanged (row_idx);
    }
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::setCheckMarkInternal (int row_idx)
{
    DBMODEL_TRACE_ENTRY;
    for (;;) {
        CheckMapIter cmi = checks_.find (row_idx);
        if (cmi != checks_.end ()) {
            if (cmi.value())
                break;
        }

        checks_.insert (row_idx, true);
        oneChanged (row_idx);
        break;
    }
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::setCheckMark (int row_idx, bool b_checked)
{
    DBMODEL_TRACE_ENTRY;
    if (has_all_) {
        if (row_idx == 0) {
            beginResetModel ();
            if (b_checked)
                setAllCheckMarks ();
            else
                clearAllCheckMarks ();
            endResetModel ();
            return;
        }
    }
    if (b_checked)
        setCheckMarkInternal (row_idx);
    else
        clearCheckMark (row_idx);
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::setSourceModel (QAbstractItemModel *sourceModel)
{
    DBMODEL_TRACE_ENTRY;
    check_column_ = 0;
    checks_.clear ();
    beginResetModel ();
    src_model_ = sourceModel;
    endResetModel ();
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbCheckProxy::data (const QModelIndex &proxyIndex, int role) const
{
    DBMODEL_TRACE_ENTRY;
    if (src_model_ == NULL) {
        return QVariant ();
    }

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
            return src_model_->index (
                        proxyIndex.row () - 1,
                        proxyIndex.column ()).data (role);
        }
    } else {
        return src_model_->index (
                    proxyIndex.row (),
                    proxyIndex.column ()).data (role);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbCheckProxy::headerData (
        int section, Qt::Orientation orientation, int role) const
{
    DBMODEL_TRACE_ENTRY;
    if (src_model_ == NULL) {
        return QVariant ();
    }
    return src_model_->headerData (section, orientation, role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
Qt::ItemFlags DbCheckProxy::flags (
        const QModelIndex &proxyIndex) const
{
    DBMODEL_TRACE_ENTRY;
    Qt::ItemFlags f;
    if (src_model_ == NULL) {
        return f;
    }

    QModelIndex idx = proxyIndex;
    for (;;) {
        if (!has_all_)
            break;
        if (idx.row() == 0) {
            f = Qt::ItemIsEnabled |Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
            if (check_column_ == proxyIndex.column ()) {
                f = f | Qt::ItemIsUserCheckable;
            }
            return f;
        } else {
            f = src_model_->index (
                        proxyIndex.row()-1, proxyIndex.column ())
                    .flags ();
            return f | Qt::ItemIsUserCheckable;
        }
    }

    f = src_model_->index (
                proxyIndex.row(), proxyIndex.column ())
            .flags ();
    if (proxyIndex.column() == check_column_)
        f = f | Qt::ItemIsUserCheckable;
    return f;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbCheckProxy::setData (
        const QModelIndex &proxyIndex, const QVariant &value, int role)
{
    DBMODEL_TRACE_ENTRY;
    if (src_model_ == NULL) {
        return false;
    }

    for (;;) {
        if (role != Qt::CheckStateRole)
            break;
        if (proxyIndex.column() != check_column_)
            break;
        setCheckMark (proxyIndex.row(), value.toInt () == Qt::Checked);
        return true;
    }
    int row = proxyIndex.row();
    if (has_all_) {
        if (row == 0)
            return false;
        --row;
    }
    return src_model_->setData (
                src_model_->index (
                    row, proxyIndex.column ()),
                value, role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbCheckProxy::rowCount (const QModelIndex & parent) const
{
    DBMODEL_TRACE_ENTRY;
    if (src_model_ == NULL) {
        return 0;
    }

    int r = src_model_->rowCount (mapToSource (parent)) + (has_all_ ? 1 : 0);
    DBMODEL_TRACE_EXIT;
    return r;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int DbCheckProxy::columnCount (const QModelIndex &parent) const
{
    DBMODEL_TRACE_ENTRY;
    if (src_model_ == NULL) {
        return 0;
    }

    int c = src_model_->columnCount (mapToSource (parent));
    DBMODEL_TRACE_EXIT;
    return c;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QModelIndex DbCheckProxy::index (
        int row, int column, const QModelIndex & /*parent*/) const
{
    int rc = rowCount ();
    if ((row < 0) || (row >= rc)) {
        return QModelIndex ();
    }
    if ((column < 0) || (column >= columnCount ())) {
        return QModelIndex ();
    }
    return createIndex (row, column);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QModelIndex DbCheckProxy::parent (const QModelIndex &child) const
{
    if (src_model_ == NULL)
        return QModelIndex ();
    if (!child.isValid ())
        return QModelIndex ();
    return mapFromSource (mapToSource (child).parent ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QModelIndex DbCheckProxy::mapFromSource (const QModelIndex &sourceIndex) const
{
    int row = sourceIndex.row ();
    if (row < 0)
        return QModelIndex ();
    if (has_all_) {
        ++row;
    }
    return createIndex (row, sourceIndex.column ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QModelIndex DbCheckProxy::mapToSource (const QModelIndex &proxyIndex) const
{
    if (src_model_ == NULL) {
        return QModelIndex ();
    }

    int row = proxyIndex.row ();
    if (has_all_) {
        if (row == 0) {
            return QModelIndex ();
        }
        --row;
    }
    return src_model_->index (row, proxyIndex.column ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::allChanged ()
{
    QList<int> ilst;
    int i_max = rowCount ();
    for(int i = 0; i < i_max; ++i) {
        ilst.append (i);
    }
    emit checkChange (ilst);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void DbCheckProxy::oneChanged (int row_idx)
{
    QList<int> ilst;
    ilst.append (row_idx);
    emit checkChange (ilst);
}
/* ========================================================================= */

