-- Человекочитаемые подписи столбцов для редактируемых таблиц админ-панели.
-- Представления v_* уже возвращают русские имена столбцов через AS.
CREATE TABLE sys_column_labels (
    table_name  TEXT NOT NULL,
    column_name TEXT NOT NULL,
    label       TEXT NOT NULL,
    PRIMARY KEY (table_name, column_name)
);

INSERT INTO sys_column_labels (table_name, column_name, label) VALUES
    ('purchases', 'id',               '№'),
    ('purchases', 'car_id',           'Автомобиль (id)'),
    ('purchases', 'client_id',        'Клиент (id)'),
    ('purchases', 'purchase_date',    'Дата покупки'),
    ('purchases', 'payment_type',     'Способ оплаты'),
    ('purchases', 'loan_amount',      'Сумма кредита'),
    ('purchases', 'loan_term_months', 'Срок кредита, мес.'),
    ('purchases', 'insurance_type',   'Страховка'),

    ('cars', 'id',                 '№'),
    ('cars', 'name',               'Модель'),
    ('cars', 'color',              'Цвет'),
    ('cars', 'price',              'Цена, ₽'),
    ('cars', 'description',        'Описание'),
    ('cars', 'image_url',          'Изображение'),
    ('cars', 'type_id',            'Тип кузова (id)'),
    ('cars', 'available_for_rent', 'Доступен в аренду'),
    ('cars', 'trim',               'Комплектация'),
    ('cars', 'stock_qty',          'На складе'),

    ('car_types', 'id',   '№'),
    ('car_types', 'name', 'Название'),

    ('clients', 'id',         '№'),
    ('clients', 'first_name', 'Имя'),
    ('clients', 'last_name',  'Фамилия'),
    ('clients', 'phone',      'Телефон'),
    ('clients', 'email',      'Email'),
    ('clients', 'password',   'Хеш пароля'),

    ('admins', 'id',       '№'),
    ('admins', 'username', 'Логин'),
    ('admins', 'password', 'Хеш пароля');
