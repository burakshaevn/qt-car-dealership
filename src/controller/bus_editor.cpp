#include "bus_editor.h"

BusEditor::BusEditor(QWidget* parent, DatabaseManager* db_, TransportCatalogue* transport_catalogue_, Bus* bus_)
	: QDialog(parent) 
	, db_manager(db_)
	, transport_catalogue(transport_catalogue_)
	, current_bus(bus_)
	, cache_stops_(bus_->stops)
{
	ui.setupUi(this);
	this->setFixedSize(657, 671);
    FillComboBox();
    DisplayCurrentBusToEditPage(ui.listWidgetStops, cache_stops_);
}

BusEditor::~BusEditor() = default;

void BusEditor::on_button_edit_clicked() {
	if (db_manager->Open()) {
        ui.stackedWidget->setCurrentWidget(ui.edit);
        DisplayCurrentBusToEditPage(ui.listWidgetStops, cache_stops_);
	}
	else {
		QMessageBox::critical(this, "Ошибка", "Выполните подключение к базе данных.");
	}
}
void BusEditor::on_button_append_2_clicked() {
    if (db_manager->Open()) {
        ui.stackedWidget->setCurrentWidget(ui.append);
    }
    else {
        QMessageBox::critical(this, "Ошибка", "Выполните подключение к базе данных.");
    }
}
void BusEditor::on_button_settings_clicked() {
	if (db_manager->Open()) {
		ui.stackedWidget->setCurrentWidget(ui.settings);

		QSqlQuery query = db_manager->ExecuteSelectQuery(QString("SELECT * FROM routing_settings;"));
		 
		if (query.next()) {  
			ui.lineEdit_bus_velocity->setText(query.value("bus_velocity").toString());
			ui.lineEdit_bus_wait_time->setText(query.value("bus_wait_time").toString());
		}
		else {
			QMessageBox::warning(this, "Предупреждение", "Данные не найдены.");
		}
	}
	else {
		QMessageBox::critical(this, "Ошибка", "Выполните подключение к базе данных.");
	}
}

// Кнопка 'Сохранить' на странице 'Настройки'
void BusEditor::on_settings_save_clicked() {
	bool ok1, ok2;
	double busVelocity = ui.lineEdit_bus_velocity->text().toDouble(&ok1);
	int busWaitTime = ui.lineEdit_bus_wait_time->text().toInt(&ok2);

	if (db_manager->Open()) {
		if (ok1 && ok2) {
			if (db_manager->UpdateRoutingSettings(busVelocity, busWaitTime)) {
				QMessageBox::information(this, "Уведомление", "Данные обновлены.");
			}
		}
		else {
			QMessageBox::warning(this, "Ошибка", "Укажите корректные значения скорости и времени ожидания.");
		}
	}
	else {
		QMessageBox::critical(this, "Ошибка", "Выполните подключение к базе данных.");
	}
}

// Кнопка 'Добавить' на странице 'Редактировать'
void BusEditor::on_edit_append_stop_clicked() {
	QString stopname = ui.lineEdit_find_stopname->text();
    if (auto stop = transport_catalogue->FindStop(stopname); stop != nullptr) {
        cache_stops_.push_back(stop);
		ui.lineEdit_find_stopname->clear();
        DisplayCurrentBusToEditPage(ui.listWidgetStops, cache_stops_);
		QMessageBox::information(this, "", QString("Остановка %1 добавлена в список проходимых остановок по маршруту.").arg(stopname));
	}
	else {
		QMessageBox::critical(this, "Ошибка", QString("Остановка %1 несуществует.").arg(stopname));
	}
}

// Кнопка 'Добавить' на странице 'Добавление'
void BusEditor::on_append_append_stop_clicked()
{
	QString stopname = ui.lineEdit_find_stopname_2->text();
    if (auto stop = transport_catalogue->FindStop(stopname); stop != nullptr) {
        cache_new_bus_stops_.push_back(stop);
		ui.lineEdit_find_stopname_2->clear();
        DisplayCurrentBusToEditPage(ui.listWidgetStops_2, cache_new_bus_stops_);
		QMessageBox::information(this, "", QString("Остановка %1 добавлена в список проходимых остановок по маршруту.").arg(stopname));
	}
	else {
		QMessageBox::critical(this, "Ошибка", QString("Остановка %1 несуществует.").arg(stopname));
	}
}

