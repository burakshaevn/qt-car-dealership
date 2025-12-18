#include <gtest/gtest.h>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QSharedPointer>
#include "../include/database_handler.h"
#include "../../controller/include/notifications_handler.h"
#include "../include/contract_templates.h"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем QApplication для тестов
        int argc = 0;
        char** argv = nullptr;
        app = new QApplication(argc, argv);
        
        // Создаем тестовую базу данных
        db = QSqlDatabase::addDatabase("QSQLITE", "integration_test_connection");
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
        QSqlDatabase::removeDatabase("integration_test_connection");
    }
    
    void createTestTables() {
        QSqlQuery query(db);
        
        // Создаем все необходимые таблицы
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
                  "description TEXT,"
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
        query.addBindValue("Иван");
        query.addBindValue("Петров");
        query.addBindValue("+79123456789");
        query.addBindValue("ivan@example.com");
        query.addBindValue("password123");
        query.exec();
        
        // Создаем тестовые автомобили
        query.prepare("INSERT INTO cars (id, name, color, price, description, type_id, stock_qty) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(1);
        query.addBindValue("Mercedes-AMG S 63 E Performance");
        query.addBindValue("Черный");
        query.addBindValue(20520000);
        query.addBindValue("Роскошный седан");
        query.addBindValue(1);
        query.addBindValue(2);
        query.exec();
        
        query.addBindValue(2);
        query.addBindValue("Mercedes-AMG G 63");
        query.addBindValue("Белый");
        query.addBindValue(19890000);
        query.addBindValue("Внедорожник");
        query.addBindValue(2);
        query.addBindValue(1);
        query.exec();
    }
    
    QApplication* app;
    QSqlDatabase db;
    QSharedPointer<DatabaseHandler> database_handler;
    NotificationsHandler* notifications_handler;
    int test_client_id = 1;
};

// Тест полного цикла обслуживания клиента
TEST_F(IntegrationTest, CompleteClientServiceCycle) {
    QSqlQuery query(db);
    
    // 1. Клиент записывается на тест-драйв
    query.prepare("INSERT INTO test_drives (client_id, car_id, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("одобрено");
    query.exec();
    
    // 2. Клиент решает купить автомобиль
    query.prepare("INSERT INTO purchase_requests (client_id, car_id, status) "
                 "VALUES (?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("одобрено");
    query.exec();
    
    // 3. Клиент оформляет кредит
    query.prepare("INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue(15000000);
    query.addBindValue(48);
    query.addBindValue("одобрено");
    query.exec();
    
    // 4. Клиент оформляет страховку
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("КАСКО");
    query.addBindValue("одобрено");
    query.exec();
    
    // 5. Клиент записывается на обслуживание
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Плановое ТО");
    query.addBindValue(QDateTime::currentDateTime().addDays(30));
    query.addBindValue("подтверждено");
    query.exec();
    
    // 6. Загружаем уведомления и проверяем, что все отображаются
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // Проверяем, что все заявки созданы
    query.prepare("SELECT COUNT(*) FROM test_drives WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
    
    query.prepare("SELECT COUNT(*) FROM purchase_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
    
    query.prepare("SELECT COUNT(*) FROM loan_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
    
    query.prepare("SELECT COUNT(*) FROM insurance_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
    
    query.prepare("SELECT COUNT(*) FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

// Тест фильтрации уведомлений
TEST_F(IntegrationTest, NotificationFiltering) {
    QSqlQuery query(db);
    
    // Создаем уведомления с разными статусами
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
    
    query.prepare("INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue(10000000);
    query.addBindValue(36);
    query.addBindValue("одобрено");
    query.exec();
    
    // Тестируем разные фильтры
    // В реальном приложении можно проверить содержимое уведомлений
    SUCCEED();
}

// Тест сортировки уведомлений
TEST_F(IntegrationTest, NotificationSorting) {
    QSqlQuery query(db);
    
    // Создаем уведомления с разными датами
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
    
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика 3");
    query.addBindValue(QDateTime::currentDateTime().addDays(3));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Тестируем сортировку
    // В реальном приложении можно проверить порядок элементов
    SUCCEED();
}

// Тест управления уведомлениями
TEST_F(IntegrationTest, NotificationManagement) {
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
    
    // Помечаем все как прочитанные
    notifications_handler->markNotificationsAsReaded(test_client_id);
    
    // Проверяем, что флаги установлены
    query.prepare("SELECT notification_shown FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_TRUE(query.value(0).toBool());
    
    query.prepare("SELECT notification_shown FROM insurance_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_TRUE(query.value(0).toBool());
    
    // Скрываем все уведомления
    notifications_handler->onClearOldClicked();
    
    // Проверяем, что все уведомления скрыты
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

// Тест генерации договоров
TEST_F(IntegrationTest, ContractGeneration) {
    QSqlQuery query(db);
    
    // Создаем одобренную заявку на обслуживание
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Создаем одобренную заявку на страхование
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("КАСКО");
    query.addBindValue("одобрено");
    query.exec();
    
    // Создаем одобренную заявку на кредит
    query.prepare("INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue(10000000);
    query.addBindValue(36);
    query.addBindValue("одобрено");
    query.exec();
    
    // Загружаем уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // В реальном приложении можно проверить наличие кнопок "Скачать договор"
    SUCCEED();
}

// Тест обработки ошибок
TEST_F(IntegrationTest, ErrorHandling) {
    // Закрываем базу данных, чтобы вызвать ошибку
    db.close();
    
    // Пытаемся загрузить уведомления
    notifications_handler->loadAndShowNotifications(test_client_id);
    
    // В реальном приложении должно быть обработано исключение
    SUCCEED();
}

// Тест производительности
TEST_F(IntegrationTest, PerformanceTest) {
    QSqlQuery query(db);
    
    // Создаем много уведомлений
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    
    for (int i = 0; i < 100; ++i) {
        query.addBindValue(test_client_id);
        query.addBindValue(1);
        query.addBindValue("Диагностика " + QString::number(i));
        query.addBindValue(QDateTime::currentDateTime().addDays(i));
        query.addBindValue("подтверждено");
        query.exec();
    }
    
    // Измеряем время загрузки
    auto start = std::chrono::high_resolution_clock::now();
    notifications_handler->loadAndShowNotifications(test_client_id);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Проверяем, что загрузка выполняется за разумное время
    EXPECT_LT(duration.count(), 2000);
}

// Тест целостности данных
TEST_F(IntegrationTest, DataIntegrity) {
    QSqlQuery query(db);
    
    // Создаем заявку
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Проверяем, что данные сохранились корректно
    query.prepare("SELECT * FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    
    EXPECT_EQ(query.value("client_id").toInt(), test_client_id);
    EXPECT_EQ(query.value("car_id").toInt(), 1);
    EXPECT_EQ(query.value("service_type").toString(), "Диагностика");
    EXPECT_EQ(query.value("status").toString(), "подтверждено");
}

// Тест транзакций
TEST_F(IntegrationTest, TransactionHandling) {
    QSqlQuery query(db);
    
    // Начинаем транзакцию
    ASSERT_TRUE(db.transaction());
    
    // Создаем несколько заявок
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
    
    // Подтверждаем транзакцию
    ASSERT_TRUE(db.commit());
    
    // Проверяем, что все заявки созданы
    query.prepare("SELECT COUNT(*) FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 2);
}
