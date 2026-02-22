#pragma once

#ifndef SETTINGS_FORM_H
#define SETTINGS_FORM_H

#include <QDialog>

class AppServices;
class QLineEdit;

class SettingsForm : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsForm(AppServices* services, QWidget* parent = nullptr);

signals:
    void ProfileSaved(const QString& fullName, const QString& email);

private slots:
    void OnSaveClicked();

private:
    void BuildUi();
    bool LoadData();

    AppServices* services_ = nullptr;
    QLineEdit* first_name_edit_ = nullptr;
    QLineEdit* last_name_edit_ = nullptr;
    QLineEdit* email_edit_ = nullptr;
    QLineEdit* phone_edit_ = nullptr;
    QLineEdit* password_edit_ = nullptr;
};

#endif // SETTINGS_FORM_H