void BusEditor::SetupStopListWidget() {
	ui.listWidgetStops->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listWidgetStops->setDragDropMode(QAbstractItemView::InternalMove);
	ui.listWidgetStops->setDefaultDropAction(Qt::MoveAction);

	connect(ui.listWidgetStops->model(), &QAbstractItemModel::rowsMoved, this, &BusEditor::UpdateStopOrder);
}

// Функция для обновления порядка остановок после перемещения
void BusEditor::UpdateStopOrder() {
	cache_stops_.clear();
	for (int i = 0; i < ui.listWidgetStops->count(); ++i) {
		QListWidgetItem* item = ui.listWidgetStops->item(i);
        auto stop = item->data(Qt::UserRole).value<std::shared_ptr<const Stop>>();
		cache_stops_.push_back(stop);
	}
}

// Функция для создания виджета остановки с меткой и кнопкой удаления
QWidget* BusEditor::CreateStopWidget(std::shared_ptr<const Stop> stop, std::vector<std::shared_ptr<const Stop>>& cache_array, QListWidget* listWidgetStops) {
	QWidget* widget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(widget);
	layout->setContentsMargins(0, 0, 0, 0);

	QLabel* label = new QLabel(stop->name, widget);
	label->setStyleSheet("font: 500 10pt 'JetBrains Mono'; color: #363636;");
	// Поиск индекса элемента stop в stops_
	auto it = std::find(cache_array.begin(), cache_array.end(), stop);
	int index = (it != cache_array.end()) ? std::distance(cache_array.begin(), it) : 0;

	// Создание поля для ввода позиции с учетом найденного индекса
	QLineEdit* positionEdit = new QLineEdit(QString::number(index + 1), widget);
	positionEdit->setFixedSize(48, 17);
	positionEdit->setAlignment(Qt::AlignCenter);
	positionEdit->setValidator(new QIntValidator(1, 999, this));
	positionEdit->move(10, 0);
	positionEdit->setStyleSheet("font: 500 10pt 'JetBrains Mono'; background-color: transparent; border: none; color: #363636;");

	QPushButton* deleteButton = new QPushButton(widget);
    deleteButton->setIcon(QIcon(":/close.svg"));
	deleteButton->setIconSize(QSize(17, 17));
	deleteButton->setFixedSize(17, 17);
	deleteButton->move(8, 3);
	deleteButton->setStyleSheet("border-radius: 0px;");

	layout->addWidget(positionEdit);
	layout->addWidget(label);
	layout->addWidget(deleteButton);
	widget->setLayout(layout);

    connect(deleteButton, &QPushButton::clicked, widget, [this, stop, &cache_array, listWidgetStops]() {
        on_edit_delete_stop_clicked(stop, cache_array, listWidgetStops);
    });

    connect(positionEdit, &QLineEdit::editingFinished, widget, [this, positionEdit, stop, &cache_array, listWidgetStops]() {
        int new_position = positionEdit->text().toInt() - 1;
        on_stop_position_changed(new_position, stop, cache_array, listWidgetStops);
    });

	return widget;
}

// Удаление остановки из `stops_` и интерфейса
void BusEditor::on_edit_delete_stop_clicked(std::shared_ptr<const Stop> stop, std::vector<std::shared_ptr<const Stop>>& cache_array, QListWidget* listWidgetStops) {
	auto it = std::find(cache_array.begin(), cache_array.end(), stop);
	if (it != cache_array.end()) {
		cache_array.erase(it);
	}
    DisplayCurrentBusToEditPage(listWidgetStops, cache_array);
}

// Функция для обработки изменения позиции остановки
void BusEditor::on_stop_position_changed(int new_position, std::shared_ptr<const Stop> stop, std::vector<std::shared_ptr<const Stop>>& cache_array, QListWidget* listWidgetStops) {
	auto it = std::find(cache_array.begin(), cache_array.end(), stop);
	if (it != cache_array.end()) {
		cache_array.erase(it);
		cache_array.insert(cache_array.begin() + new_position, stop);
	}
    DisplayCurrentBusToEditPage(listWidgetStops, cache_array);
}

