#include "SettingsForm.h"

#include <QCryptographicHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QVBoxLayout>

#include "AppServices.h"

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
    setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #1d1b20;"
        "    font: 500 12pt 'JetBrains Mono';"
        "    min-height: 20px;"
        "    margin: 3px 0px;"
        "}"
        "QLabel[type='header'] {"
        "    font: 700 16pt 'JetBrains Mono';"
        "    min-height: 30px;"
        "    margin: 0px 0px 15px 0px;"
        "}"
        "QLineEdit {"
        "    padding: 5px 8px;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    background: #fafafa;"
        "    font: 11pt 'JetBrains Mono';"
        "    min-height: 16px;"
        "    margin-bottom: 10px;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #2196F3;"
        "}"
        "QPushButton {"
        "    padding: 8px 16px;"
        "    border-radius: 8px;"
        "    font: 600 11pt 'JetBrains Mono';"
        "    min-width: 90px;"
        "    min-height: 32px;"
        "}"
        "QPushButton[type='primary'] {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "}"
        "QPushButton[type='primary']:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton[type='secondary'] {"
        "    background-color: #fafafa;"
        "    color: #1d1b20;"
        "    border: 2px solid #e0e0e0;"
        "}"
        "QPushButton[type='secondary']:hover {"
        "    background-color: #e0e0e0;"
        "}");

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

    emit ProfileSaved(fullName, email);
    QMessageBox::information(this, "Успех", "Данные профиля обновлены.");
    accept();
}
