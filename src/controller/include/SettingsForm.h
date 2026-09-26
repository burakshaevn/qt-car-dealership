#pragma once

#ifndef SETTINGS_FORM_H
#define SETTINGS_FORM_H

#include "UiKit.h"

class AppServices;
class QComboBox;
class QLineEdit;

/*!
 * \brief Profile and appearance settings.
 *
 * Profile fields are shown for customers only; the theme selector is available
 * to everyone. The theme is applied immediately and persisted by ThemeManager.
 */
class SettingsForm final : public FormDialog
{
    Q_OBJECT
public:
    explicit SettingsForm(AppServices& services, QWidget* parent = nullptr);

signals:
    void profileSaved(const QString& fullName, const QString& email);

private:
    QString save();

    AppServices& m_services;
    bool m_editProfile = false;
    QString m_initialTheme;
    QLineEdit* m_firstName = nullptr;
    QLineEdit* m_lastName = nullptr;
    QLineEdit* m_email = nullptr;
    QLineEdit* m_phone = nullptr;
    QLineEdit* m_password = nullptr;
    QLineEdit* m_confirmation = nullptr;
    QComboBox* m_theme = nullptr;
};

#endif // SETTINGS_FORM_H
