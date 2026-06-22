#pragma once

#ifndef ADMIN_RECORD_EDIT_DIALOG_H
#define ADMIN_RECORD_EDIT_DIALOG_H

#include <QSqlRecord>
#include <QDialog>
#include <QVector>

class QLineEdit;

/*!
 * \brief Dialog for editing records
 */
class AdminRecordEditDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AdminRecordEditDialog(const QSqlRecord& record, QWidget* parent = nullptr);

    /*!
     * \brief Get updated record
     * \return Updated record
     */
    QSqlRecord GetUpdatedRecord() const;

private:
    QSqlRecord record_;         ///< Record to edit
    QVector<QLineEdit*> fields_;  ///< Fields to edit
};

#endif // ADMIN_RECORD_EDIT_DIALOG_H