// Добавление нового автобуса
void BusEditor::on_append_append_bus_clicked() {
	if (db_manager->Open()) {
		//new_bus = std::make_unique<Bus>();
		Bus new_bus;
        new_bus.name = ui.lineEdit_busname_3->text();
        if (transport_catalogue->FindBus(new_bus.name) != nullptr) {
            QMessageBox::critical(this, "Ошибка", "Маршрут с именем " + new_bus.name + " уже существует.");
            return;
        }
        new_bus.color_index = ui.lineEdit_color_index_3->text().toInt();
        if (transport_catalogue->FindBus(new_bus.name) != nullptr) {
            QMessageBox::critical(this, "Ошибка", "Маршрут с цветом " + QString::number(new_bus.color_index) + " уже существует.");
            return;
        }

        new_bus.bus_type = StringToBusType(ui.comboBox_bustype_3->currentText());
        new_bus.capacity = static_cast<size_t>(ui.lineEdit_capacity_3->text().toInt());
        new_bus.is_roundtrip = ui.comboBox_is_roundtrip_3->currentText() == "Да";
        new_bus.has_wifi = ui.comboBox_is_wifi_3->currentText() == "Да";
        new_bus.has_sockets = ui.comboBox_is_sockets_3->currentText() == "Да";
        new_bus.is_day = ui.comboBox_is_day_bus_3->currentText() == "Да";
        new_bus.is_night = ui.comboBox_is_night_bus_3->currentText() == "Да";
        new_bus.is_available = ui.comboBox_is_available_3->currentText() == "Да";
        new_bus.price = ui.lineEdit_price_3->text().toDouble();

        if (!new_bus.stops.empty()) {
            QStringList missingDistances;
            for (size_t i = 0; i < cache_new_bus_stops_.size() - 1; ++i) {
                const Stop* from_stop = cache_new_bus_stops_[i].get();
                const Stop* to_stop = cache_new_bus_stops_[i + 1].get();

                int distance = db_manager->GetDistance(from_stop, to_stop);
                if (distance == 0) {
                    missingDistances.append(QString("%1 → %2").arg(from_stop->name, to_stop->name));
                }
            }

            if (!missingDistances.isEmpty()) {
                QString message = "Не найдены расстояния между следующими остановками:\n" + missingDistances.join("\n");
                QMessageBox::warning(this, "Недостающие расстояния", message);
                // Прерываем сохранение, если есть недостающие расстояния
                return;
            }
        }
        new_bus.stops = cache_new_bus_stops_;
        db_manager->UpdateBus(&new_bus);
		//auto shared_bus = std::make_shared<Bus>(std::move(*new_bus));
		transport_catalogue->UpdateBus(&new_bus);

        QMessageBox::information(this, "", "Новый маршрут успешно добавлен.");
    }
    else {
        QMessageBox::critical(this, "Ошибка", "Выполните подключение к базе данных.");
    }
}

// Кнопка 'Сохранить' на странице 'Редактирование'
void BusEditor::on_edit_save_clicked() {
	if (db_manager->Open()) {
        //std::shared_ptr<Bus> shared_bus = std::make_shared<Bus>();
		Bus current_bus;
        current_bus.name = ui.lineEdit_busname_2->text();
        current_bus.bus_type = StringToBusType(ui.comboBox_bustype_2->currentText());
        current_bus.capacity = static_cast<size_t>(ui.lineEdit_capacity_2->text().toInt());
        current_bus.is_roundtrip = ui.comboBox_is_roundtrip_2->currentText() == "Да";
        current_bus.color_index = ui.lineEdit_color_index_2->text().toInt();
        current_bus.has_wifi = ui.comboBox_is_wifi_2->currentText() == "Да";
        current_bus.has_sockets = ui.comboBox_is_sockets_2->currentText() == "Да";
        current_bus.is_day = ui.comboBox_is_day_bus_2->currentText() == "Да";
        current_bus.is_night = ui.comboBox_is_night_bus_2->currentText() == "Да";
        current_bus.is_available = ui.comboBox_is_available_2->currentText() == "Да";
        current_bus.price = ui.lineEdit_price_2->text().toDouble();
        current_bus.stops = cache_stops_;

		if (!cache_stops_.empty()) {
			QStringList missingDistances;
			for (size_t i = 0; i < cache_stops_.size() - 1; ++i) {
                const Stop* from_stop = cache_stops_[i].get();
                const Stop* to_stop = cache_stops_[i + 1].get();

				int distance = db_manager->GetDistance(from_stop, to_stop);
				if (distance == 0) {
					missingDistances.append(QString("%1 → %2").arg(from_stop->name, to_stop->name));
				}
			}

			if (!missingDistances.isEmpty()) {
				QString message = "Не найдены расстояния между следующими остановками:\n" + missingDistances.join("\n");
				QMessageBox::warning(this, "Недостающие расстояния", message);
				// Прерываем сохранение, если есть недостающие расстояния
				return;
			}
        }
        db_manager->UpdateBus(&current_bus);
        transport_catalogue->UpdateBus(&current_bus);
        QMessageBox::information(this, "", "Информация о маршруте обновлена.");
	}
	else {
		QMessageBox::critical(this, "Ошибка", "Выполните подключение к базе данных.");
	}
}

