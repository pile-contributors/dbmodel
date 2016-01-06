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

#include <dbstruct/dbrecord.h>

#include "assert.h"

#include <QSqlTableModel>
#include <QVariant>
#include <QComboBox>
#include <QLineEdit>
#include <QIdentityProxyModel>
#include <QDebug>
#include <QTimer>
#include <QCheckBox>

/*  INCLUDES    ============================================================ */
//
//
//
//
/*  DEFINITIONS    --------------------------------------------------------- */

#if 0
class ComboFix : public QIdentityProxyModel  {
    QSqlTableModel * model_;
public:
    explicit ComboFix(QSqlTableModel * model, QObject* parent = 0) :
        QIdentityProxyModel(parent),
        model_(model)
    {
        setSourceModel (model);
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (role == Qt::EditRole) role = Qt::DisplayRole;
        QVariant result = QIdentityProxyModel::data (index, role);
        DBMODEL_DEBUGM("--- role %d: %s\n", role, TMP_A(result.toString()));
        return result;
    }
    QMap<int, QVariant> itemData(const QModelIndex &index) const
    {
        DBMODEL_DEBUGM("--- itemData %d, %d\n", index.row(), index.column());
        return QIdentityProxyModel::itemData (index);
    }

    bool insertRows (int row, int count, const QModelIndex& parent = QModelIndex())
    {
        DBMODEL_DEBUGM("--- insertRows %d, %d\n", row, count);
        bool b_ret = QIdentityProxyModel::insertRows (row, count, parent);
        /*if (b_ret) {
            model_->submit();
        }*/
        return b_ret;
    }

    bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
    {
        DBMODEL_DEBUGM("--- setData %d, %d = %s\n", index.row(), index.column(), TMP_A(value.toString()));
        bool b_ret = QIdentityProxyModel::setData (index, value, role);
        if (b_ret) {
            model_->submit();
        }
        return b_ret;
    }

    bool setItemData (const QModelIndex &index, const QMap<int, QVariant> &roles)
    {
        DBMODEL_DEBUGM("--- setItemData %d, %d\n", index.row(), index.column());
        return QIdentityProxyModel::setItemData (index, roles);
    }


};
#endif


//! A line editor for helping with ComboBoxes for foreign keys.
/**
 * Faced a problem with combos edit-enabled that, when the editing starts,
 * show the index instead of the text for current item.
 *
 * As a workaround, we set initial text in forced_text_ and set a
 * one_time_trigger_ to tell us whn is the first time the editor text changes and
 * we replace that text with cached version.
 *
 * To also show the list when the editor is created we use a one-shot timer
 * that presents th pop-up (if this is done in the function the pop-up is
 * shown at the wrong position).
 */
class ComboLine : public QLineEdit  {
    Q_OBJECT
public:

    bool one_time_trigger_;
    QString forced_text_;

    //! Constructor.
    explicit ComboLine(QWidget* parent = 0) :
        QLineEdit(parent),
        one_time_trigger_(true),
        forced_text_()
    {
        connect(this, SIGNAL(textChanged(QString)),
                this, SLOT(ttChanged(QString)));
    }

public slots:

    //! We're informed that the text has changed.
    void ttChanged (const QString & s_value)
    {
        /* no longer needed because I stopped
         * QStyledItemDelegate::setEditorData (editor, index);
         *
        if (one_time_trigger_) {
            one_time_trigger_ = false;
            setText (forced_text_);
        }*/
    }

    //! Force a pop-up show.
    void showPopUp ()
    {
        QComboBox * cb = qobject_cast<QComboBox *>(parentWidget());
        if (cb == NULL)
            return;
        cb->showPopup();
    }
};


