#include "AuthController.h"

#include "DatabaseHandler.h"
#include "ThemeStyleProvider.h"

#include <QCalendarWidget>
#include <QCryptographicHash>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
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

void AuthController::SetDependencies(const QSharedPointer<DatabaseHandler>& database)
{
    database_ = database;
}

AuthController::AuthResult AuthController::Login(const QString& login, const QString& password) const
{
    AuthResult result;
    if (!database_) {
        result.error = "База данных недоступна.";
        return result;
    }

    auto adminResult = database_->ExecuteSelectQuery(QString("SELECT * FROM public.admins WHERE username = '%1';").arg(login));
    if (adminResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = adminResult.value<QSqlQuery>();
        if (query.next()) {
            UserInfo user;
            user.id_ = query.value("id").toInt();
            user.password_ = query.value("password").toString();
            user.role_ = Role::Admin;
            if (user.password_ == password) {
                result.ok = true;
                result.user = user;
            } else {
                result.error = "Неверный логин или пароль.";
            }
            return result;
        }
    }

    auto clientResult = database_->ExecuteSelectQuery(QString("SELECT * FROM public.clients WHERE email = '%1';").arg(login));
    if (clientResult.canConvert<QSqlQuery>()) {
        QSqlQuery query = clientResult.value<QSqlQuery>();
        if (query.next()) {
            UserInfo user;
            user.id_ = query.value("id").toInt();
            user.full_name_ = query.value("first_name").toString();
            user.full_name_ += " " + query.value("last_name").toString();
            user.email_ = query.value("email").toString();
            user.password_ = query.value("password").toString();
            user.role_ = Role::User;

            QString hashedInputPassword = QString(QCryptographicHash::hash(
                password.toUtf8(),
                QCryptographicHash::Sha256).toHex());

            if (user.password_ == hashedInputPassword) {
                result.ok = true;
                result.user = user;
            } else {
                result.error = "Неверный логин или пароль.";
            }
            return result;
        }
    }

    result.error = "Неверный логин или пароль.";
    return result;
}

bool AuthController::RunRegistrationDialog(QWidget* parent)
{
    if (!database_) {
        QMessageBox::warning(parent, "Ошибка", "База данных недоступна.");
        return false;
    }

    QDialog dialog(parent);
    dialog.setWindowTitle("Регистрация");
    dialog.setFixedSize(500, 800);
    ApplyThemeStyle(&dialog, "DialogForm");

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

        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id FROM clients WHERE email = :email OR phone = :phone");
        checkQuery.bindValue(":email", emailEdit->text());
        checkQuery.bindValue(":phone", phone);

        if (checkQuery.exec() && checkQuery.next()) {
            QMessageBox::warning(&dialog, "Ошибка", "Пользователь с таким email или телефоном уже существует.");
            return;
        }

        QString hashedPassword = QString(QCryptographicHash::hash(
            passwordEdit->text().toUtf8(),
            QCryptographicHash::Sha256).toHex());

        QSqlQuery query;
        query.prepare("INSERT INTO clients (first_name, last_name, phone, email, password) "
                     "VALUES (:first_name, :last_name, :phone, :email, :password)");
        query.bindValue(":first_name", firstNameEdit->text());
        query.bindValue(":last_name", lastNameEdit->text());
        query.bindValue(":phone", phone);
        query.bindValue(":email", emailEdit->text());
        query.bindValue(":password", hashedPassword);

        if (query.exec()) {
            QMessageBox::information(&dialog, "Успех",
                                     "Регистрация успешно завершена.\nТеперь вы можете войти в систему, используя email и пароль.");
            accepted = true;
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog, "Ошибка",
                                  "Не удалось создать учетную запись: " + query.lastError().text());
        }
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    return dialog.exec() == QDialog::Accepted && accepted;
}
