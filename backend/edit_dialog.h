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

class EditDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditDialog(const QSqlRecord& record, QWidget* parent = nullptr);

    QSqlRecord GetUpdatedRecord() const;

private:
    QSqlRecord record_;
    QVector<QLineEdit*> fields_;
};

#endif // EDIT_DIALOG_H
