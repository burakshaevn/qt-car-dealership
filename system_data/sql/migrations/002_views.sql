-- =============================================================================
-- 002 — представления.
-- v_requests          — единая лента всех заявок клиентов (уведомления, смена статуса).
-- v_admin_*           — табличные представления для панели администратора.
-- =============================================================================

CREATE VIEW v_requests AS
    SELECT 'service_requests' AS source_table, 'service' AS request_type,
           id, client_id, car_id, status, created_at, notification_shown,
           service_type AS additional_info, scheduled_date AS date_info
      FROM service_requests
    UNION ALL
    SELECT 'insurance_requests', 'insurance',
           id, client_id, car_id, status, created_at, notification_shown,
           insurance_type, created_at
      FROM insurance_requests
    UNION ALL
    SELECT 'loan_requests', 'loan',
           id, client_id, car_id, status, created_at, notification_shown,
           CAST(loan_amount AS TEXT), created_at
      FROM loan_requests
    UNION ALL
    SELECT 'test_drives', 'test_drive',
           id, client_id, car_id, status, created_at, notification_shown,
           '', scheduled_date
      FROM test_drives
    UNION ALL
    SELECT 'rental_requests', 'rental',
           id, client_id, car_id, status, created_at, notification_shown,
           CAST(rental_days AS TEXT), start_date
      FROM rental_requests
    UNION ALL
    SELECT 'purchase_requests', 'purchase',
           id, client_id, car_id, status, created_at, notification_shown,
           '', created_at
      FROM purchase_requests
    UNION ALL
    SELECT 'order_requests', 'order',
           id, client_id,
           COALESCE((SELECT c.id FROM cars c WHERE c.name = o.car_name AND c.color = o.color LIMIT 1),
                    (SELECT c.id FROM cars c WHERE c.name = o.car_name LIMIT 1)),
           status, created_at, notification_shown,
           car_name, created_at
      FROM order_requests o;

-- Обновление через v_requests маршрутизируется в исходную таблицу, поэтому
-- прикладному коду не нужно знать, в какой таблице хранится заявка.
CREATE TRIGGER trg_v_requests_update
INSTEAD OF UPDATE ON v_requests
BEGIN
    UPDATE service_requests   SET status = NEW.status, notification_shown = NEW.notification_shown
     WHERE OLD.source_table = 'service_requests'   AND id = OLD.id;
    UPDATE insurance_requests SET status = NEW.status, notification_shown = NEW.notification_shown
     WHERE OLD.source_table = 'insurance_requests' AND id = OLD.id;
    UPDATE loan_requests      SET status = NEW.status, notification_shown = NEW.notification_shown
     WHERE OLD.source_table = 'loan_requests'      AND id = OLD.id;
    UPDATE test_drives        SET status = NEW.status, notification_shown = NEW.notification_shown
     WHERE OLD.source_table = 'test_drives'        AND id = OLD.id;
    UPDATE rental_requests    SET status = NEW.status, notification_shown = NEW.notification_shown
     WHERE OLD.source_table = 'rental_requests'    AND id = OLD.id;
    UPDATE purchase_requests  SET status = NEW.status, notification_shown = NEW.notification_shown
     WHERE OLD.source_table = 'purchase_requests'  AND id = OLD.id;
    UPDATE order_requests     SET status = NEW.status, notification_shown = NEW.notification_shown
     WHERE OLD.source_table = 'order_requests'     AND id = OLD.id;
END;

-- ---------------------------------------------------------------------------
-- Представления панели администратора
-- ---------------------------------------------------------------------------

CREATE VIEW v_admin_service_requests AS
SELECT sr.id                                  AS "№",
       c.first_name || ' ' || c.last_name     AS "Клиент",
       c.phone                                AS "Телефон",
       cars.name                              AS "Автомобиль",
       sr.service_type                        AS "Тип услуги",
       sr.status                              AS "Статус",
       sr.created_at                          AS "Дата создания",
       sr.scheduled_date                      AS "Запланированная дата"
  FROM service_requests sr
  LEFT JOIN clients c ON c.id = sr.client_id
  LEFT JOIN cars      ON cars.id = sr.car_id;

