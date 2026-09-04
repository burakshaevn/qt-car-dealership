SELECT id, password
FROM admins
WHERE username = :username
LIMIT 1;
