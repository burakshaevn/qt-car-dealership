-- Таблицы, которые ссылаются на :table_name через внешний ключ.
SELECT m.name AS table_name,
       fk."from" AS column_name
  FROM sqlite_master m,
       pragma_foreign_key_list(m.name) fk
 WHERE m.type = 'table'
   AND fk."table" = :table_name
 ORDER BY m.name;
