-- :exclude_id позволяет исключить текущего клиента при редактировании профиля.
SELECT EXISTS (
    SELECT 1
      FROM clients
     WHERE (email = :email OR phone = :phone)
       AND id <> :exclude_id
) AS found;
