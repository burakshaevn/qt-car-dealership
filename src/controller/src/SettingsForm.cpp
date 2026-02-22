#include "SettingsForm.h"

#include <QCryptographicHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QSqlQuery>
#include <QVBoxLayout>

#include "AppServices.h"
#include "ThemeStyleProvider.h"

SettingsForm::SettingsForm(AppServices* services, QWidget* parent)
    : QDialog(parent)
    , services_(services)
{
    BuildUi();
    LoadData();
}

void SettingsForm::BuildUi()
{
    setWindowTitle("Настройки профиля");
    setFixedSize(450, 600);
    ApplyThemeStyle(this, "DialogForm");

    auto* dialogLayout = new QVBoxLayout(this);
    dialogLayout->setSpacing(10);
    dialogLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("Редактирование профиля", this);
    titleLabel->setProperty("type", "header");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    dialogLayout->addWidget(titleLabel);

    auto addLabeledField = [this, dialogLayout](const QString& label, QLineEdit*& edit) {
        dialogLayout->addWidget(new QLabel(label, this));
        edit = new QLineEdit(this);
        dialogLayout->addWidget(edit);
    };

    addLabeledField("Имя:", first_name_edit_);
    addLabeledField("Фамилия:", last_name_edit_);
    addLabeledField("Email:", email_edit_);
    addLabeledField("Телефон:", phone_edit_);
    addLabeledField("Пароль:", password_edit_);
    password_edit_->setEchoMode(QLineEdit::Password);
    password_edit_->setPlaceholderText("Введите новый пароль или оставьте пустым");
    dialogLayout->addWidget(new QLabel("Тема:", this));
    theme_combo_ = new QComboBox(this);
    theme_combo_->addItem("Светлая", "light");
    theme_combo_->addItem("Тёмная", "dark");
    dialogLayout->addWidget(theme_combo_);

    dialogLayout->addStretch();

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    auto* cancelButton = new QPushButton("Отмена", this);
    cancelButton->setProperty("type", "secondary");
    auto* saveButton = new QPushButton("Сохранить", this);
    saveButton->setProperty("type", "primary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);
    dialogLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, &SettingsForm::OnSaveClicked);
}

bool SettingsForm::LoadData()
{
    if (!services_ || !services_->GetDatabase() || !services_->GetUserSession()->IsAuthorized()) {
        return false;
    }

    QSqlQuery query;
    const QString queryStr = QString(
        "SELECT first_name, last_name, email, phone "
        "FROM clients WHERE id = %1")
        .arg(services_->GetUserSession()->GetId());

    if (!query.exec(queryStr) || !query.next()) {
        return false;
    }

    first_name_edit_->setText(query.value("first_name").toString());
    last_name_edit_->setText(query.value("last_name").toString());
    email_edit_->setText(query.value("email").toString());
    phone_edit_->setText(query.value("phone").toString());
    password_edit_->clear();
    if (theme_combo_) {
        const bool dark = (GetCurrentThemeMode() == ThemeMode::Dark);
        theme_combo_->setCurrentIndex(dark ? 1 : 0);
    }
    return true;
}

void SettingsForm::OnSaveClicked()
{
    if (!services_ || !services_->GetDatabase() || !services_->GetUserSession()->IsAuthorized()) {
        QMessageBox::warning(this, "Ошибка", "Сессия пользователя недоступна.");
        return;
    }

    const QString firstName = first_name_edit_->text().trimmed();
    const QString lastName = last_name_edit_->text().trimmed();
    const QString email = email_edit_->text().trimmed();
    const QString phone = phone_edit_->text().trimmed();
    const QString password = password_edit_->text();

    if (firstName.isEmpty() || lastName.isEmpty() || email.isEmpty() || phone.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Все поля кроме пароля должны быть заполнены.");
        return;
    }

    QString updateQuery = QString(
        "UPDATE clients SET "
        "first_name = '%1', "
        "last_name = '%2', "
        "email = '%3', "
        "phone = '%4'")
        .arg(firstName)
        .arg(lastName)
        .arg(email)
        .arg(phone);

    if (!password.isEmpty()) {
        const QString hashedPassword = QString(QCryptographicHash::hash(
            password.toUtf8(),
            QCryptographicHash::Sha256).toHex());
        updateQuery += QString(", password = '%1'").arg(hashedPassword);
    }

    updateQuery += QString(" WHERE id = %1").arg(services_->GetUserSession()->GetId());

    if (!services_->GetDatabase()->ExecuteQuery(updateQuery)) {
        QMessageBox::critical(
            this,
            "Ошибка",
            "Не удалось обновить данные профиля: " + services_->GetDatabase()->GetLastError());
        return;
    }

    const QString fullName = firstName + " " + lastName;
    services_->GetUserSession()->SetName(fullName);
    services_->GetUserSession()->SetEmail(email);
    if (theme_combo_) {
        const bool darkEnabled = theme_combo_->currentData().toString() == "dark";
        emit ThemeChanged(darkEnabled);
    }

    emit ProfileSaved(fullName, email);
    QMessageBox::information(this, "Успех", "Данные профиля обновлены.");
    accept();
}
