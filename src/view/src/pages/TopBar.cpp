#include "pages/TopBar.h"

#include "ThemeManager.h"
#include "UiKit.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace {
/// Same horizontal inset as the pages, so the bar's text lines up with the content
/// and the bar reads as part of the page rather than as a separate strip.
constexpr int kContentInset = 56;
constexpr QSize kLogoSize(26, 26);
constexpr QSize kWordmarkSize(150, 18);
/// Every item of the bar is stretched to this height and centres its text in it,
/// so tabs, the user name and the account actions share one text line.
constexpr int kBarHeight = 56;

/// Makes a bar item fill the full bar height instead of keeping its natural size.
void fillHeight(QWidget* widget)
{
    widget->setSizePolicy(widget->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
}
} // namespace

TopBar::TopBar(QWidget* parent)
    : QFrame(parent)
    , m_group(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("topbar"));
    m_group->setExclusive(true);

    setFixedHeight(kBarHeight);
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(kContentInset, 0, kContentInset, 0);
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
    m_user->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    fillHeight(m_user);
    root->addWidget(m_user);
    root->addSpacing(28);

    auto* settings = UiKit::button(tr("Настройки"), "tab", this);
    settings->setObjectName(QStringLiteral("pushButton_settings"));
    fillHeight(settings);
    root->addWidget(settings);
    root->addSpacing(28);
    auto* logout = UiKit::button(tr("Выйти"), "tab", this);
    logout->setObjectName(QStringLiteral("pushButton_logout"));
    fillHeight(logout);
    root->addWidget(logout);

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
    fillHeight(container);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto* button = UiKit::button(title, "tab", container);
    button->setObjectName(QStringLiteral("nav_") + id);
    button->setCheckable(true);
    fillHeight(button);
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
