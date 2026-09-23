SELECT DISTINCT trim
  FROM cars
 WHERE name = :name
   AND trim IS NOT NULL
   AND trim <> ''
 ORDER BY trim;
