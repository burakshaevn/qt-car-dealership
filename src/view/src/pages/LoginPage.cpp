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

    // ---- Left: the car on a plain plate with an editorial caption.
    auto* heroPanel = new QFrame(this);
    heroPanel->setObjectName(QStringLiteral("plate"));
    auto* heroLayout = new QVBoxLayout(heroPanel);
    heroLayout->setContentsMargins(56, 48, 56, 48);
    heroLayout->setSpacing(0);

    auto* wordmark = new QLabel(heroPanel);
    wordmark->setObjectName(QStringLiteral("wordmark"));
    heroLayout->addWidget(wordmark, 0, Qt::AlignLeft);
    heroLayout->addStretch(1);

    m_hero = new QLabel(heroPanel);
    m_hero->setAlignment(Qt::AlignCenter);
    m_hero->setMinimumHeight(240);
    m_hero->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    m_hero->installEventFilter(this);
    heroLayout->addWidget(m_hero, 4);
    heroLayout->addStretch(1);

    auto* caption = new QHBoxLayout;
    caption->setSpacing(0);
    auto* heroTitle = UiKit::label(tr("Автосалон\nMercedes-Benz"), "display", heroPanel);
    caption->addWidget(heroTitle, 0, Qt::AlignBottom);
    caption->addStretch(1);
    auto* heroText = UiKit::label(tr("Автомобили в наличии и под заказ.\nКредит, аренда, страхование, тест-драйв."),
                                  "muted", heroPanel);
    heroText->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    caption->addWidget(heroText, 0, Qt::AlignBottom);
    heroLayout->addLayout(caption);

    root->addWidget(heroPanel, 3);

    // ---- Right: form without a card, aligned to a fixed column.
    auto* formPanel = new QWidget(this);
    auto* formOuter = new QVBoxLayout(formPanel);
    formOuter->setContentsMargins(64, 48, 64, 48);
    formOuter->addStretch(1);

    auto* column = new QWidget(formPanel);
    column->setFixedWidth(340);
    auto* form = new QVBoxLayout(column);
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(0);

    form->addWidget(UiKit::overline(tr("Личный кабинет"), column));
    form->addSpacing(10);
    form->addWidget(UiKit::label(tr("Вход"), "h1", column));
    form->addSpacing(32);

    m_login = new QLineEdit(column);
    m_login->setObjectName(QStringLiteral("lineEdit_login"));
    m_login->setClearButtonEnabled(true);
    form->addWidget(UiKit::field(tr("Email или логин"), m_login, column));
    form->addSpacing(18);

    m_password = new QLineEdit(column);
    m_password->setObjectName(QStringLiteral("lineEdit_password"));
    m_password->setEchoMode(QLineEdit::Password);
    form->addWidget(UiKit::field(tr("Пароль"), m_password, column));

    m_error = UiKit::label({}, "error", column);
    m_error->setWordWrap(true);
    m_error->hide();
    form->addSpacing(12);
    form->addWidget(m_error);
    form->addSpacing(16);

    m_submit = UiKit::button(tr("Войти"), "primary", column);
    m_submit->setObjectName(QStringLiteral("pushButton_login"));
    m_submit->setMinimumHeight(46);
    m_submit->setDefault(true);
    form->addWidget(m_submit);
    form->addSpacing(28);
    form->addWidget(UiKit::divider(column));
    form->addSpacing(16);

    auto* registerRow = new QHBoxLayout;
    registerRow->setSpacing(8);
    registerRow->addWidget(UiKit::label(tr("Впервые у нас?"), "muted", column));
    auto* registerButton = UiKit::button(tr("Создать аккаунт"), "link", column);
    registerButton->setObjectName(QStringLiteral("pushButton_registration"));
    registerRow->addWidget(registerButton);
    registerRow->addStretch(1);
    form->addLayout(registerRow);

    formOuter->addWidget(column, 0, Qt::AlignLeft);
    formOuter->addStretch(1);
    root->addWidget(formPanel, 2);

    const auto refreshWordmark = [wordmark] {
        wordmark->setPixmap(ThemeManager::instance().icon(QStringLiteral("mercedez_benz")).pixmap(QSize(200, 24)));
    };
    refreshWordmark();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, wordmark, refreshWordmark);

    connect(m_submit, &QPushButton::clicked, this, &LoginPage::submit);
    connect(m_login, &QLineEdit::returnPressed, m_password, qOverload<>(&QWidget::setFocus));
    connect(m_password, &QLineEdit::returnPressed, this, &LoginPage::submit);
    connect(registerButton, &QPushButton::clicked, this, &LoginPage::registrationRequested);
    connect(m_login, &QLineEdit::textChanged, this, [this] { showError({}); });
    connect(m_password, &QLineEdit::textChanged, this, [this] { showError({}); });
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
    const QSize kBox = QSize(qMin(m_hero->width(), 900), qMin(m_hero->height(), 480)) * kDpr;
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
