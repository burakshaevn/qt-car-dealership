#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QDialog>
#include <QObject>

#include "stdafx.h"
#include "database_manager.h"
#include "bus_editor.h"
#include "dialog_editor.h"
#include <algorithm>

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
    virtual ~MainWindow();

    using Value = std::variant<QString, std::runtime_error>;  

private slots:
    void on_button_buses_clicked();
    void on_button_map_clicked();
    void on_button_stops_clicked();
    void on_button_db_clicked();
    void on_button_distances_clicked();

    void on_search_bus_clicked();
    void on_reset_all_filters_clicked();

    void on_stops_append_clicked();
    void on_stops_search_clicked();

    void on_search_distance_clicked();
    void on_add_distance_clicked();
    void on_delete_distance_clicked();

    void on_connect_to_db_clicked();
    void on_connect_to_db_default_clicked();

    void on_lineEdit_find_stopname_textEdited(const QString &arg1);

    void on_stops_clear_clicked();

    void on_show_colors_clicked();

private:
    Ui::MainWindow *ui;
    DatabaseManager db_manager_;
    TransportCatalogue transport_catalogue_;

    void SetLabelSettings();

    void DisplayMapOnLabel(const QString& bus_name);
    Value JsonToSVG(const QString& bus_name);

    void ClearScrollWidget(QLayout* layout);
    void ClearSearchFiltres();

    void EditBus(Bus* bus);
    void InfoBus(Bus* bus);
    void DeleteBus(Bus* bus, QVBoxLayout* layout, QLabel* background);
    void DrawBus(Bus* bus, const bool show_color, QVBoxLayout* layout);
    void DrawRelevantBuses();

    void EditStop(const std::shared_ptr<const Stop>& stop);
    void InfoStop(const std::shared_ptr<const Stop>& stop);
    void DeleteStop(const std::shared_ptr<const Stop>& stop);
    void DrawStop(const std::shared_ptr<const Stop>& stop, QVBoxLayout* layout);
    void DrawRelevantStops();

    void LoadDistances();
    void DeleteDistance();
    void AddDistance();
};

#endif // MAINWINDOW_H
