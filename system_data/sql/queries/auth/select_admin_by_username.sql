SELECT id, username, password
  FROM admins
 WHERE username = :username
 LIMIT 1;
