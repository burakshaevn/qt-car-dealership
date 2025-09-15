#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimeEdit>
#include <QCheckBox>
#include <QCryptographicHash>
#include <QGraphicsDropShadowEffect>

#include "database_handler.h"
#include "product_card.h"
#include "user.h"
#include "table.h"
#include "products.h"
#include "floating_widget.h"
#include "notifications_handler.h"

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

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_registration_clicked();

    void on_pushButton_logout_clicked();

    void on_pushButton_next_left_clicked();

    void on_pushButton_next_right_clicked();

    void on_pushButton_back_clicked();

    void on_pushButton_to_pay_clicked();

    void on_pushButton_info_clicked();

    void on_pushButton_test_drive_clicked();

    void on_pushButton_notifications_clicked();

    void on_pushButton_settings_clicked();

    void on_btn_profile_clicked();

    void on_btn_sortByColor_clicked();

    void on_btn_search_clicked();

    void on_btn_sortByType_clicked();

private:
    Ui::MainWindow *ui;

    ProductInfo m_current_product;                      ///< Информация о текущем товаре, который выбрал пользователь
    int m_current_color_index = 0;                      ///< Индекс текущего цвета у товара, который выбрал пользователь

    QSharedPointer<DatabaseHandler> m_database_manager; ///< Предоставляет интерфейс управления подключением и отправка запросов базе данных

    QSharedPointer<Products> m_products;                ///< Предоставляет интерфейс для коллективного управления карточками товаров (автомобилей)

    QSharedPointer<FloatingWidget> m_floating_widget;   ///< Предоставляет интерфейс для управления левым боковым меню

    QSharedPointer<ProductCard> m_product_cards;        ///< Предоставляет интерфейс для создания карточки товара (автомобиля)

    QScopedPointer<User> m_user;                        ///< Предоставляет интерфейс для управления данными аккаунта

    QScopedPointer<Table> m_table;                      ///< Предоставляет интерфейс для создания и управления админ-таблицами

    QScopedPointer<NotificationsHandler> m_notifications_handler; ///< Предоставляет интерфейс для управления уведомлениями

    // TODO: refactoring and remove
    // Диалог настроек и его поля
    QScopedPointer<QDialog> m_settings_dialog;
    QScopedPointer<QLineEdit> m_settings_first_name;
    QScopedPointer<QLineEdit> settings_last_name_;
    QScopedPointer<QLineEdit> settings_email_;
    QScopedPointer<QLineEdit> settings_phone_;
    QScopedPointer<QLineEdit> settings_password_;

    /*!
     * \brief Инициализация зависимостей
     */
    void BuildDependencies();
    
    /*!
     * \brief Инициализация бокового меню
     */
    void SetupFloatingMenu();

    /*!
     * \brief Загружает доступные услуги для клиента в ScrollArea на странице пользователя
     * \param ok -
     * \param selected_type -
     * \param selected_color -
     */
    void SelectionProcessing(const bool ok, const QStringView selected_type, const QStringView selected_color = QStringView());

    /*!
     * \brief Загружает доступные услуги для клиента в ScrollArea на странице пользователя
     */
    void SetupServicesScrollArea();

    /*!
     * \brief Получение списка названий купленных товаров
     * \param id пользователя, для которого выполняется запрос к БД
     * \returns Список, внутри каждой ячейки которого tuple<название_товара, цвет_товара>
     */
    QList<Products::ProductKey> GetPurchasedProducts(int m_userid) const;

    /*!
     * \brief Настраивает страницу с информацей о конкретном автомобиле
     * \param const ProductInfo& - характеристики, информация об автомобиле
     * \param QList<ProductInfo>& - список с доступными цветами конкретного автомобиля
     */
    void ShowProductOnPersonalPage(const ProductInfo&, QList<ProductInfo>&);

    /*!
     * \brief Обработчик записи автомобиля в сервис
     */
    void ServiceRequestHandler();

    /*!
     * \brief Обработчик передачи автомобиля в аренду
     */
    void RentalRequestHandler();

    /*!
     * \brief Обработчик оформления заявки на кредит
     */
    void LoanRequestHandler();

    /*!
     * \brief Обработчик оформления заявки на страхование
     */
    void InsuranceRequestHandler();
};

#endif // MAINWINDOW_H
