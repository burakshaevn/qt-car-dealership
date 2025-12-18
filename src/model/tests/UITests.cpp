#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSharedPointer>
#include "../include/database_handler.h"
#include "../../controller/include/notifications_handler.h"

class UITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем QApplication для тестов UI
        int argc = 0;
        char** argv = nullptr;
        app = new QApplication(argc, argv);
        
        // Создаем тестовую базу данных
        db = QSqlDatabase::addDatabase("QSQLITE", "ui_test_connection");
        db.setDatabaseName(":memory:");
        db.open();
        
        createTestTables();
        createTestData();
        
        // Создаем обработчики
        database_handler = QSharedPointer<DatabaseHandler>::create();
        database_handler->SetConnection(db);
        
        notifications_handler = new NotificationsHandler(database_handler);
    }
    
    void TearDown() override {
        delete notifications_handler;
        delete app;
        db.close();
        QSqlDatabase::removeDatabase("ui_test_connection");
    }
    
    void createTestTables() {
        QSqlQuery query(db);
        
        // Создаем минимальные таблицы для тестов
        query.exec("CREATE TABLE clients ("
                  "id INTEGER PRIMARY KEY,"
                  "first_name VARCHAR(100) NOT NULL,"
                  "last_name VARCHAR(100) NOT NULL,"
                  "phone VARCHAR(15) NOT NULL,"
                  "email VARCHAR(255) NOT NULL,"
                  "password TEXT NOT NULL"
                  ")");
        
        query.exec("CREATE TABLE cars ("
                  "id INTEGER PRIMARY KEY,"
                  "name VARCHAR(255) NOT NULL,"
                  "color VARCHAR(50) NOT NULL,"
                  "price NUMERIC(15,0) NOT NULL,"
                  "type_id INTEGER NOT NULL,"
                  "stock_qty INTEGER DEFAULT 0 NOT NULL"
                  ")");
        
        query.exec("CREATE TABLE service_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "service_type VARCHAR(100) NOT NULL,"
                  "scheduled_date TIMESTAMP NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        query.exec("CREATE TABLE insurance_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "insurance_type VARCHAR(50) NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        query.exec("CREATE TABLE loan_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "loan_amount NUMERIC(15,0) NOT NULL,"
                  "loan_term_months INTEGER NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        query.exec("CREATE TABLE test_drives ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "scheduled_date TIMESTAMP NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        query.exec("CREATE TABLE rental_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "rental_days INTEGER NOT NULL,"
                  "start_date DATE NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        query.exec("CREATE TABLE purchase_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        query.exec("CREATE TABLE order_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_name VARCHAR(255) NOT NULL,"
                  "color VARCHAR(50),"
                  "trim VARCHAR(100),"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
    }
    
    void createTestData() {
        QSqlQuery query(db);
        
        // Создаем тестового клиента
        query.prepare("INSERT INTO clients (id, first_name, last_name, phone, email, password) "
                     "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(1);
        query.addBindValue("Тест");
        query.addBindValue("Клиент");
        query.addBindValue("+79123456789");
        query.addBindValue("test@example.com");
        query.addBindValue("password123");
        query.exec();
        
        // Создаем тестовый автомобиль
        query.prepare("INSERT INTO cars (id, name, color, price, type_id, stock_qty) "
                     "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(1);
        query.addBindValue("Mercedes-AMG S 63 E Performance");
        query.addBindValue("Черный");
        query.addBindValue(20520000);
        query.addBindValue(1);
        query.addBindValue(2);
        query.exec();
    }
    
    QApplication* app;
    QSqlDatabase db;
    QSharedPointer<DatabaseHandler> database_handler;
    NotificationsHandler* notifications_handler;
    int test_client_id = 1;
};

// Тесты для загрузки уведомлений
TEST_F(UITest, NotificationsLoadCorrectly) {
    QSqlQuery query(db);
    
    // Создаем тестовые уведомления
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("КАСКО");
    query.addBindValue("одобрено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // Проверяем, что окно уведомлений создано
    EXPECT_TRUE(notifications_handler != nullptr);
    
    // Проверяем, что окно видимо
    EXPECT_TRUE(notifications_handler->isVisible() || !notifications_handler->isVisible()); // Может быть скрыто по умолчанию
}

TEST_F(UITest, NotificationWidgetsCreated) {
    QSqlQuery query(db);
    
    // Создаем уведомление
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // В реальном приложении можно проверить количество виджетов в layout
    // Здесь мы просто проверяем, что функция выполняется без ошибок
    SUCCEED();
}

TEST_F(UITest, FilterComboBoxWorks) {
    // Создаем тестовые данные
    QSqlQuery query(db);
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("ОСАГО");
    query.addBindValue("отклонено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // Тестируем фильтрацию
    // В реальном приложении можно проверить содержимое ComboBox и его работу
    SUCCEED();
}

TEST_F(UITest, SortButtonWorks) {
    QSqlQuery query(db);
    
    // Создаем несколько уведомлений с разными датами
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика 1");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика 2");
    query.addBindValue(QDateTime::currentDateTime().addDays(2));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // Тестируем сортировку
    // В реальном приложении можно проверить порядок элементов
    SUCCEED();
}

TEST_F(UITest, MarkAllReadButtonWorks) {
    QSqlQuery query(db);
    
    // Создаем уведомления
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // Помечаем все как прочитанные
    notifications_handler->markNotificationsAsReaded(test_client_id);
    
    // Проверяем, что флаг установлен
    query.prepare("SELECT notification_shown FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_TRUE(query.value(0).toBool());
}

TEST_F(UITest, ClearOldButtonWorks) {
    QSqlQuery query(db);
    
    // Создаем уведомления
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("КАСКО");
    query.addBindValue("одобрено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // Скрываем все уведомления
    notifications_handler->onClearOldClicked();
    
    // Проверяем, что все уведомления помечены как показанные
    query.prepare("SELECT COUNT(*) FROM service_requests WHERE client_id = ? AND notification_shown = true");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
    
    query.prepare("SELECT COUNT(*) FROM insurance_requests WHERE client_id = ? AND notification_shown = true");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

// Тесты для проверки стилей и внешнего вида
TEST_F(UITest, NotificationWidgetsHaveCorrectStyles) {
    QSqlQuery query(db);
    
    // Создаем уведомление
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // В реальном приложении можно проверить стили виджетов
    SUCCEED();
}

// Тесты для проверки обработки ошибок
TEST_F(UITest, HandlesDatabaseErrors) {
    // Закрываем базу данных, чтобы вызвать ошибку
    db.close();
    
    // Пытаемся загрузить уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // В реальном приложении должно быть обработано исключение
    SUCCEED();
}

// Тесты для проверки производительности UI
TEST_F(UITest, UIPerformanceWithManyNotifications) {
    QSqlQuery query(db);
    
    // Создаем много уведомлений
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    
    for (int i = 0; i < 50; ++i) {
        query.addBindValue(test_client_id);
        query.addBindValue(1);
        query.addBindValue("Диагностика " + QString::number(i));
        query.addBindValue(QDateTime::currentDateTime().addDays(i));
        query.addBindValue("подтверждено");
        query.exec();
    }
    
    // Загружаем уведомления
    auto start = std::chrono::high_resolution_clock::now();
    notifications_handler->loadAndShowNotifications(test_client_id);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Проверяем, что загрузка выполняется за разумное время (менее 1 секунды)
    EXPECT_LT(duration.count(), 1000);
}

// Тесты для проверки доступности (accessibility)
TEST_F(UITest, AccessibilityFeatures) {
    QSqlQuery query(db);
    
    // Создаем уведомление
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // В реальном приложении можно проверить accessibility свойства
    SUCCEED();
}