void BusEditor::DisplayCurrentBusToEditPage(QListWidget* listWidgetStops, std::vector<std::shared_ptr<const Stop>>& collection_) {
	if (db_manager->Open()) {
		QSqlQuery query = db_manager->ExecuteSelectQuery(
			QString("SELECT * FROM buses WHERE name = '%1';").arg(current_bus->name)
		);

		while (query.next()) {
			QString name = query.value("name").toString();
			QString busType = query.value("bus_type").toString();
			int capacity = query.value("capacity").toInt();
			bool isRoundTrip = query.value("is_roundtrip").toBool();
			int colorIndex = query.value("color_index").toInt();
			bool hasWifi = query.value("has_wifi").toBool();
			bool hasSockets = query.value("has_sockets").toBool();
			bool isNightBus = query.value("is_night").toBool();
			bool isAvailable = query.value("is_available").toBool();
			double price = query.value("price").toDouble();
            bool isDayBus = query.value("is_day").toBool();

			ui.lineEdit_busname_2->setText(name);
			ui.comboBox_bustype_2->setCurrentText(busType);
			ui.lineEdit_capacity_2->setText(QString::number(capacity));
			ui.comboBox_is_roundtrip_2->setCurrentText(isRoundTrip ? "Да" : "Нет");
			ui.lineEdit_color_index_2->setText(QString::number(colorIndex));
			ui.comboBox_is_wifi_2->setCurrentText(hasWifi ? "Да" : "Нет");
			ui.comboBox_is_sockets_2->setCurrentText(hasSockets ? "Да" : "Нет");
            ui.comboBox_is_day_bus_2->setCurrentText(isDayBus ? "Да" : "Нет");
			ui.comboBox_is_night_bus_2->setCurrentText(isNightBus ? "Да" : "Нет");
			ui.comboBox_is_available_2->setCurrentText(isAvailable ? "Да" : "Нет");
			ui.lineEdit_price_2->setText(QString::number(price, 'f', 2)); 

			listWidgetStops->clear();
            for (auto& stop : collection_) {
				QListWidgetItem* item = new QListWidgetItem(listWidgetStops);
                QWidget* stopWidget = CreateStopWidget(stop, collection_, listWidgetStops);
				item->setSizeHint(stopWidget->sizeHint());
				listWidgetStops->addItem(item);
				listWidgetStops->setItemWidget(item, stopWidget);
			}
		}
	}
	else {
		QMessageBox::critical(this, "Ошибка", "Выполните подключение к базе данных.");
	}
}

