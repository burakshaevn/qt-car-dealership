SELECT c.id,
       c.name,
       c.color,
       c.price,
       c.description,
       c.image_url,
       c.type_id,
       c.trim,
       c.stock_qty,
       t.name AS type_name,
       COALESCE(cc.hex, '') AS color_hex
  FROM cars c
  LEFT JOIN car_types  t  ON t.id = c.type_id
  LEFT JOIN car_colors cc ON cc.name = c.color
 WHERE c.name = :name
 ORDER BY c.id;
