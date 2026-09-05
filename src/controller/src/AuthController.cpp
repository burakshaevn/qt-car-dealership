#include "AuthController.h"

#include "DatabaseHandler.h"
#include "ThemeStyleProvider.h"

#include <QCalendarWidget>
#include <QCryptographicHash>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimeEdit>
#include <QVBoxLayout>

AuthController::AuthController(QObject* parent)
    : QObject(parent)
{
}

void AuthController::setDependencies(const QSharedPointer<DatabaseHandler>& database)
{
    m_database = database;
}

AuthController::AuthResult AuthController::login(const QString& login, const QString& password) const
{
    AuthResult result;
    if (!m_database) {
        result.Error = "База данных недоступна.";
        return result;
    }

    QSqlQuery query = m_database->executeNamedSelect(SqlQueryId::SelectAdminByUsername,
                                                     {{"username", login}});
    if (query.isActive()) {
        if (query.next()) {
            UserInfo user;
            user.Id = query.value("id").toInt();
            user.Password = query.value("password").toString();
            user.Role = Role::Admin;
            if (user.Password == password) {
                result.Ok = true;
                result.User = user;
            } else {
                result.Error = "Неверный логин или пароль.";
            }
            return result;
        }
    }

    query = m_database->executeNamedSelect(SqlQueryId::SelectClientByEmail, {{"email", login}});
    if (query.isActive()) {
        if (query.next()) {
            UserInfo user;
            user.Id = query.value("id").toInt();
            user.FullName = query.value("first_name").toString();
            user.FullName += " " + query.value("last_name").toString();
            user.Email = query.value("email").toString();
            user.Password = query.value("password").toString();
            user.Role = Role::User;

            QString hashedInputPassword = QString(QCryptographicHash::hash(
                password.toUtf8(),
                QCryptographicHash::Sha256).toHex());

            if (user.Password == hashedInputPassword) {
                result.Ok = true;
                result.User = user;
            } else {
                result.Error = "Неверный логин или пароль.";
            }
            return result;
        }
    }

    result.Error = "Неверный логин или пароль.";
    return result;
}

bool AuthController::runRegistrationDialog(QWidget* parent)
{
    if (!m_database) {
        QMessageBox::warning(parent, "Ошибка", "База данных недоступна.");
        return false;
    }

    QDialog dialog(parent);
    dialog.setWindowTitle("Регистрация");
    dialog.setFixedSize(500, 800);
    applyThemeStyle(&dialog, "DialogForm");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* titleLabel = new QLabel("Создание учетной записи", &dialog);
    titleLabel->setProperty("type", "header");
    layout->addWidget(titleLabel);

    QLabel* firstNameLabel = new QLabel("Имя:", &dialog);
    layout->addWidget(firstNameLabel);
    QLineEdit* firstNameEdit = new QLineEdit(&dialog);
    firstNameEdit->setPlaceholderText("Введите имя");
    layout->addWidget(firstNameEdit);

    QLabel* lastNameLabel = new QLabel("Фамилия:", &dialog);
    layout->addWidget(lastNameLabel);
    QLineEdit* lastNameEdit = new QLineEdit(&dialog);
    lastNameEdit->setPlaceholderText("Введите фамилию");
    layout->addWidget(lastNameEdit);

    QLabel* phoneLabel = new QLabel("Телефон:", &dialog);
    layout->addWidget(phoneLabel);
    QLineEdit* phoneEdit = new QLineEdit(&dialog);
    phoneEdit->setPlaceholderText("+7XXXXXXXXXX");
    layout->addWidget(phoneEdit);

    QLabel* emailLabel = new QLabel("Email:", &dialog);
    layout->addWidget(emailLabel);
    QLineEdit* emailEdit = new QLineEdit(&dialog);
    emailEdit->setPlaceholderText("example@domain.com");
    layout->addWidget(emailEdit);

    QLabel* passwordLabel = new QLabel("Пароль:", &dialog);
    layout->addWidget(passwordLabel);
    QLineEdit* passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setPlaceholderText("Минимум 8 символов");
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);

    QLabel* confirmPasswordLabel = new QLabel("Подтвердите пароль:", &dialog);
    layout->addWidget(confirmPasswordLabel);
    QLineEdit* confirmPasswordEdit = new QLineEdit(&dialog);
    confirmPasswordEdit->setPlaceholderText("Повторите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(confirmPasswordEdit);

    layout->addStretch();

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* registerButton = new QPushButton("Зарегистрироваться", &dialog);
    registerButton->setProperty("type", "primary");
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    cancelButton->setProperty("type", "secondary");

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(registerButton);
    layout->addLayout(buttonLayout);

    bool accepted = false;

    connect(registerButton, &QPushButton::clicked, [&]() {
        if (firstNameEdit->text().isEmpty() || lastNameEdit->text().isEmpty() ||
            phoneEdit->text().isEmpty() || emailEdit->text().isEmpty() ||
            passwordEdit->text().isEmpty() || confirmPasswordEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Все поля должны быть заполнены.");
            return;
        }

        if (passwordEdit->text().length() < 8) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароль должен содержать минимум 8 символов.");
            return;
        }

        if (passwordEdit->text() != confirmPasswordEdit->text()) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароли не совпадают.");
            return;
        }

        QRegularExpression emailRegex(R"((\w+)(\.\w+)*@(\w+)(\.\w{2,})+)");
        if (!emailRegex.match(emailEdit->text()).hasMatch()) {
            QMessageBox::warning(&dialog, "Ошибка", "Введите корректный email адрес.");
            return;
        }

        QString phone = phoneEdit->text();
        if (phone.startsWith("+")) {
            phone = phone.mid(1);
        }
        QRegularExpression phoneRegex("^[0-9]{11}$");
        if (!phoneRegex.match(phone).hasMatch()) {
            QMessageBox::warning(&dialog, "Ошибка", "Введите корректный номер телефона (11 цифр).");
            return;
        }

        QSqlQuery checkQuery = m_database->executeNamedSelect(SqlQueryId::SelectClientByEmailOrPhone,
                                                              {{"email", emailEdit->text()},
                                                               {"phone", phone}});

        if (checkQuery.isActive() && checkQuery.next()) {
            QMessageBox::warning(&dialog, "Ошибка", "Пользователь с таким email или телефоном уже существует.");
            return;
        }

        QString hashedPassword = QString(QCryptographicHash::hash(
            passwordEdit->text().toUtf8(),
            QCryptographicHash::Sha256).toHex());

        QString databaseError;
        if (m_database->executeNamedQuery(SqlQueryId::InsertClient,
                                          {{"first_name", firstNameEdit->text()},
                                           {"last_name", lastNameEdit->text()},
                                           {"phone", phone},
                                           {"email", emailEdit->text()},
                                           {"password", hashedPassword}},
                                          &databaseError)) {
            QMessageBox::information(&dialog, "Успех",
                                     "Регистрация успешно завершена.\nТеперь вы можете войти в систему, используя email и пароль.");
            accepted = true;
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog,
                                  "Ошибка",
                                  "Не удалось создать учетную запись: " + databaseError);
        }
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    return dialog.exec() == QDialog::Accepted && accepted;
}
