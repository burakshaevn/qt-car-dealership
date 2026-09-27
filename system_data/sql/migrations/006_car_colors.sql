-- Образцы цвета кузова: подпись цвета в каталоге → цвет плашки в интерфейсе.
CREATE TABLE car_colors (
    name TEXT PRIMARY KEY,
    hex  TEXT NOT NULL CHECK (hex GLOB '#[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]')
);

INSERT INTO car_colors (name, hex) VALUES
    ('Белый',              '#F1F0EC'),
    ('Бриллиант',          '#D8DBDF'),
    ('Голубой',            '#7596B4'),
    ('Жёлтый',             '#E2B53A'),
    ('Зелёный',            '#3E5A45'),
    ('Золотой магнолитет', '#B5A07C'),
    ('Красный',            '#9C1C23'),
    ('Серый',              '#7A7D80'),
    ('Синий',              '#1F3A63'),
    ('Чёрный',             '#141414');
