SELECT value
  FROM sys_strings
 WHERE category = :category
   AND key = :key;
