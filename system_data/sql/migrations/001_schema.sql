-- =============================================================================
-- 001 — базовая схема автосалона.
-- Все таблицы типизированы, имеют первичные/внешние ключи и ограничения.
-- Значения по умолчанию (статус заявки, дата создания, флаг уведомления)
-- задаются схемой, поэтому прикладной код их не передаёт.
-- =============================================================================

CREATE TABLE car_types (
    id   INTEGER PRIMARY KEY,
    name TEXT    NOT NULL UNIQUE
);

CREATE TABLE cars (
    id                 INTEGER PRIMARY KEY,
    name               TEXT    NOT NULL,
    color              TEXT    NOT NULL,
    price              INTEGER NOT NULL CHECK (price >= 0),
    description        TEXT,
    image_url          TEXT,
    type_id            INTEGER NOT NULL REFERENCES car_types (id) ON UPDATE CASCADE ON DELETE RESTRICT,
    available_for_rent INTEGER NOT NULL DEFAULT 1 CHECK (available_for_rent IN (0, 1)),
    trim               TEXT,
    stock_qty          INTEGER NOT NULL DEFAULT 0 CHECK (stock_qty >= 0)
);

CREATE INDEX idx_cars_name  ON cars (name);
CREATE INDEX idx_cars_color ON cars (color);

CREATE TABLE admins (
    id       INTEGER PRIMARY KEY,
    username TEXT    NOT NULL UNIQUE,
    password TEXT    NOT NULL -- SHA-256 hex
);

CREATE TABLE clients (
    id         INTEGER PRIMARY KEY,
    first_name TEXT    NOT NULL,
    last_name  TEXT    NOT NULL,
    phone      TEXT    NOT NULL UNIQUE,
    email      TEXT    NOT NULL UNIQUE COLLATE NOCASE,
    password   TEXT    NOT NULL -- SHA-256 hex
);

-- ---------------------------------------------------------------------------
-- Заявки клиентов
-- ---------------------------------------------------------------------------

CREATE TABLE service_requests (
    id                 INTEGER PRIMARY KEY,
    client_id          INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    car_id             INTEGER NOT NULL REFERENCES cars (id)    ON DELETE CASCADE,
    service_type       TEXT    NOT NULL,
    scheduled_date     TEXT    NOT NULL,
    status             TEXT    NOT NULL DEFAULT 'не обработано',
    created_at         TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notification_shown INTEGER NOT NULL DEFAULT 0 CHECK (notification_shown IN (0, 1))
);

CREATE TABLE purchase_requests (
    id                 INTEGER PRIMARY KEY,
    client_id          INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    car_id             INTEGER NOT NULL REFERENCES cars (id)    ON DELETE CASCADE,
    status             TEXT    NOT NULL DEFAULT 'не обработано',
    created_at         TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notification_shown INTEGER NOT NULL DEFAULT 0 CHECK (notification_shown IN (0, 1))
);

CREATE TABLE order_requests (
    id                 INTEGER PRIMARY KEY,
    client_id          INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    car_name           TEXT    NOT NULL,
    color              TEXT    NOT NULL,
    trim               TEXT,
    status             TEXT    NOT NULL DEFAULT 'не обработано',
    created_at         TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notification_shown INTEGER NOT NULL DEFAULT 0 CHECK (notification_shown IN (0, 1))
);

CREATE TABLE insurance_requests (
    id                 INTEGER PRIMARY KEY,
    client_id          INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    car_id             INTEGER NOT NULL REFERENCES cars (id)    ON DELETE CASCADE,
    insurance_type     TEXT    NOT NULL,
    status             TEXT    NOT NULL DEFAULT 'не обработано',
    created_at         TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notification_shown INTEGER NOT NULL DEFAULT 0 CHECK (notification_shown IN (0, 1))
);

CREATE TABLE loan_requests (
    id                 INTEGER PRIMARY KEY,
    client_id          INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    car_id             INTEGER NOT NULL REFERENCES cars (id)    ON DELETE CASCADE,
    loan_amount        INTEGER NOT NULL CHECK (loan_amount > 0),
    loan_term_months   INTEGER NOT NULL CHECK (loan_term_months > 0),
    status             TEXT    NOT NULL DEFAULT 'не обработано',
    created_at         TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notification_shown INTEGER NOT NULL DEFAULT 0 CHECK (notification_shown IN (0, 1))
);

