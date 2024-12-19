#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->login);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_login_clicked()
{
    if (ui->lineEdit_login->text().isEmpty() && ui->lineEdit_password->text().isEmpty()){
        QMessageBox::critical(this, "Авторизация", "Для авторизации требуется ввод логина и пароля.");
        return;
    }
    else{
        QMessageBox::information(this, "Авторизация", "Неверный логин или пароль.");
        return;
    }

    QMessageBox::information(this, "Авторизация", "Авторизация прошла успешно. Текущий статус — администратор.");
}

