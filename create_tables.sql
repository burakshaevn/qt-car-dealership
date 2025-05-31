-- Создание таблицы clients, если она не существует
CREATE TABLE IF NOT EXISTS clients (
    id SERIAL PRIMARY KEY,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    phone VARCHAR(20),
    email VARCHAR(100)
);

-- Создание таблицы cars, если она не существует
CREATE TABLE IF NOT EXISTS cars (
    id SERIAL PRIMARY KEY,
    make VARCHAR(100) NOT NULL,
    model VARCHAR(100) NOT NULL,
    year INTEGER NOT NULL,
    price DECIMAL(10,2) NOT NULL,
    color VARCHAR(50),
    status VARCHAR(50) DEFAULT 'available'
);

-- Создание таблицы service_requests, если она не существует
CREATE TABLE IF NOT EXISTS service_requests (
    id SERIAL PRIMARY KEY,
    client_id INTEGER NOT NULL REFERENCES clients(id),
    car_id INTEGER NOT NULL REFERENCES cars(id),
    service_type VARCHAR(255) NOT NULL,
    status VARCHAR(50) DEFAULT 'не обработано',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    scheduled_date DATE,
    notification_shown BOOLEAN DEFAULT false
);

-- Создание таблицы insurance_requests, если она не существует
CREATE TABLE IF NOT EXISTS insurance_requests (
    id SERIAL PRIMARY KEY,
    client_id INTEGER NOT NULL REFERENCES clients(id),
    car_id INTEGER NOT NULL REFERENCES cars(id),
    insurance_type VARCHAR(255) NOT NULL,
    status VARCHAR(50) DEFAULT 'не обработано',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    notification_shown BOOLEAN DEFAULT false
);

-- Создание таблицы loan_requests, если она не существует
CREATE TABLE IF NOT EXISTS loan_requests (
    id SERIAL PRIMARY KEY,
    client_id INTEGER NOT NULL REFERENCES clients(id),
    car_id INTEGER NOT NULL REFERENCES cars(id),
    loan_amount DECIMAL(10,2) NOT NULL,
    status VARCHAR(50) DEFAULT 'не обработано',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    notification_shown BOOLEAN DEFAULT false
);

-- Создание таблицы sell_requests, если она не существует
CREATE TABLE IF NOT EXISTS sell_requests (
    id SERIAL PRIMARY KEY,
    client_id INTEGER NOT NULL REFERENCES clients(id),
    car_id INTEGER NOT NULL REFERENCES cars(id),
    status VARCHAR(50) DEFAULT 'не обработано',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    notification_shown BOOLEAN DEFAULT false
);

-- Добавление тестовых данных
INSERT INTO clients (first_name, last_name, phone, email) VALUES
    ('Иван', 'Иванов', '+7-900-123-4567', 'ivan@example.com'),
    ('Петр', 'Петров', '+7-900-234-5678', 'petr@example.com')
ON CONFLICT DO NOTHING;

INSERT INTO cars (make, model, year, price, color, status) VALUES
    ('Toyota', 'Camry', 2023, 2500000.00, 'Белый', 'available'),
    ('Honda', 'Civic', 2022, 2000000.00, 'Черный', 'available'),
    ('BMW', 'X5', 2023, 5500000.00, 'Серый', 'available')
ON CONFLICT DO NOTHING; 