#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QCalendarWidget>
#include <QDir>
#include <QListWidget>
#include <QDockWidget>
#include <QWidget>

#include <QGraphicsDropShadowEffect>

#include "database_handler.h"
#include "product_card.h"
#include "floating_widgets.h"
#include "cart.h"
#include "user.h"
#include "table.h"
#include "Products.h"

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

    void on_pushButton_logout_clicked();

    void on_pushButton_clean_cart_clicked();

    void on_pushButton_submit_cart_clicked();

    void ShowProductOnPersonalPage(const ProductInfo&);

    void on_pushButton_next_left_clicked();

    void on_pushButton_next_right_clicked();

    void on_pushButton_back_clicked();

    void on_pushButton_to_pay_clicked();

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

    // Управляет плавающими виджетами: левым боковым меню каталога и правым плавающим меню
    std::unique_ptr<FloatingWidgets> floating_widgets_;

    // Инициализировать зависимости
    void BuildDependencies();

    // Обработка нажатий на кнопки "Ещё", "Профиль" и "Корзина" из плавающего меню
    void MoreClicked();
    void ProfileClicked();
    void SortByColorClicked();

    // Функции обработки кнопок оплаты/очистки корзины
    void CleanCart();
    void ToPayCart();

    // Создать содержимое и настроить расположение для плавающего меню (главная, профиль, и др.)
    void SetupFloatingMenu();

    // Получить список названий купленных товаров
    QList<Products::ProductKey> GetPurchasedProducts(int user_id) const;

};

#endif // MAINWINDOW_H
