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
    , m_services(services)
{
    buildUi();
    loadData();
}

void SettingsForm::buildUi()
{
    setWindowTitle("Настройки профиля");
    setFixedSize(450, 600);
    applyThemeStyle(this, "DialogForm");

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

    addLabeledField("Имя:", m_firstNameEdit);
    addLabeledField("Фамилия:", m_lastNameEdit);
    addLabeledField("Email:", m_emailEdit);
    addLabeledField("Телефон:", m_phoneEdit);
    addLabeledField("Пароль:", m_passwordEdit);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Введите новый пароль или оставьте пустым");
    dialogLayout->addWidget(new QLabel("Тема:", this));
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem("Светлая", "light");
    m_themeCombo->addItem("Тёмная", "dark");
    dialogLayout->addWidget(m_themeCombo);

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
    connect(saveButton, &QPushButton::clicked, this, &SettingsForm::onSaveClicked);
}

bool SettingsForm::loadData()
{
    if (!m_services || !m_services->getDatabase() || !m_services->getUserSession()->isAuthorized()) {
        return false;
    }

    QSqlQuery query;
    const QString kQueryStr = QString("SELECT first_name, last_name, email, phone "
                                      "FROM clients WHERE id = %1")
                                  .arg(m_services->getUserSession()->getId());

    if (!query.exec(kQueryStr) || !query.next()) {
        return false;
    }

    m_firstNameEdit->setText(query.value("first_name").toString());
    m_lastNameEdit->setText(query.value("last_name").toString());
    m_emailEdit->setText(query.value("email").toString());
    m_phoneEdit->setText(query.value("phone").toString());
    m_passwordEdit->clear();
    if (m_themeCombo) {
        const bool kDark = (getCurrentThemeMode() == ThemeMode::Dark);
        m_themeCombo->setCurrentIndex(kDark ? 1 : 0);
    }
    return true;
}

void SettingsForm::onSaveClicked()
{
    if (!m_services || !m_services->getDatabase() || !m_services->getUserSession()->isAuthorized()) {
        QMessageBox::warning(this, "Ошибка", "Сессия пользователя недоступна.");
        return;
    }

    const QString kFirstName = m_firstNameEdit->text().trimmed();
    const QString kLastName = m_lastNameEdit->text().trimmed();
    const QString kEmail = m_emailEdit->text().trimmed();
    const QString kPhone = m_phoneEdit->text().trimmed();
    const QString kPassword = m_passwordEdit->text();

    if (kFirstName.isEmpty() || kLastName.isEmpty() || kEmail.isEmpty() || kPhone.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Все поля кроме пароля должны быть заполнены.");
        return;
    }

    QString updateQuery = QString(
        "UPDATE clients SET "
        "first_name = '%1', "
        "last_name = '%2', "
        "email = '%3', "
        "phone = '%4'")
        .arg(kFirstName)
        .arg(kLastName)
        .arg(kEmail)
        .arg(kPhone);

    if (!kPassword.isEmpty()) {
        const QString kHashedPassword = QString(QCryptographicHash::hash(
            kPassword.toUtf8(),
            QCryptographicHash::Sha256).toHex());
        updateQuery += QString(", password = '%1'").arg(kHashedPassword);
    }

    updateQuery += QString(" WHERE id = %1").arg(m_services->getUserSession()->getId());

    if (!m_services->getDatabase()->executeQuery(updateQuery)) {
        QMessageBox::critical(this,
                              "Ошибка",
                              "Не удалось обновить данные профиля: "
                                  + m_services->getDatabase()->getLastError());
        return;
    }

    const QString kFullName = kFirstName + " " + kLastName;
    m_services->getUserSession()->setName(kFullName);
    m_services->getUserSession()->setEmail(kEmail);
    if (m_themeCombo) {
        const bool kDarkEnabled = m_themeCombo->currentData().toString() == "dark";
        emit themeChanged(kDarkEnabled);
    }

    emit profileSaved(kFullName, kEmail);
    QMessageBox::information(this, "Успех", "Данные профиля обновлены.");
    accept();
}
