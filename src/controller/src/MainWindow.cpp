#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PurchaseRequestStrategy.h"
#include "SelectedCarActionStrategy.h"
#include "PriceFormatter.h"
#include "ThemeStyleProvider.h"

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

void PrepareInputDialog(QInputDialog& dialog)
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
    ApplyThemeStyle(&dialog, "DialogForm");
}

QString GetTextFromThemedDialog(QWidget* parent,
                                const QString& title,
                                const QString& label,
                                bool* accepted)
{
    QInputDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setInputMode(QInputDialog::TextInput);
    PrepareInputDialog(dialog);

    const bool isAccepted = dialog.exec() == QDialog::Accepted;
    if (accepted) {
        *accepted = isAccepted;
    }
    return isAccepted ? dialog.textValue() : QString();
}

QString GetItemFromThemedDialog(QWidget* parent,
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
    PrepareInputDialog(dialog);

    const bool isAccepted = dialog.exec() == QDialog::Accepted;
    if (accepted) {
        *accepted = isAccepted;
    }
    return isAccepted ? dialog.textValue() : QString();
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_services(new AppServices) {

    ui->setupUi(this);
    ApplyThemeStyle(this, "MainShell");
    ApplyThemeIcons();

    connect(ui->pushButton_login, &QPushButton::clicked, this, &MainWindow::OnLoginClicked);
    connect(ui->pushButton_registration, &QPushButton::clicked, this, &MainWindow::OnRegistrationClicked);
    connect(ui->pushButton_logout, &QPushButton::clicked, this, &MainWindow::OnLogoutClicked);
    connect(ui->pushButton_next_left, &QPushButton::clicked, this, &MainWindow::OnNextLeftClicked);
    connect(ui->pushButton_next_right, &QPushButton::clicked, this, &MainWindow::OnNextRightClicked);
    connect(ui->pushButton_back, &QPushButton::clicked, this, &MainWindow::OnBackClicked);
    connect(ui->pushButton_to_pay, &QPushButton::clicked, this, &MainWindow::OnToPayClicked);
    connect(ui->pushButton_info, &QPushButton::clicked, this, &MainWindow::OnInfoClicked);
    connect(ui->pushButton_test_drive, &QPushButton::clicked, this, &MainWindow::OnTestDriveClicked);
    connect(ui->pushButton_order, &QPushButton::clicked, this, &MainWindow::OnOrderClicked);
    connect(ui->pushButton_notifications, &QPushButton::clicked, this, &MainWindow::OnNotificationsClicked);
    connect(ui->pushButton_settings, &QPushButton::clicked, this, &MainWindow::OnSettingsClicked);

    m_services->EnsureCore();

    // By default, show login page
    ui->stackedWidget->setCurrentWidget(ui->login);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ApplyThemeIcons()
{
    setWindowIcon(LoadThemeIcon("logo.svg"));
    if (ui->label_2) {
        ui->label_2->setScaledContents(false);
        ui->label_2->setPixmap(LoadThemeIcon("mercedez_benz.svg").pixmap(273, 31));
    }
    if (ui->pushButton_settings) {
        ApplyThemeIcon(ui->pushButton_settings, "settings.svg");
    }
    if (ui->pushButton_notifications) {
        ApplyThemeIcon(ui->pushButton_notifications, "inbox.svg");
    }
    if (ui->pushButton_logout) {
        ApplyThemeIcon(ui->pushButton_logout, "navigate_next.svg");
    }
    if (ui->pushButton_next_left) {
        ApplyThemeIcon(ui->pushButton_next_left, "navigate_before.svg");
    }
    if (ui->pushButton_next_right) {
        ApplyThemeIcon(ui->pushButton_next_right, "navigate_next.svg");
    }
}

void MainWindow::SetDarkThemeEnabled(bool enabled)
{
    const ThemeMode mode = enabled ? ThemeMode::Dark : ThemeMode::Light;
    qApp->setProperty("app_theme", enabled ? "dark" : "light");
    const auto topLevels = qApp->topLevelWidgets();
    for (QWidget* widget : topLevels) {
        ReapplyThemeStyles(widget, mode);
        ReapplyThemeIcons(widget, mode);
    }
    ApplyThemeIcons();
    if (ui->catalogListView) {
        ui->catalogListView->viewport()->update();
    }
    if (ui->purchasedListView) {
        ui->purchasedListView->viewport()->update();
    }
    if (ui->purchaseMethodListView) {
        ui->purchaseMethodListView->viewport()->update();
    }
}

void MainWindow::UpdateUser(const UserInfo& user, QWidget* parent)
{
    Q_UNUSED(parent);
    m_services->GetUserSession()->SetCurrentUser(user);
}

void MainWindow::OnLoginClicked()
{
    m_services->EnsureControllers(this);

    AuthController::AuthResult auth = m_services->GetAuth()->Login(
        ui->lineEdit_login->text(),
        ui->lineEdit_password->text()
    );

    if (!auth.ok) {
        QMessageBox::critical(this, "Авторизация", auth.error);
        return;
    }

    UserInfo user = auth.user;

    if (user.role_ == Role::Admin) {
        ui->lineEdit_login->clear();
        ui->lineEdit_password->clear();
        QMessageBox::information(this, "Авторизация", "Выполнена авторизация как администратор.");
        UpdateUser(user, this);

        if (m_services->GetAdminTable()) {
            m_services->GetAdminTable()->disconnect(this);
            connect(m_services->GetAdminTable(), &AdminTableController::LogoutRequested, this, &MainWindow::OnLogoutClicked);
            m_services->GetAdminTable()->Show(ui->stackedWidget, this);
        }
        return;
    }

    if (user.role_ == Role::User) {
        BuildDependencies();
        UpdateUser(user, this);

        m_services->GetProducts()->PullProducts();

        if (m_services->GetCatalog()) {
            m_services->GetCatalog()->ResetDefault();
        }

        ui->lineEdit_login->clear();
        ui->lineEdit_password->clear();
        QMessageBox::information(this, "Авторизация", "Выполнена авторизация как пользователь.");

        if (m_services->GetFloatingWidget()) {
            m_services->GetFloatingWidget()->setVisible(true);
        }

        ui->stackedWidget->setCurrentWidget(ui->main);
    }
}

void MainWindow::OnLogoutClicked()
{
    if (!this->ui->stackedWidget) return;

    if (m_services->GetUserSession()->IsUser()) {
        if (ui->catalogListView) {
            ui->catalogListView->setModel(nullptr);
            ui->catalogListView->setItemDelegate(nullptr);
        }

        if (ui->purchasedListView) {
            ui->purchasedListView->setModel(nullptr);
            ui->purchasedListView->setItemDelegate(nullptr);
        }
    }
    m_services->ResetSession();

    if (this->ui->stackedWidget && this->ui->login) {
        this->ui->stackedWidget->setCurrentWidget(this->ui->login);
    }
}

void MainWindow::BuildDependencies() {
    m_services->EnsureControllers(this);
    if (!m_services->GetFloatingWidget()) {
        SetupFloatingMenu();
    }

    m_services->GetCatalog()->disconnect(this);
    connect(m_services->GetCatalog(), &CatalogController::ProductSelected, this, [this](const ProductInfo& product) {
        if (!m_services->GetProducts()) {
            return;
        }
        auto productColors = m_services->GetProducts()->GetAllProductsWithName(product);
        ShowProductOnPersonalPage(product, productColors);
    });

    m_services->GetCatalog()->Initialize(ui->catalogListView);
    m_services->GetProfile()->Initialize(ui->purchasedListView, ui->label_clientname, ui->groupBox_6);
}

void MainWindow::SetupServicesScrollArea()
{
    if (!m_purchase_methods_controller) {
        m_purchase_methods_controller.reset(new PurchaseMethodsController(this));
        connect(m_purchase_methods_controller.get(), &PurchaseMethodsController::OpenCatalogRequested, this, [this]() {
            ui->stackedWidget->setCurrentWidget(ui->main);
        });
    }
    m_purchase_methods_controller->Initialize(ui->purchaseMethodListView, m_services.get(), this);
}


void MainWindow::SelectionProcessing(const bool ok, const QStringView selected_type, const QStringView selected_color)
{
    if (!ok) {
        return;
    }

    if (!m_services->GetCatalog()) {
        return;
    }

    m_services->GetCatalog()->ApplyFilter(selected_type, selected_color);
    ui->stackedWidget->setCurrentWidget(ui->main);
}
void MainWindow::ShowProductOnPersonalPage(const ProductInfo& product, QList<ProductInfo>& m_current_productcolors_) {
    ui->label_name->setText(product.name_);
    ui->label_price->setText(FormatPrice(product.price_) + " руб.");
    m_current_product = product;
    // ui->label_color_index->setText(QString::number(m_current_color_index + 1) + "/" + QString::number(m_current_productcolors_.size()));

    QString imagePath = QDir::cleanPath(product.image_path_);
    QPixmap originalPixmap(imagePath);

    if (!originalPixmap.isNull()) {
        const int availableWidth = ui->groupBox_2
            ? qMax(ui->groupBox_2->width(), ui->groupBox_2->minimumWidth())
            : 900;
        const int imageWidth = qBound(520, availableWidth - 180, 700);
        const QPixmap scaledPixmap = originalPixmap.scaled(
            QSize(imageWidth, 285),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        ui->label_car_image->setPixmap(scaledPixmap);
        ui->label_car_image->setMinimumSize(imageWidth, 250);
        ui->label_car_image->setMaximumSize(imageWidth, 285);
        ui->label_car_image->setAlignment(Qt::AlignCenter);
        ui->label_car_image->show();
    }

    ui->pushButton_order->setVisible(product.stock_qty_ <= 0); // If product is not in stock, show order button
    ui->pushButton_to_pay->setVisible(product.stock_qty_ > 0); // If product is in stock, show to pay button

    ui->stackedWidget->setCurrentWidget(ui->personal);
    UpdatePersonalPageLayout();
}

void MainWindow::OnNextLeftClicked()
{
    // If there are other products with the same name, show the previous product
    auto m_current_productcolors_ = m_services->GetProducts()->GetAllProductsWithName(m_current_product);
    if (!m_current_productcolors_.isEmpty())
    {
        m_current_color_index = (m_current_color_index - 1 + m_current_productcolors_.size()) % m_current_productcolors_.size();
        m_current_product = m_current_productcolors_.at(m_current_color_index);
        ShowProductOnPersonalPage(m_current_productcolors_.at(m_current_color_index), m_current_productcolors_);
    }
}

void MainWindow::OnNextRightClicked()
{
    // If there are other products with the same name, show the next product
    auto m_current_productcolors_ = m_services->GetProducts()->GetAllProductsWithName(m_current_product);
    if (!m_current_productcolors_.isEmpty())
    {
        m_current_color_index = (m_current_color_index + 1) % m_current_productcolors_.size();
        m_current_product = m_current_productcolors_.at(m_current_color_index);
        ShowProductOnPersonalPage(m_current_productcolors_.at(m_current_color_index), m_current_productcolors_);
    }
}

void MainWindow::OnBackClicked()
{
    if (!m_services->GetUserSession()->IsUser()) return;

    // Reset current product state
    m_current_product = ProductInfo();
    m_current_color_index = 0;

    // Rebuild the main catalog view
    if (m_services->GetCatalog()) {
        m_services->GetCatalog()->ResetDefault();
    }

    ui->stackedWidget->setCurrentWidget(ui->main);
}

void MainWindow::OnInfoClicked()
{
    if (m_current_product.name_.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Не выбран автомобиль для просмотра информации.");
        return;
    }

    // Show product information
    QString info = QString("Название: %1\nЦена: %2 руб.\nЦвет: %3\nОписание: %4")
                       .arg(m_current_product.name_)
                       .arg(FormatPrice(m_current_product.price_))
                       .arg(m_current_product.color_)
                       .arg(m_current_product.description_.isEmpty() ? "Описание отсутствует." : m_current_product.description_);

    QMessageBox::information(this, "Информация об автомобиле", info);
}

void MainWindow::OnTestDriveClicked()
{
    auto strategy = CreatePurchaseRequestStrategy(PurchaseMethod::TestDrive);
    if (!strategy) {
        QMessageBox::warning(this, "Error", "Strategy for test drive is not available.");
        return;
    }
    strategy->Execute(this, m_services.get());
}

void MainWindow::OnToPayClicked()
{
    auto strategy = CreateSelectedCarActionStrategy(SelectedCarAction::Checkout);
    if (!strategy) {
        QMessageBox::warning(this, "Error", "Checkout strategy is not available.");
        return;
    }
    strategy->Execute(this, m_services.get(), m_current_product);
}

void MainWindow::OnOrderClicked()
{
    auto strategy = CreateSelectedCarActionStrategy(SelectedCarAction::Order);
    if (!strategy) {
        QMessageBox::warning(this, "Error", "Order strategy is not available.");
        return;
    }
    strategy->Execute(this, m_services.get(), m_current_product);
}

void MainWindow::OnNotificationsClicked()
{
    if (!m_services->GetUserSession()->IsAuthorized() || !m_services->GetNotifications()) {
        return;
    }
    m_services->GetNotifications()->ShowForUser(m_services->GetUserSession()->GetId(), this);
}

void MainWindow::OnSettingsClicked()
{
    if (!m_services->GetUserSession()->IsAuthorized()) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к настройкам.");
        return;
    }

    if (!m_settings_form) {
        m_settings_form.reset(new SettingsForm(m_services.get(), this));

        connect(m_settings_form.get(), &SettingsForm::ProfileSaved, this, [this](const QString& fullName, const QString&) {
            if (ui->label_clientname) {
                ui->label_clientname->setText(fullName + " — профиль");
            }
        });
        connect(m_settings_form.get(), &SettingsForm::ThemeChanged, this, [this](bool darkEnabled) {
            SetDarkThemeEnabled(darkEnabled);
        });

        connect(m_settings_form.get(), &QDialog::finished, this, [this]() {
            m_settings_form.reset();
        });
    }

    m_settings_form->show();
    m_settings_form->raise();
    m_settings_form->activateWindow();
}

void MainWindow::OnRegistrationClicked()
{
    m_services->EnsureControllers(this);

    bool registered = m_services->GetAuth()->RunRegistrationDialog(this);
    if (registered) {
        ui->lineEdit_login->clear();
        ui->lineEdit_password->clear();
    }
}
void MainWindow::OnProfileClicked()
{
    if (!m_services->GetUserSession()->IsUser()) {
        QMessageBox::warning(this, "Ошибка", "Авторизуйтесь для доступа к профилю.");
        return;
    }

    if (ui->stackedWidget->currentWidget() == ui->user_page) return;
    
    if (m_services->GetProfile()) {
        m_services->GetProfile()->ShowProfile(m_services->GetUserSession()->GetId(), m_services->GetUserSession()->GetName());
    }

    SetupServicesScrollArea();

    ui->stackedWidget->setCurrentWidget(ui->user_page);
}

void MainWindow::OnSortByColorClicked()
{
    if (m_services->GetUserSession()->IsAuthorized()) {
        if (m_services->GetUserSession()->IsUser())
        {
            bool ok;
            const QString selectedColor = GetItemFromThemedDialog(
                this,
                "Поиск по цветам",
                "Выберите цвет:",
                m_services->GetProducts()->GetAvailableColors(),
                &ok);

            SelectionProcessing(ok, QStringView(), selectedColor);
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::OnSearchClicked()
{
    bool ok;
    const QString term = GetTextFromThemedDialog(
        this,
        "Поиск",
        "Укажите поисковый запрос:",
        &ok);

    if (ok && !term.isEmpty())
    {
        if (!m_services->GetProducts() || !m_services->GetCatalog()) {
            QMessageBox::warning(this, "Поиск", "Каталог недоступен.");
            return;
        }

        int relevant_count = m_services->GetCatalog()->Search(term);
        if (relevant_count > 0)
        {
            ui->stackedWidget->setCurrentWidget(ui->main);
            QMessageBox::information(this, "Поиск", "Найдено " + QString::number(relevant_count) + " результатов по запросу «" + term + "».");
        }
        else
        {
            QMessageBox::warning(this, "Поиск", "Отсутствуют релевантные результаты.");
        }
    }
}

void MainWindow::OnSortByTypeClicked()
{
    if (m_services->GetUserSession()->IsAuthorized())
    {
        if (m_services->GetUserSession()->IsUser())
        {
            QStringList types;  // Список типов авто для выпадающего списка

            if (m_services->GetDatabase()) {
                types = m_services->GetDatabase()->GetCarTypeNames();
            }
            types << QString::fromUtf8("Все");

            bool ok;
            const QString selected_type = GetItemFromThemedDialog(
                this,
                "Поиск по типу авто",
                "Тип:",
                types,
                &ok);

            QString defaultColor;
            if (m_services->GetDatabase()) {
                defaultColor = m_services->GetDatabase()->GetDefaultCatalogColor();
            }

            if (selected_type == QString::fromUtf8("Все")) {
                SelectionProcessing(ok, QStringView(), defaultColor);
            } else {
                SelectionProcessing(ok, selected_type, defaultColor);
            }
            return;
        }
    }
    QMessageBox::warning(this, "Ошибка", "Чтобы переключаться по остальным разделам, необходимо авторизоваться как пользователь.");
}

void MainWindow::SetupFloatingMenu()
{
    m_services->EnsureFloatingWidget(this);

    m_services->GetFloatingWidget()->BuildFloatingMenu(
    58,                                               // Floating menu x position
    this->height(),                                   // Floating menu height
        [this]() { this->OnSortByTypeClicked(); },    // On sort by type clicked
        [this]() { this->OnSearchClicked(); },        // On search clicked
        [this]() { this->OnSortByColorClicked(); },   // On sort by color clicked
        [this]() { this->OnProfileClicked(); }        // On profile clicked
    );

    // Show floating menu
    if (m_services->GetFloatingWidget()) {
        m_services->GetFloatingWidget()->setVisible(false);
    }
}

void MainWindow::UpdateFloatingMenuPosition()
{
    if (m_services->GetFloatingWidget()) {
        // Update floating menu position
        int x = 58;  // Floating menu x position
        int y = (this->height() - m_services->GetFloatingWidget()->height()) / 2;  // Floating menu y position
        m_services->GetFloatingWidget()->move(x, y);
    }
}

void MainWindow::UpdatePersonalPageLayout()
{
    if (!ui || !ui->groupBox_2 || !ui->pushButton_next_left || !ui->pushButton_next_right) {
        return;
    }

    constexpr int arrowSize = 68;
    constexpr int sideMargin = 16;
    const int y = qMax(0, (ui->groupBox_2->height() - arrowSize) / 2);

    ui->pushButton_next_left->setGeometry(sideMargin, y, arrowSize, arrowSize);
    ui->pushButton_next_right->setGeometry(
        qMax(sideMargin, ui->groupBox_2->width() - arrowSize - sideMargin),
        y,
        arrowSize,
        arrowSize);
    ui->pushButton_next_left->raise();
    ui->pushButton_next_right->raise();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);  // Resize event

    // Update floating menu position
    UpdateFloatingMenuPosition();
    UpdatePersonalPageLayout();
}
