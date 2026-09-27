-- Лучший вариант модели в выбранной комплектации:
-- сначала точное совпадение по цвету, затем — с наибольшим остатком.
SELECT id, color, stock_qty
  FROM cars
 WHERE name = :name
   AND trim = :trim
 ORDER BY (color = :color) DESC, stock_qty DESC
 LIMIT 1;
