#include "AuthController.h"

#include "AppServices.h"
#include "UiKit.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QRegularExpression>

namespace {
constexpr int kMinPasswordLength = 8;
} // namespace

AuthController::AuthController(AppServices& services, QObject* parent)
    : QObject(parent)
    , m_services(services)
{}

QString AuthController::login(const QString& login, const QString& password)
{
    if (!m_services.isReady()) {
        return tr("База данных недоступна.");
    }
    const std::optional<UserInfo> kUser = m_services.clients().authenticate(login, password);
    if (!kUser) {
        return tr("Неверный логин или пароль.");
    }
    m_services.session().setCurrentUser(*kUser);
    return {};
}

QString AuthController::normalizePhone(const QString& phone)
{
    static const QRegularExpression kNonDigits(QStringLiteral("\\D"));
    QString digits = QString(phone).remove(kNonDigits);
    // Russian numbers are stored in the international form: 8XXXXXXXXXX -> 7XXXXXXXXXX.
    if (digits.size() == 11 && digits.startsWith(QLatin1Char('8'))) {
        digits[0] = QLatin1Char('7');
    }
    return digits;
}

QString AuthController::validateProfile(const QString& firstName,
                                        const QString& lastName,
                                        const QString& email,
                                        const QString& phone)
{
    if (firstName.trimmed().isEmpty() || lastName.trimmed().isEmpty() || email.trimmed().isEmpty()
        || phone.trimmed().isEmpty()) {
        return tr("Заполните все поля.");
    }
    static const QRegularExpression kEmail(QStringLiteral(R"(^[^@\s]+@[^@\s]+\.[^@\s]{2,}$)"));
    if (!kEmail.match(email.trimmed()).hasMatch()) {
        return tr("Введите корректный email.");
    }
    if (normalizePhone(phone).size() != 11) {
        return tr("Номер телефона должен содержать 11 цифр.");
    }
    return {};
}

QString AuthController::validatePassword(const QString& password, const QString& confirmation)
{
    if (password.size() < kMinPasswordLength) {
        return tr("Пароль должен содержать минимум %1 символов.").arg(kMinPasswordLength);
    }
    if (password != confirmation) {
        return tr("Пароли не совпадают.");
    }
    return {};
}

std::optional<QString> AuthController::runRegistrationDialog(QWidget* parent)
{
    if (!m_services.isReady()) {
        return std::nullopt;
    }

    FormDialog dialog(tr("Создание аккаунта"), tr("Заполните данные, чтобы оформлять заявки и следить за их статусом."),
                      parent);
    dialog.setAcceptText(tr("Зарегистрироваться"));

    auto* firstName = new QLineEdit;
    auto* lastName = new QLineEdit;
    auto* nameRow = new QWidget;
    auto* nameLayout = new QHBoxLayout(nameRow);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(12);
    nameLayout->addWidget(UiKit::field(tr("Имя"), firstName));
    nameLayout->addWidget(UiKit::field(tr("Фамилия"), lastName));
    dialog.addWidget(nameRow);

    auto* email = new QLineEdit;
    email->setPlaceholderText(QStringLiteral("name@example.com"));
    dialog.addField(tr("Email"), email);

    auto* phone = new QLineEdit;
    phone->setPlaceholderText(QStringLiteral("+7 900 000-00-00"));
    dialog.addField(tr("Телефон"), phone);

    auto* password = new QLineEdit;
    password->setEchoMode(QLineEdit::Password);
    password->setPlaceholderText(tr("Минимум %1 символов").arg(kMinPasswordLength));
    dialog.addField(tr("Пароль"), password);

    auto* confirmation = new QLineEdit;
    confirmation->setEchoMode(QLineEdit::Password);
    dialog.addField(tr("Повторите пароль"), confirmation);

    dialog.setValidator([&]() -> QString {
        QString error = validateProfile(firstName->text(), lastName->text(), email->text(), phone->text());
        if (error.isEmpty()) {
            error = validatePassword(password->text(), confirmation->text());
        }
        if (!error.isEmpty()) {
            return error;
        }
        const QString kPhone = normalizePhone(phone->text());
        if (m_services.clients().isEmailOrPhoneTaken(email->text().trimmed(), kPhone)) {
            return tr("Пользователь с таким email или телефоном уже существует.");
        }
        const ClientProfile kProfile{firstName->text().trimmed(), lastName->text().trimmed(),
                                     email->text().trimmed(), kPhone};
        QString dbError;
        if (!m_services.clients().registerClient(kProfile, password->text(), &dbError)) {
            return dbError;
        }
        return {};
    });

    if (dialog.exec() != QDialog::Accepted) {
        return std::nullopt;
    }
    return email->text().trimmed();
}
