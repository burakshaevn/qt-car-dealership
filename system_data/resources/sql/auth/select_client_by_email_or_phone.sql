SELECT id
FROM clients
WHERE email = :email OR phone = :phone
LIMIT 1;
