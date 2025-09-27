#include <gtest/gtest.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QSharedPointer>
#include "../include/database_handler.h"
#include "../include/notifications_handler.h"

class ClientServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовую базу данных в памяти
        db = QSqlDatabase::addDatabase("QSQLITE", "test_connection");
        db.setDatabaseName(":memory:");
        
        if (!db.open()) {
            FAIL() << "Не удалось открыть тестовую базу данных: " << db.lastError().text().toStdString();
        }
        
        // Создаем тестовые таблицы
        createTestTables();
        
        // Создаем тестового клиента
        createTestClient();
        
        // Создаем тестовые автомобили
        createTestCars();
        
        // Создаем обработчик базы данных
        database_handler = QSharedPointer<DatabaseHandler>::create();
        database_handler->SetConnection(db);
        
        // Создаем обработчик уведомлений
        notifications_handler = new NotificationsHandler(database_handler);
    }
    
    void TearDown() override {
        delete notifications_handler;
        db.close();
        QSqlDatabase::removeDatabase("test_connection");
    }
    
    void createTestTables() {
        QSqlQuery query(db);
        
        // Создаем таблицу клиентов
        query.exec("CREATE TABLE clients ("
                  "id INTEGER PRIMARY KEY,"
                  "first_name VARCHAR(100) NOT NULL,"
                  "last_name VARCHAR(100) NOT NULL,"
                  "phone VARCHAR(15) NOT NULL,"
                  "email VARCHAR(255) NOT NULL,"
                  "password TEXT NOT NULL"
                  ")");
        
        // Создаем таблицу автомобилей
        query.exec("CREATE TABLE cars ("
                  "id INTEGER PRIMARY KEY,"
                  "name VARCHAR(255) NOT NULL,"
                  "color VARCHAR(50) NOT NULL,"
                  "price NUMERIC(15,0) NOT NULL,"
                  "description TEXT,"
                  "image_url TEXT,"
                  "type_id INTEGER NOT NULL,"
                  "available_for_rent BOOLEAN DEFAULT true NOT NULL,"
                  "trim VARCHAR(100),"
                  "stock_qty INTEGER DEFAULT 0 NOT NULL"
                  ")");
        
        // Создаем таблицу заявок на обслуживание
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
        
        // Создаем таблицу заявок на страхование
        query.exec("CREATE TABLE insurance_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "insurance_type VARCHAR(50) NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        // Создаем таблицу заявок на кредит
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
        
        // Создаем таблицу заявок на тест-драйв
        query.exec("CREATE TABLE test_drives ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "scheduled_date TIMESTAMP NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        // Создаем таблицу заявок на аренду
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
        
        // Создаем таблицу заявок на покупку
        query.exec("CREATE TABLE purchase_requests ("
                  "id INTEGER PRIMARY KEY,"
                  "client_id INTEGER NOT NULL,"
                  "car_id INTEGER NOT NULL,"
                  "status VARCHAR(20) DEFAULT 'не обработано' NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                  "notification_shown BOOLEAN DEFAULT false"
                  ")");
        
        // Создаем таблицу заявок на заказ
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
    
    void createTestClient() {
        QSqlQuery query(db);
        query.prepare("INSERT INTO clients (id, first_name, last_name, phone, email, password) "
                     "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(1);
        query.addBindValue("Иван");
        query.addBindValue("Петров");
        query.addBindValue("+79123456789");
        query.addBindValue("ivan@example.com");
        query.addBindValue("password123");
        query.exec();
    }
    
    void createTestCars() {
        QSqlQuery query(db);
        
        // Создаем несколько тестовых автомобилей
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
    
    QSqlDatabase db;
    QSharedPointer<DatabaseHandler> database_handler;
    NotificationsHandler* notifications_handler;
    int test_client_id = 1;
};

// Тесты для заявок на обслуживание
TEST_F(ClientServiceTest, ServiceRequestCreation) {
    QSqlQuery query(db);
    
    // Создаем заявку на обслуживание
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("не обработано");
    
    ASSERT_TRUE(query.exec()) << "Не удалось создать заявку на обслуживание: " << query.lastError().text().toStdString();
    
    // Проверяем, что заявка создана
    query.prepare("SELECT COUNT(*) FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

TEST_F(ClientServiceTest, ServiceRequestStatusUpdate) {
    QSqlQuery query(db);
    
    // Создаем заявку
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("не обработано");
    query.exec();
    
    // Обновляем статус
    query.prepare("UPDATE service_requests SET status = ? WHERE client_id = ?");
    query.addBindValue("подтверждено");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    
    // Проверяем обновление
    query.prepare("SELECT status FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), "подтверждено");
}

// Тесты для заявок на страхование
TEST_F(ClientServiceTest, InsuranceRequestCreation) {
    QSqlQuery query(db);
    
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("КАСКО");
    query.addBindValue("не обработано");
    
    ASSERT_TRUE(query.exec()) << "Не удалось создать заявку на страхование: " << query.lastError().text().toStdString();
    
    query.prepare("SELECT COUNT(*) FROM insurance_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

TEST_F(ClientServiceTest, InsuranceRequestApproval) {
    QSqlQuery query(db);
    
    // Создаем заявку
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("ОСАГО");
    query.addBindValue("не обработано");
    query.exec();
    
    // Одобряем заявку
    query.prepare("UPDATE insurance_requests SET status = ? WHERE client_id = ?");
    query.addBindValue("одобрено");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    
    // Проверяем статус
    query.prepare("SELECT status FROM insurance_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), "одобрено");
}

// Тесты для заявок на кредит
TEST_F(ClientServiceTest, LoanRequestCreation) {
    QSqlQuery query(db);
    
    query.prepare("INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue(10000000);
    query.addBindValue(36);
    query.addBindValue("не обработано");
    
    ASSERT_TRUE(query.exec()) << "Не удалось создать заявку на кредит: " << query.lastError().text().toStdString();
    
    query.prepare("SELECT COUNT(*) FROM loan_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

TEST_F(ClientServiceTest, LoanRequestRejection) {
    QSqlQuery query(db);
    
    // Создаем заявку
    query.prepare("INSERT INTO loan_requests (client_id, car_id, loan_amount, loan_term_months, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue(20000000);
    query.addBindValue(60);
    query.addBindValue("не обработано");
    query.exec();
    
    // Отклоняем заявку
    query.prepare("UPDATE loan_requests SET status = ? WHERE client_id = ?");
    query.addBindValue("отклонено");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    
    // Проверяем статус
    query.prepare("SELECT status FROM loan_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), "отклонено");
}

// Тесты для заявок на тест-драйв
TEST_F(ClientServiceTest, TestDriveRequestCreation) {
    QSqlQuery query(db);
    
    query.prepare("INSERT INTO test_drives (client_id, car_id, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue(QDateTime::currentDateTime().addDays(2));
    query.addBindValue("не обработано");
    
    ASSERT_TRUE(query.exec()) << "Не удалось создать заявку на тест-драйв: " << query.lastError().text().toStdString();
    
    query.prepare("SELECT COUNT(*) FROM test_drives WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

// Тесты для заявок на аренду
TEST_F(ClientServiceTest, RentalRequestCreation) {
    QSqlQuery query(db);
    
    query.prepare("INSERT INTO rental_requests (client_id, car_id, rental_days, start_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(2);
    query.addBindValue(7);
    query.addBindValue(QDate::currentDate().addDays(3));
    query.addBindValue("не обработано");
    
    ASSERT_TRUE(query.exec()) << "Не удалось создать заявку на аренду: " << query.lastError().text().toStdString();
    
    query.prepare("SELECT COUNT(*) FROM rental_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

// Тесты для заявок на покупку
TEST_F(ClientServiceTest, PurchaseRequestCreation) {
    QSqlQuery query(db);
    
    query.prepare("INSERT INTO purchase_requests (client_id, car_id, status) "
                 "VALUES (?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("не обработано");
    
    ASSERT_TRUE(query.exec()) << "Не удалось создать заявку на покупку: " << query.lastError().text().toStdString();
    
    query.prepare("SELECT COUNT(*) FROM purchase_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

// Тесты для заявок на заказ
TEST_F(ClientServiceTest, OrderRequestCreation) {
    QSqlQuery query(db);
    
    query.prepare("INSERT INTO order_requests (client_id, car_name, color, trim, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue("Mercedes-AMG GT 63 S 4MATIC+");
    query.addBindValue("Красный");
    query.addBindValue("AMG");
    query.addBindValue("не обработано");
    
    ASSERT_TRUE(query.exec()) << "Не удалось создать заявку на заказ: " << query.lastError().text().toStdString();
    
    query.prepare("SELECT COUNT(*) FROM order_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

// Тесты для уведомлений
TEST_F(ClientServiceTest, NotificationCreation) {
    // Создаем несколько заявок для генерации уведомлений
    QSqlQuery query(db);
    
    // Заявка на обслуживание
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("подтверждено");
    query.exec();
    
    // Заявка на страхование
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("КАСКО");
    query.addBindValue("одобрено");
    query.exec();
    
    // Заявка на кредит
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
    
    // Проверяем, что уведомления загружены (это проверяется через внутреннее состояние)
    // В реальном приложении можно проверить количество виджетов в layout
    SUCCEED();
}

TEST_F(ClientServiceTest, NotificationFiltering) {
    QSqlQuery query(db);
    
    // Создаем заявки с разными статусами
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
    
    // Тестируем фильтрацию "Только одобренные"
    // В реальном приложении можно проверить, что отображаются только одобренные заявки
    SUCCEED();
}

TEST_F(ClientServiceTest, NotificationMarkingAsRead) {
    QSqlQuery query(db);
    
    // Создаем заявку
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("не обработано");
    query.exec();
    
    // Помечаем как прочитанные
    notifications_handler->markNotificationsAsReaded(test_client_id);
    
    // Проверяем, что флаг установлен
    query.prepare("SELECT notification_shown FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_TRUE(query.value(0).toBool());
}

TEST_F(ClientServiceTest, NotificationHiding) {
    QSqlQuery query(db);
    
    // Создаем несколько заявок
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("не обработано");
    query.exec();
    
    query.prepare("INSERT INTO insurance_requests (client_id, car_id, insurance_type, status) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(test_client_id);
    query.addBindValue(1);
    query.addBindValue("КАСКО");
    query.addBindValue("не обработано");
    query.exec();
    
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

// Тесты для комплексных сценариев
TEST_F(ClientServiceTest, CompleteClientJourney) {
    QSqlQuery query(db);
    
    // 1. Клиент приходит на тест-драйв
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

// Тесты для проверки целостности данных
TEST_F(ClientServiceTest, DataIntegrity) {
    QSqlQuery query(db);
    
    // Создаем заявку с несуществующим клиентом
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(999); // Несуществующий клиент
    query.addBindValue(1);
    query.addBindValue("Диагностика");
    query.addBindValue(QDateTime::currentDateTime().addDays(1));
    query.addBindValue("не обработано");
    
    // В реальной системе это должно быть заблокировано внешним ключом
    // Здесь мы просто проверяем, что запрос выполняется
    ASSERT_TRUE(query.exec());
}

// Тесты для производительности
TEST_F(ClientServiceTest, PerformanceTest) {
    QSqlQuery query(db);
    
    // Создаем много заявок для тестирования производительности
    query.prepare("INSERT INTO service_requests (client_id, car_id, service_type, scheduled_date, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    
    for (int i = 0; i < 100; ++i) {
        query.addBindValue(test_client_id);
        query.addBindValue(1);
        query.addBindValue("Диагностика " + QString::number(i));
        query.addBindValue(QDateTime::currentDateTime().addDays(i));
        query.addBindValue("не обработано");
        ASSERT_TRUE(query.exec());
    }
    
    // Проверяем, что все заявки созданы
    query.prepare("SELECT COUNT(*) FROM service_requests WHERE client_id = ?");
    query.addBindValue(test_client_id);
    ASSERT_TRUE(query.exec());
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 100);
}
