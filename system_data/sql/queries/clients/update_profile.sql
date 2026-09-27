-- :password = NULL оставляет текущий пароль без изменений.
UPDATE clients
   SET first_name = :first_name,
       last_name  = :last_name,
       email      = :email,
       phone      = :phone,
       password   = COALESCE(:password, password)
 WHERE id = :id;
