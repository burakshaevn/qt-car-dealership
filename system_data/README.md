# SQLite: SQL ↔ DB

SQLite stores the entire database in a single file.

The database file can have different extensions:

- `.db`
- `.sqlite`
- `.sqlite3`

The extension itself does not matter. SQLite identifies the database by its contents, not by the filename extension.

## Requirements

Install SQLite CLI:

```bash
brew install sqlite
```

# DB → SQL
```bash
sqlite3 system_data.db .dump > system_data.sql
```

# SQL → DB
```bash
sqlite3 system_data.db < system_data.sql
```
# System data

This module owns database assets that are deployed with the application:

* `sysdb/system_data.sqlite` is the initial SQLite template.
* `resources/sql/**/*.sql` contains named, parameterised statements used by
  repositories and controllers.

`DatabaseHandler::LoadDefault()` opens the existing
`sysdb/system_data.sqlite` file. It does not create or copy a database. Set
`CAR_DEALERSHIP_DB` to open a different SQLite file (for example, in tests).

To add a statement, create an `.sql` file under `resources/sql`, register its
path in the `SqlQueryId` mapping in `SystemData.cpp`, add a corresponding enum
value, and execute
it through `DatabaseHandler::ExecuteNamedSelect()` or `ExecuteNamedQuery()`.
Bind values by name; do not interpolate user data into SQL.
