SELECT c.name, c.color
  FROM purchases p
  JOIN cars c ON c.id = p.car_id
 WHERE p.client_id = :client_id
 ORDER BY p.purchase_date DESC;
