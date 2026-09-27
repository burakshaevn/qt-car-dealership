#include "MainWindow.h"

#include "AdminController.h"
#include "AppServices.h"
#include "AuthController.h"
#include "CatalogController.h"
#include "NotificationsController.h"
#include "RequestController.h"
#include "SettingsForm.h"
#include "ThemeManager.h"
#include "WindowChrome.h"
#include "pages/AdminPage.h"
#include "pages/CatalogPage.h"
#include "pages/LoginPage.h"
#include "pages/NavigationSidebar.h"
#include "pages/NotificationsPage.h"
#include "pages/ProductPage.h"
#include "pages/ProfilePage.h"
#include "pages/TopBar.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStackedWidget>

namespace Section {
const QString kCatalog = QStringLiteral("catalog");
const QString kProfile = QStringLiteral("profile");
const QString kNotifications = QStringLiteral("notifications");
} // namespace Section

MainWindow::MainWindow(AppServices& services, QWidget* parent)
    : QMainWindow(parent)
    , m_services(services)
    , m_auth(new AuthController(services))
    , m_requests(new RequestController(services))
{
    // Like native macOS apps, the window carries no application name: the title
    // follows the current section and the native title bar blends into the page.
    setWindowTitle(tr("Вход"));
    WindowChrome::attach(this);
    setWindowIcon(ThemeManager::instance().icon(QStringLiteral("logo")));
    setMinimumSize(1100, 720);
    resize(1320, 840);

    buildUi();

    connect(m_requests.get(), &RequestController::requestSubmitted, this, &MainWindow::refreshBadges);
    connect(m_requests.get(), &RequestController::catalogRequested, this, [this] { navigate(Section::kCatalog); });

    // Hero image for the sign-in screen: first car in stock.
    m_services.products().pullProducts();
    const QList<ProductInfo> kProducts = m_services.products().products();
    const auto kHero = std::find_if(kProducts.cbegin(), kProducts.cend(),
                                    [](const ProductInfo& p) { return p.StockQty > 0; });
    if (kHero != kProducts.cend()) {
        m_loginPage->setHeroImage(kHero->ImagePath);
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi()
{
    m_root = new QStackedWidget(this);
    m_root->setObjectName(QStringLiteral("stackedWidget"));
    setCentralWidget(m_root);

    m_loginPage = new LoginPage(m_root);
    m_root->addWidget(m_loginPage);
    connect(m_loginPage, &LoginPage::loginRequested, this, &MainWindow::onLogin);
    connect(m_loginPage, &LoginPage::registrationRequested, this, &MainWindow::onRegistration);
}

void MainWindow::onLogin(const QString& login, const QString& password)
{
    const QString kError = m_auth->login(login, password);
    if (!kError.isEmpty()) {
        m_loginPage->showError(kError);
        return;
    }
    m_loginPage->clear();
    if (m_services.session().isAdmin()) {
        startAdminSession();
    } else {
        startCustomerSession();
    }
}

void MainWindow::onRegistration()
{
    if (const auto kEmail = m_auth->runRegistrationDialog(this)) {
        QMessageBox::information(this, tr("Регистрация"),
                                 tr("Аккаунт создан. Войдите, используя email %1.").arg(*kEmail));
    }
}

void MainWindow::onLogout()
{
    m_catalog.reset();
    m_notifications.reset();
    m_admin.reset();
    m_purchasedModel.clear();
    m_services.session().clear();

    m_root->setCurrentWidget(m_loginPage);
    setWindowTitle(tr("Вход"));
    if (m_workspace) {
        m_root->removeWidget(m_workspace);
        m_workspace->deleteLater();
    }
    m_workspace = nullptr;
    m_sidebar = nullptr;
    m_topBar = nullptr;
    m_pages = nullptr;
    m_catalogPage = nullptr;
    m_productPage = nullptr;
    m_profilePage = nullptr;
    m_notificationsPage = nullptr;
    m_adminPage = nullptr;
}

void MainWindow::startCustomerSession()
{
    const UserSession& session = m_services.session();

    m_workspace = new QWidget(m_root);
    auto* layout = new QVBoxLayout(m_workspace);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_topBar = new TopBar(m_workspace);
    m_topBar->addSection(Section::kCatalog, tr("Модельный ряд"));
    m_topBar->addSection(Section::kProfile, tr("Личный кабинет"));
    m_topBar->addSection(Section::kNotifications, tr("Уведомления"));
    m_topBar->setUser(session.name(), session.email());
    layout->addWidget(m_topBar);

    m_pages = new QStackedWidget(m_workspace);
    m_catalogPage = new CatalogPage(m_pages);
    m_productPage = new ProductPage(m_pages);
    m_profilePage = new ProfilePage(m_pages);
    m_notificationsPage = new NotificationsPage(m_pages);
    m_pages->addWidget(m_catalogPage);
    m_pages->addWidget(m_productPage);
    m_pages->addWidget(m_profilePage);
    m_pages->addWidget(m_notificationsPage);
    layout->addWidget(m_pages, 1);

    m_catalog.reset(new CatalogController(m_services, m_catalogPage));
    m_notifications.reset(new NotificationsController(m_services, m_notificationsPage));
    m_profilePage->setPurchasedModel(&m_purchasedModel);

    connect(m_topBar, &TopBar::sectionSelected, this, &MainWindow::navigate);
    connect(m_topBar, &TopBar::settingsRequested, this, &MainWindow::openSettings);
    connect(m_topBar, &TopBar::logoutRequested, this, &MainWindow::onLogout);
    connect(m_catalog.get(), &CatalogController::productSelected, this, &MainWindow::showProduct);
    connect(m_notifications.get(), &NotificationsController::unreadCountChanged, this,
            [this](const int count) { m_topBar->setBadge(Section::kNotifications, count); });

    connect(m_productPage, &ProductPage::backRequested, this, [this] { navigate(Section::kCatalog); });
    connect(m_productPage, &ProductPage::checkoutRequested, this,
            [this](const ProductInfo& p) { m_requests->checkout(this, p); });
    connect(m_productPage, &ProductPage::orderRequested, this,
            [this](const ProductInfo& p) { m_requests->order(this, p); });
    connect(m_productPage, &ProductPage::testDriveRequested, this,
            [this](const ProductInfo& p) { m_requests->request(this, PurchaseMethod::TestDrive, p); });

    connect(m_profilePage, &ProfilePage::serviceRequested, this,
            [this](const PurchaseMethod method) { m_requests->request(this, method); });
    connect(m_profilePage, &ProfilePage::editProfileRequested, this, &MainWindow::openSettings);
    connect(m_profilePage, &ProfilePage::productActivated, this, [this](const QModelIndex& index) {
        showProduct(m_purchasedModel.productAt(index.row()));
    });

    m_root->addWidget(m_workspace);
    m_root->setCurrentWidget(m_workspace);

    m_catalog->reload();
    refreshBadges();
    navigate(Section::kCatalog);
}

void MainWindow::startAdminSession()
{
    m_workspace = new QWidget(m_root);
    auto* layout = new QHBoxLayout(m_workspace);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_sidebar = new NavigationSidebar(m_workspace);
    m_sidebar->setUser(m_services.session().name(), tr("Администратор"));
    layout->addWidget(m_sidebar);

    m_adminPage = new AdminPage(m_workspace);
    layout->addWidget(m_adminPage, 1);
    m_admin.reset(new AdminController(m_services, m_adminPage));

    // Sections come from sys_admin_tables: requests first, then reference data.
    const QList<AdminTableInfo> kTables = m_admin->tables();
    for (const bool kRequests : {true, false}) {
        m_sidebar->addCaption(kRequests ? tr("Заявки") : tr("Справочники"));
        for (const AdminTableInfo& info : kTables) {
            if (info.IsRequest == kRequests) {
                m_sidebar->addSection(info.TableName, info.DisplayName, QString());
            }
        }
    }

    connect(m_sidebar, &NavigationSidebar::sectionSelected, m_admin.get(), &AdminController::open);
    connect(m_sidebar, &NavigationSidebar::sectionSelected, this, [this, kTables](const QString& table) {
        for (const AdminTableInfo& info : kTables) {
            if (info.TableName == table) {
                setWindowTitle(info.DisplayName);
                break;
            }
        }
    });
    connect(m_sidebar, &NavigationSidebar::settingsRequested, this, &MainWindow::openSettings);
    connect(m_sidebar, &NavigationSidebar::logoutRequested, this, &MainWindow::onLogout);

    m_root->addWidget(m_workspace);
    m_root->setCurrentWidget(m_workspace);

    if (!kTables.isEmpty()) {
        m_sidebar->setCurrentSection(kTables.first().TableName);
        m_admin->open(kTables.first().TableName);
        setWindowTitle(kTables.first().DisplayName);
    }
}

void MainWindow::navigate(const QString& section)
{
    if (!m_pages) {
        return;
    }
    m_topBar->setCurrentSection(section);
    if (section == Section::kCatalog) {
        m_pages->setCurrentWidget(m_catalogPage);
        setWindowTitle(tr("Модельный ряд"));
    } else if (section == Section::kProfile) {
        refreshProfile();
        m_pages->setCurrentWidget(m_profilePage);
        setWindowTitle(tr("Личный кабинет"));
    } else if (section == Section::kNotifications) {
        m_notifications->refresh();
        m_pages->setCurrentWidget(m_notificationsPage);
        setWindowTitle(tr("Уведомления"));
    }
}

void MainWindow::showProduct(const ProductInfo& product)
{
    const QList<ProductInfo> kVariants = m_services.products().variantsOf(product.Name);
    int current = 0;
    for (int i = 0; i < kVariants.size(); ++i) {
        if (kVariants.at(i).Color == product.Color) {
            current = i;
            break;
        }
    }
    m_productPage->setVariants(kVariants.isEmpty() ? QList<ProductInfo>{product} : kVariants, current);
    m_topBar->setCurrentSection(Section::kCatalog);
    m_pages->setCurrentWidget(m_productPage);
    setWindowTitle(product.Name);
}

void MainWindow::refreshProfile()
{
    const UserSession& session = m_services.session();
    m_profilePage->setUser(session.name(), session.email());

    QList<ProductInfo> purchased;
    const auto kKeys = m_services.products().purchasedBy(session.id());
    for (const auto& key : kKeys) {
        if (const ProductInfo* info = m_services.products().findProduct(key)) {
            purchased.append(*info);
        }
    }
    m_purchasedModel.setProducts(purchased);
    m_profilePage->setPurchasedCount(static_cast<int>(purchased.size()));
}

void MainWindow::refreshBadges()
{
    if (m_topBar && m_notifications) {
        m_topBar->setBadge(Section::kNotifications, m_notifications->unreadCount());
    }
}

void MainWindow::openSettings()
{
    SettingsForm form(m_services, this);
    connect(&form, &SettingsForm::profileSaved, this, [this](const QString& fullName, const QString& email) {
        if (m_topBar) {
            m_topBar->setUser(fullName, email);
        }
        if (m_sidebar) {
            m_sidebar->setUser(fullName, email);
        }
        if (m_profilePage) {
            m_profilePage->setUser(fullName, email);
        }
    });
    form.exec();
}
