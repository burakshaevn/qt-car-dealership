-- Данные для заполнения договора по заявке любого типа.
SELECT r.request_type,
       COALESCE(o.car_name, c.name)  AS car_name,
       COALESCE(o.color, c.color)    AS car_color,
       COALESCE(c.price, 0)          AS car_price,
       COALESCE(o.trim, c.trim)      AS trim,
       l.loan_term_months            AS loan_term_months
  FROM v_requests r
  LEFT JOIN cars c           ON c.id = r.car_id
  LEFT JOIN order_requests o ON r.source_table = 'order_requests' AND o.id = r.id
  LEFT JOIN loan_requests l  ON r.source_table = 'loan_requests'  AND l.id = r.id
 WHERE r.source_table = :source_table
   AND r.id = :id;
