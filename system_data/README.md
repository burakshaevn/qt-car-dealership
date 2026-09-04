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