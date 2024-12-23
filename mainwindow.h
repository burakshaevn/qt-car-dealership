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

    void on_pushButton_profile_clicked();

    void on_pushButton_cars_clicked();

    void on_pushButton_back_clicked();

    void onCarSelected(const QString& name, const QString& color, double price, const QString& imagePath);

private:
    Ui::MainWindow *ui;

    Car current_car_;
    DatabaseManager db_manager_;
    std::unique_ptr<User> user_;
    std::unique_ptr<Table> table_;
    std::unique_ptr<QTableWidget> table_services_;

    std::unique_ptr<QWidget> side_widget_;
    QListWidget* side_list_;

    void SetupFloatingMenu();
    void DrawCars(const QString& condition);
    void SetupSideMenu();
};
#endif // MAINWINDOW_H