CREATE TABLE test_drives (
    id                 INTEGER PRIMARY KEY,
    client_id          INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    car_id             INTEGER NOT NULL REFERENCES cars (id)    ON DELETE CASCADE,
    scheduled_date     TEXT    NOT NULL,
    status             TEXT    NOT NULL DEFAULT 'не обработано',
    created_at         TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notification_shown INTEGER NOT NULL DEFAULT 0 CHECK (notification_shown IN (0, 1))
);

CREATE TABLE rental_requests (
    id                 INTEGER PRIMARY KEY,
    client_id          INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    car_id             INTEGER NOT NULL REFERENCES cars (id)    ON DELETE CASCADE,
    rental_days        INTEGER NOT NULL CHECK (rental_days > 0),
    start_date         TEXT    NOT NULL,
    status             TEXT    NOT NULL DEFAULT 'не обработано',
    created_at         TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notification_shown INTEGER NOT NULL DEFAULT 0 CHECK (notification_shown IN (0, 1))
);

CREATE TABLE purchases (
    id               INTEGER PRIMARY KEY,
    car_id           INTEGER NOT NULL REFERENCES cars (id)    ON DELETE RESTRICT,
    client_id        INTEGER NOT NULL REFERENCES clients (id) ON DELETE CASCADE,
    purchase_date    TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    payment_type     TEXT    NOT NULL DEFAULT 'наличные',
    loan_amount      INTEGER,
    loan_term_months INTEGER,
    insurance_type   TEXT
);

CREATE INDEX idx_purchases_client ON purchases (client_id);

-- ---------------------------------------------------------------------------
-- Шаблоны договоров
-- ---------------------------------------------------------------------------

CREATE TABLE contract_templates (
    id            INTEGER PRIMARY KEY,
    code          TEXT    NOT NULL,
    title         TEXT    NOT NULL,
    body_template TEXT    NOT NULL,
    version       INTEGER NOT NULL DEFAULT 1,
    is_active     INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1)),
    updated_at    TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE (code, version)
);

-- ---------------------------------------------------------------------------
-- Системные справочники (конфигурация приложения хранится в данных, а не в коде)
-- ---------------------------------------------------------------------------

CREATE TABLE sys_strings (
    category TEXT NOT NULL,
    key      TEXT NOT NULL,
    value    TEXT NOT NULL,
    PRIMARY KEY (category, key)
);

-- Варианты для выпадающих списков форм (сроки кредита, виды страховки и т.п.).
CREATE TABLE sys_options (
    category   TEXT    NOT NULL,
    value      TEXT    NOT NULL,
    label      TEXT    NOT NULL,
    sort_order INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (category, value)
);

-- Какие таблицы видит администратор и как с ними работать.
CREATE TABLE sys_admin_tables (
    table_name     TEXT    PRIMARY KEY,
    display_name   TEXT    NOT NULL,
    description    TEXT    NOT NULL DEFAULT '',
    view_name      TEXT,            -- представление для отображения (NULL — сама таблица)
    is_request     INTEGER NOT NULL DEFAULT 0 CHECK (is_request IN (0, 1)),
    approve_status TEXT,
    reject_status  TEXT,
    sort_order     INTEGER NOT NULL DEFAULT 0
);

-- ---------------------------------------------------------------------------
-- Бизнес-правила
-- ---------------------------------------------------------------------------

CREATE TRIGGER trg_purchase_request_approved
AFTER UPDATE OF status ON purchase_requests
WHEN NEW.status = 'одобрено' AND OLD.status <> 'одобрено'
BEGIN
    SELECT RAISE(ABORT, 'no_stock')
    WHERE COALESCE((SELECT stock_qty FROM cars WHERE id = NEW.car_id), 0) <= 0;

    UPDATE cars SET stock_qty = stock_qty - 1 WHERE id = NEW.car_id;

    INSERT INTO purchases (car_id, client_id) VALUES (NEW.car_id, NEW.client_id);
END;

CREATE TRIGGER trg_order_request_approved
AFTER UPDATE OF status ON order_requests
WHEN NEW.status = 'одобрено' AND OLD.status <> 'одобрено'
     AND EXISTS (SELECT 1 FROM cars WHERE name = NEW.car_name)
BEGIN
    INSERT INTO purchases (car_id, client_id)
    VALUES (
        COALESCE(
            (SELECT id FROM cars WHERE name = NEW.car_name AND color = NEW.color AND trim IS NEW.trim LIMIT 1),
            (SELECT id FROM cars WHERE name = NEW.car_name LIMIT 1)
        ),
        NEW.client_id
    );
END;
