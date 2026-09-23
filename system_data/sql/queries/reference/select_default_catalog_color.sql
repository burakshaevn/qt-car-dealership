-- Предпочтительный цвет берётся из настроек; если его нет в каталоге —
-- первый по алфавиту.
SELECT color
  FROM cars
 WHERE color IS NOT NULL AND color <> ''
 ORDER BY color = (SELECT value FROM sys_strings WHERE category = 'defaults' AND key = 'catalog_color') DESC,
          color
 LIMIT 1;
