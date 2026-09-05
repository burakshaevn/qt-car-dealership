#include "MainWindow.h"
#include "PriceFormatter.h"
#include "PurchaseRequestStrategy.h"
#include "SelectedCarActionStrategy.h"
#include "ThemeStyleProvider.h"
#include "ui_MainWindow.h"

#include <QSpinBox>
#include <QCalendarWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QScrollArea>
#include <QComboBox>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QApplication>
#include <QIcon>
#include <QStyle>

namespace {

void prepareInputDialog(QInputDialog& dialog)
{
    if (auto* buttons = dialog.findChild<QDialogButtonBox*>()) {
        if (auto* okButton = buttons->button(QDialogButtonBox::Ok)) {
            okButton->setProperty("type", "primary");
            okButton->style()->unpolish(okButton);
            okButton->style()->polish(okButton);
        }
        if (auto* cancelButton = buttons->button(QDialogButtonBox::Cancel)) {
            cancelButton->setProperty("type", "secondary");
            cancelButton->style()->unpolish(cancelButton);
            cancelButton->style()->polish(cancelButton);
        }
    }
    applyThemeStyle(&dialog, "DialogForm");
}

QString getTextFromThemedDialog(QWidget* parent,
                                const QString& title,
                                const QString& label,
                                bool* accepted)
{
    QInputDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setInputMode(QInputDialog::TextInput);
    prepareInputDialog(dialog);

    const bool kIsAccepted = dialog.exec() == QDialog::Accepted;
    if (accepted) {
        *accepted = kIsAccepted;
    }
    return kIsAccepted ? dialog.textValue() : QString();
}

QString getItemFromThemedDialog(QWidget* parent,
                                const QString& title,
                                const QString& label,
                                const QStringList& items,
                                bool* accepted)
{
    QInputDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setComboBoxItems(items);
    dialog.setComboBoxEditable(false);
    prepareInputDialog(dialog);

    const bool kIsAccepted = dialog.exec() == QDialog::Accepted;
    if (accepted) {
        *accepted = kIsAccepted;
    }
    return kIsAccepted ? dialog.textValue() : QString();
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_services(new AppServices)
{
    m_ui->setupUi(this);
    applyThemeStyle(this, "MainShell");
    applyThemeIcons();

    connect(m_ui->pushButton_login, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_ui->pushButton_registration,
            &QPushButton::clicked,
            this,
            &MainWindow::onRegistrationClicked);
    connect(m_ui->pushButton_logout, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(m_ui->pushButton_next_left, &QPushButton::clicked, this, &MainWindow::onNextLeftClicked);
    connect(m_ui->pushButton_next_right,
            &QPushButton::clicked,
            this,
            &MainWindow::onNextRightClicked);
    connect(m_ui->pushButton_back, &QPushButton::clicked, this, &MainWindow::onBackClicked);
    connect(m_ui->pushButton_to_pay, &QPushButton::clicked, this, &MainWindow::onToPayClicked);
    connect(m_ui->pushButton_info, &QPushButton::clicked, this, &MainWindow::onInfoClicked);
    connect(m_ui->pushButton_test_drive,
            &QPushButton::clicked,
            this,
            &MainWindow::onTestDriveClicked);
    connect(m_ui->pushButton_order, &QPushButton::clicked, this, &MainWindow::onOrderClicked);
    connect(m_ui->pushButton_notifications,
            &QPushButton::clicked,
            this,
            &MainWindow::onNotificationsClicked);
    connect(m_ui->pushButton_settings, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

    m_services->ensureCore();

    // By default, show login page
    m_ui->stackedWidget->setCurrentWidget(m_ui->login);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::applyThemeIcons()
{
    setWindowIcon(loadThemeIcon("logo.svg"));
    if (m_ui->label_2) {
        m_ui->label_2->setScaledContents(false);
        m_ui->label_2->setPixmap(loadThemeIcon("mercedez_benz.svg").pixmap(273, 31));
    }
    if (m_ui->pushButton_settings) {
        applyThemeIcon(m_ui->pushButton_settings, "settings.svg");
    }
    if (m_ui->pushButton_notifications) {
        applyThemeIcon(m_ui->pushButton_notifications, "inbox.svg");
    }
    if (m_ui->pushButton_logout) {
        applyThemeIcon(m_ui->pushButton_logout, "navigate_next.svg");
    }
    if (m_ui->pushButton_next_left) {
        applyThemeIcon(m_ui->pushButton_next_left, "navigate_before.svg");
    }
    if (m_ui->pushButton_next_right) {
        applyThemeIcon(m_ui->pushButton_next_right, "navigate_next.svg");
    }
}

void MainWindow::setDarkThemeEnabled(bool enabled)
{
    const ThemeMode kMode = enabled ? ThemeMode::Dark : ThemeMode::Light;
    qApp->setProperty("app_theme", enabled ? "dark" : "light");
    const auto kTopLevels = qApp->topLevelWidgets();
    for (QWidget* widget : kTopLevels) {
        reapplyThemeStyles(widget, kMode);
        reapplyThemeIcons(widget, kMode);
    }
    applyThemeIcons();
    if (m_ui->catalogListView) {
        m_ui->catalogListView->viewport()->update();
    }
    if (m_ui->purchasedListView) {
        m_ui->purchasedListView->viewport()->update();
    }
    if (m_ui->purchaseMethodListView) {
        m_ui->purchaseMethodListView->viewport()->update();
    }
}

void MainWindow::updateUser(const UserInfo& user, QWidget* parent)
{
    Q_UNUSED(parent);
    m_services->getUserSession()->setCurrentUser(user);
}

void MainWindow::onLoginClicked()
{
    m_services->ensureControllers(this);

    AuthController::AuthResult auth = m_services->getAuth()->login(m_ui->lineEdit_login->text(),
                                                                   m_ui->lineEdit_password->text());

    if (!auth.Ok) {
        QMessageBox::critical(this, "Авторизация", auth.Error);
        return;
    }

    UserInfo user = auth.User;

    if (user.Role == Role::Admin) {
        m_ui->lineEdit_login->clear();
        m_ui->lineEdit_password->clear();
        QMessageBox::information(this, "Авторизация", "Выполнена авторизация как администратор.");
        updateUser(user, this);

        if (m_services->getAdminTable()) {
            m_services->getAdminTable()->disconnect(this);
            connect(m_services->getAdminTable(),
                    &AdminTableController::logoutRequested,
                    this,
                    &MainWindow::onLogoutClicked);
            m_services->getAdminTable()->show(m_ui->stackedWidget, this);
        }
        return;
    }

    if (user.Role == Role::User) {
        buildDependencies();
        updateUser(user, this);

        m_services->getProducts()->pullProducts();

        if (m_services->getCatalog()) {
            m_services->getCatalog()->resetDefault();
        }

        m_ui->lineEdit_login->clear();
        m_ui->lineEdit_password->clear();
        QMessageBox::information(this, "Авторизация", "Выполнена авторизация как пользователь.");

        if (m_services->getFloatingWidget()) {
            m_services->getFloatingWidget()->setVisible(true);
        }

        m_ui->stackedWidget->setCurrentWidget(m_ui->main);
    }
}

void MainWindow::onLogoutClicked()
{
    if (!this->m_ui->stackedWidget)
        return;

    if (m_services->getUserSession()->isUser()) {
        if (m_ui->catalogListView) {
            m_ui->catalogListView->setModel(nullptr);
            m_ui->catalogListView->setItemDelegate(nullptr);
        }

        if (m_ui->purchasedListView) {
            m_ui->purchasedListView->setModel(nullptr);
            m_ui->purchasedListView->setItemDelegate(nullptr);
        }
    }
    m_services->resetSession();

    if (this->m_ui->stackedWidget && this->m_ui->login) {
        this->m_ui->stackedWidget->setCurrentWidget(this->m_ui->login);
    }
}

void MainWindow::buildDependencies()
{
    m_services->ensureControllers(this);
    if (!m_services->getFloatingWidget()) {
        setupFloatingMenu();
    }

    m_services->getCatalog()->disconnect(this);
    connect(m_services->getCatalog(),
            &CatalogController::productSelected,
            this,
            [this](const ProductInfo& product) {
                if (!m_services->getProducts()) {
                    return;
                }
                auto productColors = m_services->getProducts()->getAllProductsWithName(product);
                showProductOnPersonalPage(product, productColors);
            });

    m_services->getCatalog()->initialize(m_ui->catalogListView);
    m_services->getProfile()->initialize(m_ui->purchasedListView,
                                         m_ui->label_clientname,
                                         m_ui->groupBox_6);
}

void MainWindow::setupServicesScrollArea()
{
    if (!m_purchaseMethodsController) {
        m_purchaseMethodsController.reset(new PurchaseMethodsController(this));
        connect(m_purchaseMethodsController.get(),
                &PurchaseMethodsController::openCatalogRequested,
                this,
                [this]() { m_ui->stackedWidget->setCurrentWidget(m_ui->main); });
    }
    m_purchaseMethodsController->initialize(m_ui->purchaseMethodListView, m_services.get(), this);
}

void MainWindow::selectionProcessing(const bool kOk,
                                     const QStringView kSelectedType,
                                     const QStringView kSelectedColor)
{
    if (!kOk) {
        return;
    }

    if (!m_services->getCatalog()) {
        return;
    }

    m_services->getCatalog()->applyFilter(kSelectedType, kSelectedColor);
    m_ui->stackedWidget->setCurrentWidget(m_ui->main);
}
void MainWindow::showProductOnPersonalPage(const ProductInfo& product,
                                           QList<ProductInfo>& mCurrentProductcolors)
{
    m_ui->label_name->setText(product.Name);
    m_ui->label_price->setText(formatPrice(product.Price) + " руб.");
    m_currentProduct = product;
    // m_ui->label_Colorindex->setText(QString::number(m_currentColorIndex + 1) + "/" + QString::number(m_currentProductcolors_.size()));

    QString imagePath = QDir::cleanPath(product.ImagePath);
    QPixmap originalPixmap(imagePath);

    if (!originalPixmap.isNull()) {
        const int kAvailableWidth = m_ui->groupBox_2 ? qMax(m_ui->groupBox_2->width(),
                                                            m_ui->groupBox_2->minimumWidth())
                                                     : 900;
        const int kImageWidth = qBound(520, kAvailableWidth - 180, 700);
        const QPixmap kScaledPixmap = originalPixmap.scaled(
            QSize(kImageWidth, 285),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        m_ui->label_car_image->setPixmap(kScaledPixmap);
        m_ui->label_car_image->setMinimumSize(kImageWidth, 250);
        m_ui->label_car_image->setMaximumSize(kImageWidth, 285);
        m_ui->label_car_image->setAlignment(Qt::AlignCenter);
        m_ui->label_car_image->show();
    }

    m_ui->pushButton_order->setVisible(product.StockQty
                                       <= 0); // If product is not in stock, show order button
    m_ui->pushButton_to_pay->setVisible(product.StockQty
                                        > 0); // If product is in stock, show to pay button

    m_ui->stackedWidget->setCurrentWidget(m_ui->personal);
    updatePersonalPageLayout();
}

void MainWindow::onNextLeftClicked()
{
    // If there are other products with the same name, show the previous product
    auto mCurrentProductcolors = m_services->getProducts()->getAllProductsWithName(m_currentProduct);
    if (!mCurrentProductcolors.isEmpty())
    {
        m_currentColorIndex = (m_currentColorIndex - 1 + mCurrentProductcolors.size())
                              % mCurrentProductcolors.size();
        m_currentProduct = mCurrentProductcolors.at(m_currentColorIndex);
        showProductOnPersonalPage(mCurrentProductcolors.at(m_currentColorIndex),
                                  mCurrentProductcolors);
    }
}

void MainWindow::onNextRightClicked()
{
    // If there are other products with the same name, show the next product
    auto mCurrentProductcolors = m_services->getProducts()->getAllProductsWithName(m_currentProduct);
    if (!mCurrentProductcolors.isEmpty())
    {
        m_currentColorIndex = (m_currentColorIndex + 1) % mCurrentProductcolors.size();
        m_currentProduct = mCurrentProductcolors.at(m_currentColorIndex);
        showProductOnPersonalPage(mCurrentProductcolors.at(m_currentColorIndex),
                                  mCurrentProductcolors);
    }
}

void MainWindow::onBackClicked()
{
    if (!m_services->getUserSession()->isUser())
        return;

    // Reset current product state
    m_currentProduct = ProductInfo();
    m_currentColorIndex = 0;

    // Rebm_uild the main catalog view
    if (m_services->getCatalog()) {
        m_services->getCatalog()->resetDefault();
    }

    m_ui->stackedWidget->setCurrentWidget(m_ui->main);
}

void MainWindow::onInfoClicked()
{
    if (m_currentProduct.Name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для просмотра информации.");
        return;
    }

    // Show product information
    QString info = QString("Название: %1\nЦена: %2 руб.\nЦвет: %3\nОписание: %4")
                       .arg(m_currentProduct.Name)
                       .arg(formatPrice(m_currentProduct.Price))
                       .arg(m_currentProduct.Color)
                       .arg(m_currentProduct.Description.isEmpty() ? "Описание отсутствует."
                                                                   : m_currentProduct.Description);

    QMessageBox::information(this, "Информация об автомобиле", info);
}

void MainWindow::onTestDriveClicked()
{
    auto strategy = createPurchaseRequestStrategy(PurchaseMethod::TestDrive);
    if (!strategy) {
        QMessageBox::warning(this, "Error", "Strategy for test drive is not available.");
        return;
    }
    strategy->execute(this, m_services.get());
}

void MainWindow::onToPayClicked()
{
    auto strategy = createSelectedCarActionStrategy(SelectedCarAction::Checkout);
    if (!strategy) {
        QMessageBox::warning(this, "Error", "Checkout strategy is not available.");
        return;
    }
    strategy->execute(this, m_services.get(), m_currentProduct);
}

void MainWindow::onOrderClicked()
{
    auto strategy = createSelectedCarActionStrategy(SelectedCarAction::Order);
    if (!strategy) {
        QMessageBox::warning(this, "Error", "Order strategy is not available.");
        return;
    }
    strategy->execute(this, m_services.get(), m_currentProduct);
}

void MainWindow::onNotificationsClicked()
{
    if (!m_services->getUserSession()->isAuthorized() || !m_services->getNotifications()) {
        return;
    }
    m_services->getNotifications()->showForUser(m_services->getUserSession()->getId(), this);
}

void MainWindow::onSettingsClicked()
{
    if (!m_services->getUserSession()->isAuthorized()) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к настройкам.");
        return;
    }

    if (!m_settingsForm) {
        m_settingsForm.reset(new SettingsForm(m_services.get(), this));

        connect(m_settingsForm.get(),
                &SettingsForm::profileSaved,
                this,
                [this](const QString& fullName, const QString&) {
                    if (m_ui->label_clientname) {
                        m_ui->label_clientname->setText(fullName + " — профиль");
                    }
                });
        connect(m_settingsForm.get(), &SettingsForm::themeChanged, this, [this](bool darkEnabled) {
            setDarkThemeEnabled(darkEnabled);
        });

        connect(m_settingsForm.get(), &QDialog::finished, this, [this]() {
            m_settingsForm.reset();
        });
    }

    m_settingsForm->show();
    m_settingsForm->raise();
    m_settingsForm->activateWindow();
}

void MainWindow::onRegistrationClicked()
{
    m_services->ensureControllers(this);

    bool registered = m_services->getAuth()->runRegistrationDialog(this);
    if (registered) {
        m_ui->lineEdit_login->clear();
        m_ui->lineEdit_password->clear();
    }
}
void MainWindow::onProfileClicked()
{
    if (!m_services->getUserSession()->isUser()) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к профилю.");
        return;
    }

    if (m_ui->stackedWidget->currentWidget() == m_ui->user_page)
        return;

    if (m_services->getProfile()) {
        m_services->getProfile()->showProfile(m_services->getUserSession()->getId(),
                                              m_services->getUserSession()->getName());
    }

    setupServicesScrollArea();

    m_ui->stackedWidget->setCurrentWidget(m_ui->user_page);
}

void MainWindow::onSortByColorClicked()
{
    if (m_services->getUserSession()->isAuthorized()) {
        if (m_services->getUserSession()->isUser()) {
            bool ok;
            const QString kSelectedColor
                = getItemFromThemedDialog(this,
                                          "Поиск по цветам",
                                          "Выберите цвет:",
                                          m_services->getProducts()->getAvailableColors(),
                                          &ok);

            selectionProcessing(ok, QStringView(), kSelectedColor);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::onSearchClicked()
{
    bool ok;
    const QString kTerm = getTextFromThemedDialog(
        this,
        "Поиск",
        "Укажите поисковый запрос:",
        &ok);

    if (ok && !kTerm.isEmpty())
    {
        if (!m_services->getProducts() || !m_services->getCatalog()) {
            QMessageBox::warning(this, "Поиск", "Каталог недоступен.");
            return;
        }

        int relevantCount = m_services->getCatalog()->search(kTerm);
        if (relevantCount > 0)
        {
            m_ui->stackedWidget->setCurrentWidget(m_ui->main);
            QMessageBox::information(this, "Поиск", "Найдено " + QString::number(relevantCount) + " результатов по запросу «" + kTerm + "».");
        }
        else
        {
            QMessageBox::warning(this, "Поиск", "Отсутствуют релевантные результаты.");
        }
    }
}

void MainWindow::onSortByTypeClicked()
{
    if (m_services->getUserSession()->isAuthorized()) {
        if (m_services->getUserSession()->isUser()) {
            QStringList types;  // Список типов авто для выпадающего списка

            if (m_services->getDatabase()) {
                types = m_services->getDatabase()->getCarTypeNames();
            }
            types << QString::fromUtf8("Все");

            bool ok;
            const QString kSelectedType = getItemFromThemedDialog(
                this,
                "Поиск по типу авто",
                "Тип:",
                types,
                &ok);

            QString defaultColor;
            if (m_services->getDatabase()) {
                defaultColor = m_services->getDatabase()->getDefaultCatalogColor();
            }

            if (kSelectedType == QString::fromUtf8("Все")) {
                selectionProcessing(ok, QStringView(), defaultColor);
            } else {
                selectionProcessing(ok, kSelectedType, defaultColor);
            }
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::setupFloatingMenu()
{
    m_services->ensureFloatingWidget(this);

    m_services->getFloatingWidget()->buildFloatingMenu(
        58,                                         // Floating menu x position
        this->height(),                             // Floating menu height
        [this]() { this->onSortByTypeClicked(); },  // On sort by type clicked
        [this]() { this->onSearchClicked(); },      // On search clicked
        [this]() { this->onSortByColorClicked(); }, // On sort by color clicked
        [this]() { this->onProfileClicked(); }      // On profile clicked
    );

    // Show floating menu
    if (m_services->getFloatingWidget()) {
        m_services->getFloatingWidget()->setVisible(false);
    }
}

void MainWindow::updateFloatingMenuPosition()
{
    if (m_services->getFloatingWidget()) {
        // Update floating menu position
        int x = 58;  // Floating menu x position
        int y = (this->height() - m_services->getFloatingWidget()->height())
                / 2; // Floating menu y position
        m_services->getFloatingWidget()->move(x, y);
    }
}

void MainWindow::updatePersonalPageLayout()
{
    if (!m_ui || !m_ui->groupBox_2 || !m_ui->pushButton_next_left || !m_ui->pushButton_next_right) {
        return;
    }

    constexpr int kArrowSize = 68;
    constexpr int kSideMargin = 16;
    const int kY = qMax(0, (m_ui->groupBox_2->height() - kArrowSize) / 2);

    m_ui->pushButton_next_left->setGeometry(kSideMargin, kY, kArrowSize, kArrowSize);
    m_ui->pushButton_next_right->setGeometry(qMax(kSideMargin,
                                                  m_ui->groupBox_2->width() - kArrowSize
                                                      - kSideMargin),
                                             kY,
                                             kArrowSize,
                                             kArrowSize);
    m_ui->pushButton_next_left->raise();
    m_ui->pushButton_next_right->raise();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);  // Resize event

    // Update floating menu position
    updateFloatingMenuPosition();
    updatePersonalPageLayout();
}
