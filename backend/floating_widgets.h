#pragma once

#ifndef FLOATING_WIDGETSH_H
#define FLOATING_WIDGETSH_H

#include <QObject>
#include <QLabel>
#include <QListWidget>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QComboBox>

#include "database_handler.h"

class MainWindow;

class FloatingWidgets : public QWidget
{
    Q_OBJECT
public:
    explicit FloatingWidgets(std::shared_ptr<DatabaseHandler> db_manager, QWidget *parent = nullptr);

    [[maybe_unused]] inline void SetFloatingMenu(QWidget* floating_menu)  {
        if (floating_menu_){
            floating_menu_.reset();
            floating_menu_ = std::make_unique<QWidget>(floating_menu);
        }
    }
    inline QWidget* GetFloatingMenu() {
        return floating_menu_.get();
    }

    [[maybe_unused]] inline void SetSideMenu(QWidget* side_menu) {
        if (side_menu_) {
            side_menu_.reset();
            side_menu_ = std::make_unique<QWidget>(side_menu);
        }
    }
    inline QWidget* GetSideMenu(){
        return side_menu_.get();
    }

    [[maybe_unused]] inline void SetSideList(QListWidget* side_list) {
        side_list_ = side_list;
    }
    inline QListWidget* GetSideList(){
        return side_list_;
    }

    void BuildFloatingMenu(
        const std::function<void()>& onMoreClicked,
        const std::function<void()>& onSearchClicked,
        const std::function<void()>& onColorFilterClicked,
        const std::function<void()>& onUserProfileClicked
        );

    void BuildSideMenu(
        const std::function<void(const QString&)>& onItemClicked,
        const std::function<void()>& onViewAllClicked
        );

private:
    std::shared_ptr<DatabaseHandler> db_manager_;

    // Плавающее меню
    std::unique_ptr<QWidget> floating_menu_;

    // Боковое меню
    std::unique_ptr<QWidget> side_menu_;
    QListWidget* side_list_;
};

#endif // FLOATING_WIDGETSH_H