void BusEditor::FillComboBox()
{
	// Page: edit; ComboBox: bus_type
	ui.comboBox_bustype_2->addItem("undefined");
	ui.comboBox_bustype_2->setCurrentText("undefined");
	ui.comboBox_bustype_2->addItem("autobus");
	ui.comboBox_bustype_2->addItem("electrobus");
	ui.comboBox_bustype_2->addItem("trolleybus");

	// Page: edit; ComboBox: is_roundtrip
	ui.comboBox_is_roundtrip_2->addItem("Не выбрано");
	ui.comboBox_is_roundtrip_2->setCurrentText("Не выбрано");
	ui.comboBox_is_roundtrip_2->addItem("Да");
	ui.comboBox_is_roundtrip_2->addItem("Нет");

	// Page: edit; ComboBox: has_wifi
	ui.comboBox_is_wifi_2->addItem("Не выбрано");
	ui.comboBox_is_wifi_2->setCurrentText("Не выбрано");
	ui.comboBox_is_wifi_2->addItem("Да");
	ui.comboBox_is_wifi_2->addItem("Нет");

	// Page: edit; ComboBox: has_sockets
	ui.comboBox_is_sockets_2->addItem("Не выбрано");
	ui.comboBox_is_sockets_2->setCurrentText("Не выбрано");
	ui.comboBox_is_sockets_2->addItem("Да");
	ui.comboBox_is_sockets_2->addItem("Нет");

	// Page: edit; ComboBox: is_day_bus
	ui.comboBox_is_day_bus_2->addItem("Не выбрано");
	ui.comboBox_is_day_bus_2->setCurrentText("Не выбрано");
	ui.comboBox_is_day_bus_2->addItem("Да");
	ui.comboBox_is_day_bus_2->addItem("Нет");

	// Page: edit; ComboBox: is_night_bus
	ui.comboBox_is_night_bus_2->addItem("Не выбрано");
	ui.comboBox_is_night_bus_2->setCurrentText("Не выбрано");
	ui.comboBox_is_night_bus_2->addItem("Да");
	ui.comboBox_is_night_bus_2->addItem("Нет");

	// Page: edit; ComboBox: is_available
	ui.comboBox_is_available_2->addItem("Не выбрано");
	ui.comboBox_is_available_2->setCurrentText("Не выбрано");
	ui.comboBox_is_available_2->addItem("Да");
	ui.comboBox_is_available_2->addItem("Нет");

	// Page: append; ComboBox: bus_type
	ui.comboBox_bustype_3->addItem("undefined");
	ui.comboBox_bustype_3->setCurrentText("undefined");
	ui.comboBox_bustype_3->addItem("autobus");
	ui.comboBox_bustype_3->addItem("electrobus");
	ui.comboBox_bustype_3->addItem("trolleybus");

	// Page: append; ComboBox: is_roundtrip
	ui.comboBox_is_roundtrip_3->addItem("Не выбрано");
	ui.comboBox_is_roundtrip_3->setCurrentText("Не выбрано");
	ui.comboBox_is_roundtrip_3->addItem("Да");
	ui.comboBox_is_roundtrip_3->addItem("Нет");

	// Page: append; ComboBox: has_wifi
	ui.comboBox_is_wifi_3->addItem("Не выбрано");
	ui.comboBox_is_wifi_3->setCurrentText("Не выбрано");
	ui.comboBox_is_wifi_3->addItem("Да");
	ui.comboBox_is_wifi_3->addItem("Нет");

	// Page: append; ComboBox: has_sockets
	ui.comboBox_is_sockets_3->addItem("Не выбрано");
	ui.comboBox_is_sockets_3->setCurrentText("Не выбрано");
	ui.comboBox_is_sockets_3->addItem("Да");
	ui.comboBox_is_sockets_3->addItem("Нет");

	// Page: append; ComboBox: is_day_bus
	ui.comboBox_is_day_bus_3->addItem("Не выбрано");
	ui.comboBox_is_day_bus_3->setCurrentText("Не выбрано");
	ui.comboBox_is_day_bus_3->addItem("Да");
	ui.comboBox_is_day_bus_3->addItem("Нет");

	// Page: append; ComboBox: is_night_bus
	ui.comboBox_is_night_bus_3->addItem("Не выбрано");
	ui.comboBox_is_night_bus_3->setCurrentText("Не выбрано");
	ui.comboBox_is_night_bus_3->addItem("Да");
	ui.comboBox_is_night_bus_3->addItem("Нет");

	// Page: append; ComboBox: is_available
	ui.comboBox_is_available_3->addItem("Не выбрано");
	ui.comboBox_is_available_3->setCurrentText("Не выбрано");
	ui.comboBox_is_available_3->addItem("Да");
	ui.comboBox_is_available_3->addItem("Нет");
}
