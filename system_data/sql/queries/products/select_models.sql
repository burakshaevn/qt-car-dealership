SELECT MIN(id) AS id, name
  FROM cars
 GROUP BY name
 ORDER BY name;
