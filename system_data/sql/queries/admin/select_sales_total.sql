SELECT COALESCE(SUM(c.price), 0) AS total
  FROM purchases p
  JOIN cars c ON c.id = p.car_id;
