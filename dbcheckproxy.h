/**
 * @file dbcheckproxy.h
 * @brief Declarations for DbModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2016 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_DBCHECKPROXY_H_INCLUDE
#define GUARD_DBCHECKPROXY_H_INCLUDE

#include <dbmodel/dbmodel-config.h>
#include <dbmodel/dbmodelcol.h>
#include <dbmodel/dbmodeltbl.h>

#include <QMap>
#include <QList>
#include <QSqlRecord>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QVector>
#include <QModelIndex>
#include <QIdentityProxyModel>

QT_BEGIN_NAMESPACE
class QSqlQueryModel;
class QSqlTableModel;
class QAbstractItemView;
QT_END_NAMESPACE

//! A Qt model that adds check boxes to a column.
class DBMODEL_EXPORT DbCheckProxy : public QAbstractItemModel {
    Q_OBJECT

    int check_column_; /**< the index of the column where the check marks will appear */
    QMap<int,bool> checks_; /**< the list of rows and associated checked values */
    bool has_all_; /**< shows an additional column that checks all / none */
    QAbstractItemModel * src_model_;

public:

    //! Constructor.
    explicit DbCheckProxy (
            int check_column = 0,
            QObject *parent = 0);

    //! The column that shows check marks.
    int
    checkColumn () const {
        return check_column_;
    }

    //! Change the column that shows check marks.
    void
    setCheckColumn (
            int value) {
        check_column_ = value;
    }

    //! Show or hide the All row.
    void
    setHasAll (
            bool value) {
        has_all_ = value;
    }

    //! Do we show the All row?.
    bool
    hasAll () const {
        return has_all_;
    }

    //! Tell if a certain row is checked or not.
    bool
    isChecked (
            int row_idx) const;

    //! Remove check marks from all rows.
    void
    clearAllCheckMarks ();

    //! Set check marks from all rows.
    void
    setAllCheckMarks ();

    //! Clear the check mark for a row.
    void
    clearCheckMark (
            int row_idx);

    //! Set the check mark for a row.
    void
    setCheckMark (
            int row_idx,
            bool b_checked = true);

    //! Get the index of all rows that have a check mark.
    QList<int>
    checkedRows () {
        return checks_.keys ();
    }

    QAbstractItemModel *
    sourceModel () const {
        return src_model_;
    }

    virtual void
    setSourceModel (
            QAbstractItemModel *sourceModel);

    virtual QVariant
    data (
            const QModelIndex &proxyIndex,
            int role = Qt::DisplayRole) const;

    virtual QVariant
    headerData (
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;

    virtual Qt::ItemFlags
    flags (
            const QModelIndex &index) const;

    virtual bool
    setData (
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole);

    virtual int
    rowCount (
            const QModelIndex& parent = QModelIndex()) const;

    virtual int
    columnCount (
            const QModelIndex& parent = QModelIndex()) const;


    virtual QModelIndex
    index (
            int row,
            int column,
            const QModelIndex& parent = QModelIndex()) const;

    virtual QModelIndex
    parent (
            const QModelIndex& child) const;

    virtual QModelIndex
    mapFromSource (
            const QModelIndex& sourceIndex) const;

    virtual QModelIndex
    mapToSource (
            const QModelIndex& proxyIndex) const;

private:

    void
    allChanged ();

    void
    oneChanged (
            int row_idx);

    void
    setCheckMarkInternal (
            int row_idx);

signals:

    //! Emitted when one or more items change check status.
    void
    checkChange (
            QList<int> rows);

};

#endif // GUARD_DBCHECKPROXY_H_INCLUDE
