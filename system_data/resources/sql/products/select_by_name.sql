SELECT id, name, color, price, description, image_url, type_id, trim, stock_qty
FROM cars
WHERE name = :name
ORDER BY id ASC;
