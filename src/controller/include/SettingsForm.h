#pragma once

#ifndef SETTINGS_FORM_H
#define SETTINGS_FORM_H

#include <QDialog>

class AppServices;
class QLineEdit;
class QComboBox;

class SettingsForm : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsForm(AppServices* services, QWidget* parent = nullptr);

signals:
    void profileSaved(const QString& fullName, const QString& email);
    void themeChanged(bool darkEnabled);

private slots:
    void onSaveClicked();

private:
    void buildUi();
    bool loadData();

    AppServices* m_services = nullptr;
    QLineEdit* m_firstNameEdit = nullptr;
    QLineEdit* m_lastNameEdit = nullptr;
    QLineEdit* m_emailEdit = nullptr;
    QLineEdit* m_phoneEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QComboBox* m_themeCombo = nullptr;
};

#endif // SETTINGS_FORM_H
