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

#include "database_manager.h"
#include "user.h"
#include "table.h"

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

    void on_pushButton_cars_clicked();
    void on_pushButton_search_clicked();
    void on_pushButton_profile_clicked();

    void on_pushButton_back_clicked();

    void onCarSelected(const Car& car);
    void showCarColor(const Car& car);

    void on_pushButton_next_left_clicked();

    void on_pushButton_next_right_clicked();

    void on_pushButton_to_pay_clicked();

    void onSortByColorClicked();

private:
    Ui::MainWindow *ui;

    Car current_car_;
    QMap<QWidget*, Car> carCardsMap_;
    QVector<Car> carColors_; // Список вариантов автомобиля (разные цвета)
    int currentColorIndex_; // Индекс текущего цвета

    DatabaseManager db_manager_;
    std::unique_ptr<User> user_;
    std::unique_ptr<Table> table_;
    // std::unique_ptr<QTableWidget> table_services_;
    std::unique_ptr<QWidget> floating_menu_; // Плавающее меню
    std::unique_ptr<QComboBox> colorDropdown_; // Выпадающий список цветов

    std::unique_ptr<QWidget> side_widget_;
    QListWidget* side_list_;

    void SetupFloatingMenu();

    void DrawCars(QScrollArea* scrollArea, const QString& condition);

    void SetupSideMenu();

    bool eventFilter(QObject* obj, QEvent* event);

    QString FormatPrice(int price);

    QList<Car> GetCars(int user_id) const;

    void UpdateColorDropdown();
};
#endif // MAINWINDOW_H
