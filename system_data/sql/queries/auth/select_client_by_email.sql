SELECT id, first_name, last_name, email, password
  FROM clients
 WHERE email = :email
 LIMIT 1;