#include "dbmodelcol.moc"

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
 * and `table_` are used, with `t_primary_` set to -1 to
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
    table_(NULL),
    t_primary_(-1),
    t_display_(-1),
    label_(),
    original_()
{
    DBMODEL_TRACE_ENTRY;
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelCol::DbModelCol(const DbColumn & source, int user_index,
        const DbModelTbl & table) :
    user_index_(user_index),
    table_(&table),
    t_primary_(-1),
    t_display_(-1),
    label_(),
    original_(source)
{
    DBMODEL_TRACE_ENTRY;
    assert(table_->isValid());
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelCol::DbModelCol (const DbModelCol & other) :
    user_index_(other.user_index_),
    table_(other.table_),
    t_primary_(other.t_primary_),
    t_display_(other.t_display_),
    label_(other.label_),
    original_(other.original_)
{
    DBMODEL_TRACE_ENTRY;
    if (table_ != NULL) {
        assert(table_->isValid());
    }
    DBMODEL_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
DbModelCol & DbModelCol::operator= (const DbModelCol & other)
{
    user_index_ = other.user_index_;
    table_ = other.table_;
    t_primary_ = other.t_primary_;
    t_display_ = other.t_display_;
    label_ = other.label_;
    original_ = other.original_;

    if (table_ != NULL) {
        assert(table_->isValid());
    }
    return *this;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelCol::setTristate (
        QCheckBox *control, const QVariant & value, bool b_delegate_enh) const
{
    DBMODEL_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {
        control->setTristate (true);
        int val = value.toInt();
        if ((val < Qt::Unchecked) || (val > Qt::Checked)) {
            val = Qt::PartiallyChecked;
        }
        Qt::CheckState sts = (Qt::CheckState)val;
        control->setCheckState (sts);
        b_ret = true;
        break;
    }
    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelCol::setCombo (
        QComboBox *control, const QVariant & key, bool b_delegate_enh) const
{
    DBMODEL_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {

        if (!isForeign()) {
            DBMODEL_DEBUGM("The column is not a foreign one\n");
            break;
        }

        if (!table_->isValid()) {
            DBMODEL_DEBUGM("Referenced table is not valid\n");
            break;
        }

        QSqlTableModel * model = table_->sqlModel();
        bool b_found = false;

        if (original_.foreign_behaviour_ == DbColumn::FB_CHOOSE_ADD) {
            // foreign keys allowed to edit
            control->setEditable (true);
            control->setInsertPolicy (QComboBox::InsertAtBottom);
            ComboLine * le = new ComboLine(control);
            control->setLineEdit (le);
            if (b_delegate_enh)
                QTimer::singleShot (200, le, SLOT(showPopUp()));

            int i_max = model->rowCount ();
            for (int i = 0; i < i_max; ++i) {
                QString s_value = model->data(model->index(i, t_display_),
                                              Qt::DisplayRole).toString();
                control->addItem(s_value, QVariant(i));
                if (model->data(model->index(i, t_primary_), Qt::EditRole) == key) {
                    control->setCurrentIndex (i);
                    control->setEditText (s_value);
                    le->forced_text_ = s_value;
                    le->one_time_trigger_ = true;
                    b_found = true;
                }
            }

            // in dialogs we do not need this enhancement
            if (!b_delegate_enh || !b_found) {
                le->one_time_trigger_ = false;
            }
//            if (!b_found) {
//                QString s = model->data(model->index(i, t_display_),
//                                        Qt::DisplayRole)
//                            .toString();
//                control->setCurrentText (s);
//            }

        } else {
            // foreign keys not allowed to edit
            control->setEditable (false);

            control->setModel (model);
            control->setModelColumn (t_display_);

            int i_max = model->rowCount ();
            for (int i = 0; i < i_max; ++i) {
                if (model->data(model->index(i, t_primary_), Qt::EditRole) == key) {
                    control->setCurrentIndex (i);
                    b_found = true;
                    break;
                }
            }

        }
        if (!b_found) {
            DBMODEL_DEBUGM("The key <%s> was not found in related "
                           "<%s> model\n",
                           TMP_A(key.toString()),
                           TMP_A(table_->tableName()));
            break;
        }

        b_ret = true;
        break;
    }
    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModelCol::comboInsert (
        DbModel * top_model, const QString & value) const
{

    DBMODEL_TRACE_ENTRY;
    QVariant result;
    for (;;) {
        // we're going to insert a new record in target model
        // then we will save that record's id
        DbRecMap map;
        map.insert(original_.foreign_ref_, value);
        DbRecord * rec = table_->metadata()->createDefaultRecord ();
        rec->retrieve (map);
        if (!rec->save (table_->metadata(), top_model->database()->database())) {
            delete rec;
            DBMODEL_DEBUGM("Could not save new value in "
                           "reference table\n");
            break;
        }

        // get back the id
        map = rec->toMap ();
        result = map.value (original_.foreign_key_);

        // re-select the model to acknoledge the new value
        table_->sqlModel()->select();

        break;
    }

    DBMODEL_TRACE_EXIT;
    return result;

}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant DbModelCol::comboResult (
        DbModel * top_model, QComboBox *control) const
{
    DBMODEL_TRACE_ENTRY;
    QVariant result;
    for (;;) {
        // the table must be valid
        if (!table_->isValid())
            return result;
        int crt_idx = control->currentIndex();
        if (crt_idx == -1) {
            // this is a new one
            result = DbModelCol::comboInsert (
                    top_model, control->currentText());
            break;
        }

#if 1
        // see if the user is able to modify the source table
        if (original_.foreign_behaviour_ == DbColumn::FB_CHOOSE_ADD) {
            QString user_text = control->currentText();
            QString prev_text = control->itemText (crt_idx);

            // did the user actually added a new thingy?
            QVariant udata = control->itemData (crt_idx);
            if (!udata.isValid() || (user_text != prev_text)) {
                // this is a new one
                result = DbModelCol::comboInsert (
                        top_model, control->currentText());
                break;
            }
        }

        // just save the key in main model if this is an existing value
        QSqlRecord rec = table_->sqlModel()->record (crt_idx);
        result = rec.value (t_primary_);

#else
        // see if the user is able to modify the source table
        if (original_.foreign_behaviour_ == DbColumn::FB_CHOOSE_ADD) {

            // did the user actually added a new thingy?
            QComboBox::InsertPolicy ip = control->insertPolicy();
            if (crt_idx != -1) {
                QString s_crt_txt = control->currentText();
                QString s_prev = table_->model->data(table_->model->index(crt_idx, t_display_)).toString();
                if (s_crt_txt != control->itemText (crt_idx)) {
                    crt_idx = -1;
                }
                if (s_crt_txt != s_prev) {
                    crt_idx = -1;
                }
            }
            control->setInsertPolicy (QComboBox::InsertAtBottom);
            if (crt_idx == -1) {

                // we're going to insert a new record in target model
                // then we will save that record's id
                DbRecMap map;
                map.insert(original_.foreign_key_, control->currentText());
                DbRecord * rec = table_->meta->createDefaultRecord ();
                rec->retrieve (map);
                if (!rec->save (table_->meta, top_model->database()->database())) {
                    delete rec;
                    DBMODEL_DEBUGM("TCould not save new value in "
                                   "reference table\n");
                    break;
                }

                // get back the id
                map = rec->toMap ();
                result = map.value (original_.foreign_key_);

                // re-select the model to acknoledge the new value
                table_->model->select();

                // thanks for temporary object
                delete rec;
                break;
            }
        }

        // just save the key in main model if this is an existing value
        QSqlRecord rec = table_->model->record (crt_idx);
        result = rec.value (t_primary_);

#endif
        break;
    }

    DBMODEL_TRACE_EXIT;
    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelCol::getComboValue (
        const QModelIndex &index, DbModel * top_model,
        QComboBox *control) const
{
    DBMODEL_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {
        QVariant result = comboResult (top_model, control);
        if (!result.isValid())
            break;
        top_model->setData (index, result, Qt::EditRole);
        b_ret = true;
        break;
    }
    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool DbModelCol::getTristateValue (
        const QModelIndex &index, DbModel * top_model,
        QCheckBox *control) const
{
    DBMODEL_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {
        top_model->setData (
                    index, (int)control->checkState(),
                    Qt::EditRole);
        b_ret = true;
        break;
    }
    DBMODEL_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */



/*  CLASS    =============================================================== */
//
//
//
//
void DbModelCol::anchorVtable() const {}
/* ------------------------------------------------------------------------- */
/* ========================================================================= */

