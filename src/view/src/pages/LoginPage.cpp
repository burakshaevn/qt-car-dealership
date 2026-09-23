#include "pages/LoginPage.h"

#include "ThemeManager.h"
#include "UiKit.h"

#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginPage::LoginPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("loginPage"));

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---- Hero panel
    auto* heroPanel = new QFrame(this);
    heroPanel->setObjectName(QStringLiteral("loginHero"));
    auto* heroLayout = new QVBoxLayout(heroPanel);
    heroLayout->setContentsMargins(56, 48, 56, 48);
    heroLayout->setSpacing(0);

    auto* wordmark = new QLabel(heroPanel);
    wordmark->setObjectName(QStringLiteral("wordmark"));
    wordmark->setPixmap(ThemeManager::instance().icon(QStringLiteral("mercedez_benz")).pixmap(QSize(230, 27)));
    heroLayout->addWidget(wordmark, 0, Qt::AlignLeft);
    heroLayout->addStretch(1);

    m_hero = new QLabel(heroPanel);
    m_hero->setAlignment(Qt::AlignCenter);
    m_hero->setMinimumHeight(220);
    m_hero->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    m_hero->installEventFilter(this);
    heroLayout->addWidget(m_hero, 3);
    heroLayout->addSpacing(24);

    auto* heroTitle = UiKit::label(tr("Автомобиль мечты —\nв несколько кликов"), "h1", heroPanel);
    heroLayout->addWidget(heroTitle);
    heroLayout->addSpacing(10);
    auto* heroText = UiKit::label(
        tr("Каталог в наличии и под заказ, кредит, аренда, страхование и тест-драйв — всё в одном месте."),
        "subtitle", heroPanel);
    heroText->setWordWrap(true);
    heroText->setMaximumWidth(520);
    heroLayout->addWidget(heroText);
    heroLayout->addStretch(1);

    root->addWidget(heroPanel, 3);

    // ---- Form panel
    auto* formPanel = new QWidget(this);
    auto* formOuter = new QVBoxLayout(formPanel);
    formOuter->setContentsMargins(40, 40, 40, 40);
    formOuter->addStretch(1);

    auto* card = UiKit::card(formPanel);
    card->setObjectName(QStringLiteral("loginCard"));
    card->setMinimumWidth(380);
    card->setMaximumWidth(420);
    auto* form = new QVBoxLayout(card);
    form->setContentsMargins(36, 36, 36, 32);
    form->setSpacing(0);

    form->addWidget(UiKit::label(tr("Вход"), "h2", card));
    form->addSpacing(6);
    form->addWidget(UiKit::label(tr("Войдите по email или логину администратора"), "muted", card));
    form->addSpacing(28);

    m_login = new QLineEdit(card);
    m_login->setObjectName(QStringLiteral("lineEdit_login"));
    m_login->setPlaceholderText(tr("name@example.com"));
    m_login->setClearButtonEnabled(true);
    form->addWidget(UiKit::field(tr("Email или логин"), m_login, card));
    form->addSpacing(16);

    m_password = new QLineEdit(card);
    m_password->setObjectName(QStringLiteral("lineEdit_password"));
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setPlaceholderText(tr("Пароль"));
    form->addWidget(UiKit::field(tr("Пароль"), m_password, card));

    m_error = UiKit::label({}, "error", card);
    m_error->setWordWrap(true);
    m_error->hide();
    form->addSpacing(12);
    form->addWidget(m_error);
    form->addSpacing(12);

    m_submit = UiKit::button(tr("Войти"), "primary", card);
    m_submit->setObjectName(QStringLiteral("pushButton_login"));
    m_submit->setMinimumHeight(44);
    m_submit->setDefault(true);
    form->addWidget(m_submit);
    form->addSpacing(18);

    auto* registerRow = new QHBoxLayout;
    registerRow->setSpacing(4);
    registerRow->addStretch(1);
    registerRow->addWidget(UiKit::label(tr("Нет аккаунта?"), "muted", card));
    auto* registerButton = UiKit::button(tr("Зарегистрироваться"), "link", card);
    registerButton->setObjectName(QStringLiteral("pushButton_registration"));
    registerRow->addWidget(registerButton);
    registerRow->addStretch(1);
    form->addLayout(registerRow);

    formOuter->addWidget(card, 0, Qt::AlignHCenter);
    formOuter->addStretch(1);
    root->addWidget(formPanel, 2);

    connect(m_submit, &QPushButton::clicked, this, &LoginPage::submit);
    connect(m_login, &QLineEdit::returnPressed, m_password, qOverload<>(&QWidget::setFocus));
    connect(m_password, &QLineEdit::returnPressed, this, &LoginPage::submit);
    connect(registerButton, &QPushButton::clicked, this, &LoginPage::registrationRequested);
    connect(m_login, &QLineEdit::textChanged, this, [this] { showError({}); });
    connect(m_password, &QLineEdit::textChanged, this, [this] { showError({}); });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [wordmark] {
        wordmark->setPixmap(ThemeManager::instance().icon(QStringLiteral("mercedez_benz")).pixmap(QSize(230, 27)));
    });
}

QString LoginPage::login() const
{
    return m_login->text().trimmed();
}

QString LoginPage::password() const
{
    return m_password->text();
}

void LoginPage::setHeroImage(const QString& path)
{
    m_heroPath = path;
    updateHero();
}

void LoginPage::updateHero()
{
    const QPixmap kSource(m_heroPath);
    if (kSource.isNull() || m_hero->width() <= 0) {
        m_hero->clear();
        return;
    }
    const qreal kDpr = devicePixelRatioF();
    const QSize kBox = QSize(qMin(m_hero->width(), 760), qMin(m_hero->height(), 420)) * kDpr;
    QPixmap scaled = kSource.scaled(kBox, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(kDpr);
    m_hero->setPixmap(scaled);
}

bool LoginPage::eventFilter(QObject* watched, QEvent* event)
{
    // Scale when the label itself gets its final geometry, not when the page does.
    if (m_hero && watched == m_hero && event->type() == QEvent::Resize) {
        updateHero();
    }
    return QWidget::eventFilter(watched, event);
}

void LoginPage::showError(const QString& message)
{
    m_error->setText(message);
    m_error->setVisible(!message.isEmpty());
    m_password->setProperty("invalid", !message.isEmpty());
    ThemeManager::repolish(m_password);
}

void LoginPage::clear()
{
    m_login->clear();
    m_password->clear();
    showError({});
}

void LoginPage::setBusy(const bool busy)
{
    m_submit->setEnabled(!busy);
}

void LoginPage::submit()
{
    if (login().isEmpty() || password().isEmpty()) {
        showError(tr("Введите логин и пароль."));
        return;
    }
    emit loginRequested(login(), password());
}
