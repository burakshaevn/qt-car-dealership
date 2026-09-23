#include "pages/TopBar.h"

#include "ThemeManager.h"
#include "UiKit.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace {
constexpr QSize kLogoSize(26, 26);
constexpr QSize kWordmarkSize(150, 18);
} // namespace

TopBar::TopBar(QWidget* parent)
    : QFrame(parent)
    , m_group(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("topbar"));
    m_group->setExclusive(true);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(40, 0, 40, 0);
    root->setSpacing(0);

    m_logo = new QLabel(this);
    root->addWidget(m_logo, 0, Qt::AlignVCenter);
    root->addSpacing(12);
    m_wordmark = new QLabel(this);
    root->addWidget(m_wordmark, 0, Qt::AlignVCenter);
    root->addSpacing(56);

    m_tabs = new QHBoxLayout;
    m_tabs->setSpacing(32);
    root->addLayout(m_tabs);
    root->addStretch(1);

    m_user = UiKit::label(QString(), nullptr, this);
    m_user->setObjectName(QStringLiteral("topbarUser"));
    root->addWidget(m_user, 0, Qt::AlignVCenter);
    root->addSpacing(24);

    auto* settings = UiKit::button(tr("Настройки"), "ghost", this);
    settings->setObjectName(QStringLiteral("pushButton_settings"));
    root->addWidget(settings, 0, Qt::AlignVCenter);
    root->addSpacing(20);
    auto* logout = UiKit::button(tr("Выйти"), "ghost", this);
    logout->setObjectName(QStringLiteral("pushButton_logout"));
    root->addWidget(logout, 0, Qt::AlignVCenter);

    refreshBrand();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &TopBar::refreshBrand);
    connect(settings, &QPushButton::clicked, this, &TopBar::settingsRequested);
    connect(logout, &QPushButton::clicked, this, &TopBar::logoutRequested);
}

void TopBar::refreshBrand()
{
    const ThemeManager& theme = ThemeManager::instance();
    m_logo->setPixmap(theme.icon(QStringLiteral("logo")).pixmap(kLogoSize));
    m_wordmark->setPixmap(theme.icon(QStringLiteral("mercedez_benz")).pixmap(kWordmarkSize));
}

void TopBar::addSection(const QString& id, const QString& title)
{
    auto* container = new QWidget(this);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto* button = UiKit::button(title, "tab", container);
    button->setObjectName(QStringLiteral("nav_") + id);
    button->setCheckable(true);
    layout->addWidget(button);

    auto* badge = new QLabel(container);
    badge->setObjectName(QStringLiteral("navCount"));
    badge->setAlignment(Qt::AlignCenter);
    badge->hide();
    layout->addWidget(badge, 0, Qt::AlignVCenter);

    m_group->addButton(button);
    m_buttons.insert(id, button);
    m_badges.insert(id, badge);
    m_tabs->addWidget(container);

    connect(button, &QPushButton::clicked, this, [this, id] { emit sectionSelected(id); });
}

void TopBar::setCurrentSection(const QString& id)
{
    if (QPushButton* button = m_buttons.value(id)) {
        button->setChecked(true);
    }
}

void TopBar::setBadge(const QString& id, const int count)
{
    if (QLabel* badge = m_badges.value(id)) {
        badge->setText(count > 99 ? QStringLiteral("99+") : QString::number(count));
        badge->setVisible(count > 0);
    }
}

void TopBar::setUser(const QString& name, const QString& subtitle)
{
    m_user->setText(name);
    m_user->setToolTip(subtitle);
}
