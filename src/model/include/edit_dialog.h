#pragma once

#ifndef EDIT_DIALOG_H
#define EDIT_DIALOG_H

#include <QSqlRecord>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

/*!
 * \brief Dialog for editing records
 */
class EditDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditDialog(const QSqlRecord& record, QWidget* parent = nullptr);

    /*!
     * \brief Get updated record
     * \return Updated record
     */
    QSqlRecord GetUpdatedRecord() const;

private:
    QSqlRecord record_; ///< Record to edit
    QVector<QLineEdit*> fields_; ///< Fields to edit
};

#endif // EDIT_DIALOG_H
