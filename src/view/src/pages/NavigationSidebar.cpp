#include "pages/NavigationSidebar.h"

#include "ThemeManager.h"
#include "UiKit.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
const QString kNavIconToken = QStringLiteral("textSecondary");
const QString kNavIconCheckedToken = QStringLiteral("accent");
} // namespace

NavigationSidebar::NavigationSidebar(QWidget* parent)
    : QFrame(parent)
    , m_group(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("sidebar"));
    setFixedWidth(272);
    m_group->setExclusive(true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 22, 16, 18);
    root->setSpacing(0);

    // Brand
    auto* brand = new QHBoxLayout;
    brand->setContentsMargins(8, 0, 0, 0);
    brand->setSpacing(10);
    auto* logo = new QLabel(this);
    logo->setPixmap(ThemeManager::instance().icon(QStringLiteral("logo")).pixmap(QSize(30, 30)));
    brand->addWidget(logo);
    auto* brandText = new QVBoxLayout;
    brandText->setSpacing(0);
    auto* title = new QLabel(tr("Mercedes-Benz"), this);
    title->setObjectName(QStringLiteral("sidebarBrand"));
    brandText->addWidget(title);
    brandText->addWidget(UiKit::label(tr("Автосалон"), "muted", this));
    brand->addLayout(brandText);
    brand->addStretch(1);
    root->addLayout(brand);
    root->addSpacing(26);

    m_sections = new QVBoxLayout;
    m_sections->setSpacing(2);
    root->addLayout(m_sections);
    root->addStretch(1);

    // User card
    root->addWidget(UiKit::divider(this));
    root->addSpacing(14);
    auto* user = new QHBoxLayout;
    user->setContentsMargins(4, 0, 0, 0);
    user->setSpacing(10);
    m_avatar = new QLabel(this);
    m_avatar->setObjectName(QStringLiteral("avatar"));
    m_avatar->setFixedSize(36, 36);
    m_avatar->setAlignment(Qt::AlignCenter);
    user->addWidget(m_avatar);

    auto* userText = new QVBoxLayout;
    userText->setSpacing(1);
    m_userName = new QLabel(this);
    m_userName->setObjectName(QStringLiteral("sidebarUserName"));
    m_userSubtitle = new QLabel(this);
    m_userSubtitle->setObjectName(QStringLiteral("sidebarUserEmail"));
    userText->addWidget(m_userName);
    userText->addWidget(m_userSubtitle);
    user->addLayout(userText, 1);
    root->addLayout(user);
    root->addSpacing(12);

    auto* actions = new QHBoxLayout;
    actions->setSpacing(6);
    auto* settings = UiKit::button(tr("Настройки"), "ghost", this);
    settings->setObjectName(QStringLiteral("pushButton_settings"));
    settings->setIconSize(QSize(18, 18));
    ThemeManager::instance().bindIcon(settings, QStringLiteral("settings"), kNavIconToken);
    auto* logout = UiKit::button(QString(), "icon", this);
    logout->setObjectName(QStringLiteral("pushButton_logout"));
    logout->setToolTip(tr("Выйти"));
    logout->setIconSize(QSize(18, 18));
    ThemeManager::instance().bindIcon(logout, QStringLiteral("logout"), QStringLiteral("danger"));
    actions->addWidget(settings, 1);
    actions->addWidget(logout);
    root->addLayout(actions);

    connect(settings, &QPushButton::clicked, this, &NavigationSidebar::settingsRequested);
    connect(logout, &QPushButton::clicked, this, &NavigationSidebar::logoutRequested);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [logo] {
        logo->setPixmap(ThemeManager::instance().icon(QStringLiteral("logo")).pixmap(QSize(30, 30)));
    });
}

void NavigationSidebar::addCaption(const QString& text)
{
    auto* caption = new QLabel(text.toUpper(), this);
    caption->setObjectName(QStringLiteral("sidebarCaption"));
    m_sections->addWidget(caption);
}

void NavigationSidebar::addSection(const QString& id, const QString& title, const QString& icon)
{
    auto* button = UiKit::button(title, "nav", this);
    button->setObjectName(QStringLiteral("nav_") + id);
    button->setCheckable(true);
    button->setIconSize(QSize(20, 20));
    ThemeManager::instance().bindIcon(button, icon, kNavIconToken, kNavIconCheckedToken);

    // Badge lives inside the button, aligned right.
    auto* badgeLayout = new QHBoxLayout(button);
    badgeLayout->setContentsMargins(0, 0, 10, 0);
    badgeLayout->addStretch(1);
    auto* badge = new QLabel(button);
    badge->setObjectName(QStringLiteral("notificationBadge"));
    badge->setMinimumSize(18, 18);
    badge->setAlignment(Qt::AlignCenter);
    badge->setAttribute(Qt::WA_TransparentForMouseEvents);
    badge->hide();
    badgeLayout->addWidget(badge);

    m_group->addButton(button);
    m_buttons.insert(id, button);
    m_badges.insert(id, badge);
    m_sections->addWidget(button);

    connect(button, &QPushButton::clicked, this, [this, id] { emit sectionSelected(id); });
}

void NavigationSidebar::setCurrentSection(const QString& id)
{
    if (QPushButton* button = m_buttons.value(id)) {
        button->setChecked(true);
    } else if (QAbstractButton* checked = m_group->checkedButton()) {
        m_group->setExclusive(false);
        checked->setChecked(false);
        m_group->setExclusive(true);
    }
}

void NavigationSidebar::setBadge(const QString& id, const int count)
{
    if (QLabel* badge = m_badges.value(id)) {
        badge->setText(count > 99 ? QStringLiteral("99+") : QString::number(count));
        badge->setVisible(count > 0);
    }
}

void NavigationSidebar::setUser(const QString& name, const QString& subtitle)
{
    m_avatar->setText(UiKit::initials(name));
    m_userName->setText(name);
    m_userSubtitle->setText(subtitle);
    m_userSubtitle->setVisible(!subtitle.isEmpty());
}
