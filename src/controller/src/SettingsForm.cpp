#include "SettingsForm.h"

#include "AppServices.h"
#include "AuthController.h"
#include "ThemeManager.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>

SettingsForm::SettingsForm(AppServices& services, QWidget* parent)
    : FormDialog(tr("Настройки"), tr("Личные данные и внешний вид приложения."), parent)
    , m_services(services)
    , m_editProfile(services.session().isUser())
    , m_initialTheme(ThemeManager::instance().theme())
{
    setAcceptText(tr("Сохранить"));

    if (m_editProfile) {
        m_firstName = new QLineEdit;
        m_lastName = new QLineEdit;
        auto* nameRow = new QWidget;
        auto* nameLayout = new QHBoxLayout(nameRow);
        nameLayout->setContentsMargins(0, 0, 0, 0);
        nameLayout->setSpacing(12);
        nameLayout->addWidget(UiKit::field(tr("Имя"), m_firstName));
        nameLayout->addWidget(UiKit::field(tr("Фамилия"), m_lastName));
        addWidget(nameRow);

        m_email = new QLineEdit;
        addField(tr("Email"), m_email);
        m_phone = new QLineEdit;
        addField(tr("Телефон"), m_phone);

        m_password = new QLineEdit;
        m_password->setEchoMode(QLineEdit::Password);
        m_password->setPlaceholderText(tr("Оставьте пустым, чтобы не менять"));
        addField(tr("Новый пароль"), m_password);
        m_confirmation = new QLineEdit;
        m_confirmation->setEchoMode(QLineEdit::Password);
        addField(tr("Повторите пароль"), m_confirmation);

        if (const auto kProfile = m_services.clients().profile(m_services.session().id())) {
            m_firstName->setText(kProfile->FirstName);
            m_lastName->setText(kProfile->LastName);
            m_email->setText(kProfile->Email);
            m_phone->setText(kProfile->Phone);
        }
        addSpacing(6);
        addWidget(UiKit::divider());
        addSpacing(6);
    }

    ThemeManager& theme = ThemeManager::instance();
    m_theme = new QComboBox;
    for (const QString& name : theme.availableThemes()) {
        m_theme->addItem(theme.themeTitle(name), name);
    }
    m_theme->setCurrentIndex(m_theme->findData(m_initialTheme));
    addField(tr("Тема оформления"), m_theme);

    // Live preview; reverted on cancel.
    connect(m_theme, &QComboBox::currentIndexChanged, this, [this] {
        ThemeManager::instance().setTheme(m_theme->currentData().toString());
    });
    connect(this, &QDialog::rejected, this, [this] { ThemeManager::instance().setTheme(m_initialTheme); });

    setValidator([this] { return save(); });
}

QString SettingsForm::save()
{
    if (!m_editProfile) {
        return {};
    }

    const QString kFirstName = m_firstName->text().trimmed();
    const QString kLastName = m_lastName->text().trimmed();
    const QString kEmail = m_email->text().trimmed();
    const QString kPhone = AuthController::normalizePhone(m_phone->text());

    QString error = AuthController::validateProfile(kFirstName, kLastName, kEmail, kPhone);
    if (error.isEmpty() && !m_password->text().isEmpty()) {
        error = AuthController::validatePassword(m_password->text(), m_confirmation->text());
    }
    if (!error.isEmpty()) {
        return error;
    }

    const int kClientId = m_services.session().id();
    if (m_services.clients().isEmailOrPhoneTaken(kEmail, kPhone, kClientId)) {
        return tr("Этот email или телефон уже используется другим пользователем.");
    }
    if (!m_services.clients().updateProfile(kClientId, {kFirstName, kLastName, kEmail, kPhone}, m_password->text(),
                                            &error)) {
        return error;
    }

    const QString kFullName = kFirstName + QLatin1Char(' ') + kLastName;
    m_services.session().setName(kFullName);
    m_services.session().setEmail(kEmail);
    emit profileSaved(kFullName, kEmail);
    return {};
}
