#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QCalendarWidget>
#include <QScrollArea>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QComboBox>
#include <QCheckBox>
#include <QCryptographicHash>

#include <QGraphicsDropShadowEffect>

#include "database_handler.h"
#include "product_card.h"
#include "floating_widgets.h"
#include "cart.h"
#include "user.h"
#include "table.h"
#include "products.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void UpdateUser(const UserInfo& user, QWidget* parent);
    
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void on_pushButton_login_clicked();
    void on_pushButton_registration_clicked();
    void on_pushButton_logout_clicked();

    void on_pushButton_clean_cart_clicked();

    void on_pushButton_submit_cart_clicked();

    void ShowProductOnPersonalPage(const ProductInfo&, QList<ProductInfo>&);

    void on_pushButton_next_left_clicked();

    void on_pushButton_next_right_clicked();

    void on_pushButton_back_clicked();

    void on_pushButton_to_pay_clicked();

    void on_pushButton_info_clicked();

    void on_pushButton_test_drive_clicked();

    void on_pushButton_notifications_clicked();

    void on_pushButton_settings_clicked();

    bool CheckNotifications();

    // New service button handlers
    void handleServiceRequest();
    void handleRentalRequest();
    void handleLoanRequest();
    void handleInsuranceRequest();

    void AddNotification(const QString& title, const QString& message);
    void AddNotification(const QString& title, QWidget* content);

    void MarkNotificationsAsRead();

private:
    Ui::MainWindow *ui;

    ProductInfo current_product_;
    int current_color_index_ = 0; // Индекс текущего цвета

    // Управление базой данных
    std::shared_ptr<DatabaseHandler> db_manager_;

    // Управляет информацией о каждом автомобиле
    std::shared_ptr<Products> products_;

    // Управляет карточками товаров
    std::shared_ptr<ProductCard> product_card_;

    // Управляет корзиной.
    // Корзина хранится в статической памяти. Это значит, что после повторного запуска приложения
    // текущая корзина пользователя будет очищена.
    // Сама хэш-таблица хранит пару ключ-значение, в виде <название товара (QString), карточка (QWidget*)>
    std::shared_ptr<Cart> cart_;

    std::unique_ptr<User> user_;
    std::unique_ptr<Table> table_;

    // Диалог настроек и его поля
    std::unique_ptr<QDialog> settings_dialog_;
    std::unique_ptr<QLineEdit> settings_first_name_;
    std::unique_ptr<QLineEdit> settings_last_name_;
    std::unique_ptr<QLineEdit> settings_email_;
    std::unique_ptr<QLineEdit> settings_phone_;
    std::unique_ptr<QLineEdit> settings_password_;

    // Управляет плавающими виджетами: левым боковым меню каталога и правым плавающим меню
    std::unique_ptr<FloatingWidgets> floating_widgets_;

    QList<QPushButton*> service_buttons_; // Хранит кнопки сервисов для предотвращения их удаления

    // Инициализировать зависимости
    void BuildDependencies();

    // Создать содержимое и настроить расположение для плавающего меню (главная, профиль, и др.)
    void SetupFloatingMenu();

    // Обработка нажатий на кнопки из плавающего меню
    void SortByProductType();
    void ProfileClicked();
    void SortByColor();
    void SearchByTerm();

    void SelectionProcessing(const bool ok, const QStringView selected_type, const QStringView selected_color = QStringView());
    void SetupServicesScrollArea();

    // Функции обработки кнопок оплаты/очистки корзины
    void CleanCart();
    void ToPayCart();

    // Получить список названий купленных товаров
    QList<Products::ProductKey> GetPurchasedProducts(int user_id) const;

    // Функции для работы с уведомлениями
    void UpdateNotifications();
    void ClearNotifications();

    // Функция для перевода статусов заявок на кредит
    QString TranslateLoanStatus(const QString& status);
};

#endif // MAINWINDOW_H