CREATE VIEW v_admin_purchases AS
SELECT p.id                                   AS "№",
       cl.first_name || ' ' || cl.last_name   AS "Клиент",
       c.name || ' (' || c.color || ')'       AS "Автомобиль",
       p.payment_type                         AS "Тип оплаты",
       COALESCE(p.loan_amount, 0)             AS "Сумма кредита",
       COALESCE(p.loan_term_months, 0)        AS "Срок кредита (мес)",
       COALESCE(p.insurance_type, '—')        AS "Тип страховки",
       p.purchase_date                        AS "Дата продажи",
       c.price                                AS "Итого"
  FROM purchases p
  JOIN clients cl ON cl.id = p.client_id
  JOIN cars c     ON c.id = p.car_id;

CREATE VIEW v_admin_test_drives AS
SELECT td.id                                  AS "№",
       c.first_name || ' ' || c.last_name     AS "Клиент",
       c.phone                                AS "Телефон",
       cars.name || ' (' || cars.color || ')' AS "Автомобиль",
       td.scheduled_date                      AS "Дата тест-драйва",
       td.status                              AS "Статус",
       td.created_at                          AS "Дата создания"
  FROM test_drives td
  LEFT JOIN clients c ON c.id = td.client_id
  LEFT JOIN cars      ON cars.id = td.car_id;

CREATE VIEW v_admin_insurance_requests AS
SELECT ir.id                                  AS "№",
       c.first_name || ' ' || c.last_name     AS "Клиент",
       c.phone                                AS "Телефон",
       cars.name                              AS "Автомобиль",
       ir.insurance_type                      AS "Тип страховки",
       ir.status                              AS "Статус",
       ir.created_at                          AS "Дата создания"
  FROM insurance_requests ir
  LEFT JOIN clients c ON c.id = ir.client_id
  LEFT JOIN cars      ON cars.id = ir.car_id;

CREATE VIEW v_admin_loan_requests AS
SELECT lr.id                                  AS "№",
       c.first_name || ' ' || c.last_name     AS "Клиент",
       c.phone                                AS "Телефон",
       cars.name                              AS "Автомобиль",
       cars.price                             AS "Цена автомобиля",
       lr.loan_amount                         AS "Сумма кредита",
       lr.loan_term_months                    AS "Срок (месяцев)",
       lr.status                              AS "Статус",
       lr.created_at                          AS "Дата создания"
  FROM loan_requests lr
  LEFT JOIN clients c ON c.id = lr.client_id
  LEFT JOIN cars      ON cars.id = lr.car_id;

CREATE VIEW v_admin_purchase_requests AS
SELECT pr.id                                  AS "№",
       c.first_name || ' ' || c.last_name     AS "Клиент",
       c.phone                                AS "Телефон",
       cars.name || ' (' || cars.color || ')' AS "Автомобиль",
       cars.price                             AS "Цена",
       pr.status                              AS "Статус",
       pr.created_at                          AS "Дата создания"
  FROM purchase_requests pr
  LEFT JOIN clients c ON c.id = pr.client_id
  LEFT JOIN cars      ON cars.id = pr.car_id;

CREATE VIEW v_admin_order_requests AS
SELECT o.id                                   AS "№",
       c.first_name || ' ' || c.last_name     AS "Клиент",
       c.phone                                AS "Телефон",
       o.car_name                             AS "Автомобиль",
       o.color                                AS "Цвет",
       o.trim                                 AS "Комплектация",
       o.status                               AS "Статус",
       o.created_at                           AS "Дата создания"
  FROM order_requests o
  LEFT JOIN clients c ON c.id = o.client_id;

CREATE VIEW v_admin_rental_requests AS
SELECT rr.id                                  AS "№",
       c.first_name || ' ' || c.last_name     AS "Клиент",
       c.phone                                AS "Телефон",
       cars.name || ' (' || cars.color || ')' AS "Автомобиль",
       rr.rental_days                         AS "Дней аренды",
       rr.start_date                          AS "Дата начала",
       rr.status                              AS "Статус",
       rr.created_at                          AS "Дата создания"
  FROM rental_requests rr
  LEFT JOIN clients c ON c.id = rr.client_id
  LEFT JOIN cars      ON cars.id = rr.car_id;
