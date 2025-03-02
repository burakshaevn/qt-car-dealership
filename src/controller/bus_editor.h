#pragma once

#include <QMainWindow>
#include "ui_bus_editor.h"
#include "database_manager.h"
#include "mainwindow.h"
#include <QListWidget>

class BusEditor : public QDialog
{
	Q_OBJECT

public:
	BusEditor(QWidget *parent = nullptr, DatabaseManager* db_ = nullptr, TransportCatalogue* transport_catalogue_ = nullptr, Bus* bus_ = nullptr);
    ~BusEditor();

private slots:
    void on_button_edit_clicked();
    void on_button_append_2_clicked();
	void on_button_settings_clicked();

    void on_append_append_bus_clicked();
    void on_edit_save_clicked();
	void on_settings_save_clicked();

    void on_edit_append_stop_clicked();
    void on_append_append_stop_clicked();

private:
	Ui::BusEditorClass ui;
	// Указатель на текущий автобус, для которого открыто окно BusEditor
	Bus* current_bus;
	// Указатель на новый маршрут, который будет добавляться через окно «Добавление».
	// Изначально он будет nullptr. Как только открывается окно «Добавление», он
	// инициализируется new Bus(), куда впоследствии будет занесена информация из лейблов QLabel и QComboBox.  
	DatabaseManager* db_manager;
	TransportCatalogue* transport_catalogue;

	// Хранит остановки current_bus.
	// Отдельный контейнер создаётся для того, чтобы при сохранении изменений мы могли выполнить синхронизацию 
	// с кэшем и базой данных.
	// Почему сразу не работать с current_bus->stops? Потому что пользователь может не прожать кнопку «Сохранить»,
	// что говорит нам об отмене изменений.
    std::vector<std::shared_ptr<const Stop>> cache_stops_;
    std::vector<std::shared_ptr<const Stop>> cache_new_bus_stops_;

    void DisplayCurrentBusToEditPage(QListWidget* listWidgetStops, std::vector<std::shared_ptr<const Stop>>& collection_);

    void FillComboBox();

	void SetupStopListWidget();

    // Функция для обновления порядка остановок после перемещения
	void UpdateStopOrder(); 

    // Функция для создания виджета остановки с меткой и кнопкой удаления
    QWidget* CreateStopWidget(std::shared_ptr<const Stop> stop, std::vector<std::shared_ptr<const Stop>>& cache_array, QListWidget* listWidgetStops);

    // Удаление остановки из `stops_` и интерфейса
    void on_edit_delete_stop_clicked(std::shared_ptr<const Stop> stop, std::vector<std::shared_ptr<const Stop>>& cache_array, QListWidget* listWidgetStops);

    // Функция для обработки изменения позиции остановки
    void on_stop_position_changed(int new_position, std::shared_ptr<const Stop> stop, std::vector<std::shared_ptr<const Stop>>& cache_array, QListWidget* listWidgetStops);
};
