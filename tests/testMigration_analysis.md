# Migration Analysis: `DatabaseManager::migrate()`

**File**: `src/database/databasemanager.cpp` (lines 105–242)  
**Date**: 2026-09-19  
**Status**: Functional but with several gaps vs. the documented spec and robustness issues.

---

## 1. How It Works (Step-by-Step)

### Precondition
- Requires `m_databaseReady == true`. If the database connection isn't set up, returns `false` immediately.

### Phase 1 — Read or Bootstrap `meta` Table
1. Checks if the `meta` table exists via `m_database.tables()`.
2. **If `meta` exists**: executes `SELECT version FROM meta;` and reads the single row.  
   - No row → logs warning, returns `false`.  
   - Row found → `databaseVersion = q.value(0).toInt()`.
3. **If `meta` does NOT exist**:
   - Creates `meta (version INTEGER, updated_at TEXT)`.
   - Infers whether the database is "pre-existing" by checking for the `admins` table.
   - Seeds `meta` with `version = 1` if `admins` exists, else `version = 0`.
   - Sets `databaseVersion` to that initial version.

### Phase 2 — Discover Available Schema Files
- Iterates `v1.sql`, `v2.sql`, … using `QFile::exists(":/schema/schema/vN.sql")`.
- Builds a `QMap<int, QString>` mapping version numbers to resource paths.
- `lastVersion = version - 1` (the highest discovered version).

### Phase 3 — Up-to-Date Check
- If `databaseVersion >= lastVersion`, returns `true` immediately (no migration needed).

### Phase 4 — Gap Validation
- For every `v` from `databaseVersion + 1` to `lastVersion`:
  - Verifies `versions.contains(v)`.
  - Missing file → logs warning, returns `false`.

### Phase 5 — Incremental Migration (in a transaction)
- Starts a SQLite transaction.
- While `databaseVersion < lastVersion`:
  1. Opens `v{nextVersion}.sql` from the resource map.
  2. Reads the entire file into a string.
  3. Splits on the literal delimiter `"---- SEP"` (Qt::SkipEmptyParts).
  4. For each non-empty trimmed segment, executes it via `QSqlQuery::exec()`.
  5. On any failure: logs the error (including source version, target version, and segment index), rolls back, returns `false`.
  6. After all segments succeed: updates `meta.version` to `nextVersion` and `updated_at` to `CURRENT_TIMESTAMP`.
  7. Increments `databaseVersion`.
- Commits the transaction.
- Sets `m_isFirstRun = false`.
- Returns `true`.

---

## 2. Edge Cases Handled

| Edge Case | Behavior |
|---|---|
| Fresh DB (no `meta`, no `admins`) | `meta` created, version seeded at 0, migrates from v1 up. |
| Pre-existing DB (`meta` missing, `admins` present) | `meta` created, version seeded at 1 — **assumes v1.sql already applied** (see gaps). |
| `meta` table exists but empty (no rows) | Returns `false` with warning. |
| DB version already at or beyond last schema file | Returns `true` without migrating. |
| Gap in schema file sequence (e.g., v1 + v3 but no v2) | Detected during gap check; returns `false`. |
| Schema file cannot be opened | Rolls back transaction, returns `false`. |
| Individual SQL statement fails mid-migration | Rolls back entire transaction, returns `false` with context (from/to version, segment index). |
| Transaction commit fails | Returns `false`. |

---

## 3. Gaps vs. the Spec and Design Issues

### 3.1 The "Pre-Existing DB" Heuristic Is Unreliable
When `meta` is missing but `admins` exists, the code seeds version at **1** and skips v1.sql. This is an implicit assumption that:
- The `admins` table was created by v1.sql specifically, and
- No other v1.sql changes (indexes, other tables) are missing.

If a database was created by a different process, or if v1.sql contains more than just the `admins` table, those parts will never be applied. There is no validation that the schema actually matches what v1.sql would have produced.

**Impact**: Moderate. In practice, the current v1.sql creates many tables beyond `admins`, so a DB missing `meta` but having `admins` would be partially migrated — potentially causing FK violations or missing tables later.

### 3.2 No Handling of `databaseVersion > lastVersion`
The code only checks `databaseVersion >= lastVersion` for the "up-to-date" short-circuit. If `databaseVersion` is **greater** than `lastVersion` (e.g., the DB was migrated with a newer schema file that was since removed, or the `meta` version was manually altered), the gap validation loop `for (v = databaseVersion+1; v <= lastVersion; ++v)` simply doesn't execute (since `databaseVersion+1 > lastVersion`), and migration proceeds with an empty while loop — returning `true` without error.

This means a DB claiming to be at version 5 when only v1.sql exists will be treated as "up-to-date" without complaint. This could mask tampering or configuration errors.

**Impact**: Low in normal operation, but a silent acceptance of inconsistent state.

### 3.3 `updated_at` Format Inconsistency
- When seeding a new `meta` row (line 144), the `updated_at` value is set via SQLite's `CURRENT_TIMESTAMP` (a literal SQL expression evaluated at insert time).
- When updating version during migration (line 222–224), the same approach is used.

Both use `CURRENT_TIMESTAMP`, so they're consistent with each other. However, `CURRENT_TIMESTAMP` in SQLite returns UTC in format `YYYY-MM-DD HH:MM:SS`. If any other code reads `updated_at` and expects a different format or timezone, it may break. This is not a bug in `migrate()` per se, but worth noting.

### 3.4 `QSqlQuery::exec()` with Multiple Statements
The schema files use `---- SEP` to delimit individual statements, and each segment is passed to `QSqlQuery::exec()` separately. This works because each segment is a single SQL statement (or a BEGIN…COMMIT block in v1.sql).

However, the v1.sql file itself wraps **all** DDL in a single `BEGIN TRANSACTION; … COMMIT;` block (lines 6 and 508). The `---- SEP` splitter breaks this outer transaction into individual segments, meaning each `---- SEP`-delimited chunk is executed in its own implicit transaction (SQLite autocommit mode), not in the outer transaction that `migrate()` started.

Wait — let me re-examine. The v1.sql file has:
```
BEGIN TRANSACTION;
...many CREATE TABLE statements...
COMMIT;
```
But each is separated by `---- SEP`. So the splitter produces segments like:
- Segment 0: `PRAGMA foreign_keys = ON;\nBEGIN TRANSACTION;`
- Segment 1: `CREATE TABLE roles (...)`
- ...
- Segment N: `COMMIT;`

Each segment is executed individually by `QSqlQuery::exec()`. The `BEGIN TRANSACTION` in segment 0 starts a transaction, but then segment 1's `CREATE TABLE` executes in that same transaction context (since SQLite stays in transaction mode until `COMMIT`). The `COMMIT` in the last segment commits it.

**BUT**: `migrate()` also starts its own transaction at line 181. So there's a **nested transaction** situation:
- Outer transaction: `m_database.transaction()` (line 181)
- Inner transaction: `BEGIN TRANSACTION` inside the schema file (line 6 of v1.sql)

SQLite does **not** support nested transactions. The `BEGIN TRANSACTION` inside the schema file will fail with `SQLITE_ERROR` ("cannot start a transaction within a transaction") — **unless** the outer transaction was started with a different mechanism.

Actually, in SQLite, `BEGIN TRANSACTION` when already in a transaction returns an error. But `QSqlDatabase::transaction()` in Qt calls `BEGIN TRANSACTION` under the hood. So the nested `BEGIN` in the SQL file should fail.

Wait — does it? Let me reconsider. Qt's `QSqlDatabase::transaction()` for SQLite calls `sqlite3_exec(db, "BEGIN TRANSACTION", ...)`. Then when the schema file's first segment `PRAGMA foreign_keys = ON; BEGIN TRANSACTION;` is executed, the `BEGIN TRANSACTION` would indeed fail because we're already in a transaction.

**This appears to be a real bug**: the v1.sql file contains `BEGIN TRANSACTION; … COMMIT;` but `migrate()` already wraps everything in its own transaction. The inner `BEGIN TRANSACTION` should cause an error.

However — the existing test `testDatabaseMigrate` passes (`initializeFromSetup(":memory:", ...)` → `migrate()`). So either:
1. SQLite is lenient in some cases, or
2. The `---- SEP` splitting produces segments where the `BEGIN TRANSACTION` is in the same segment as `PRAGMA foreign_keys = ON;`, and `QSqlQuery::exec()` handles multi-statement buffers differently.

Actually, for SQLite, `QSqlQuery::exec()` with a multi-statement string: SQLite's `sqlite3_exec()` executes all statements in the string sequentially. If the string contains `PRAGMA foreign_keys = ON; BEGIN TRANSACTION;`, both execute. The `BEGIN TRANSACTION` would fail if we're already in a transaction.

But the test passes. So maybe in practice the current v1.sql works because the `BEGIN TRANSACTION` is the first statement after the PRAGMA, and Qt's SQLite driver or SQLite itself handles this gracefully in some configurations. Or perhaps the test was written before the outer transaction was added.

This needs verification with an actual run, but it's a potential issue when v1.sql is restructured or when additional schema files are added that don't have their own `BEGIN/COMMIT`.

### 3.5 No Handling of Partially Applied Migrations
If a migration fails after some statements in a version's schema file have succeeded (but before the version is recorded in `meta`), the transaction is rolled back — so the DDL changes are undone. This is correct because SQLite DDL is transactional.

However, if the failure occurs **after** `meta.version` is updated for a version but **before** the next version's migration completes, the transaction rollback would also undo the `meta` update. So the DB returns to its pre-migration state. This is fine.

The real gap: there's no **"resume from failed version"** mechanism. If a migration fails, the caller gets `false` and must retry from scratch. For large schema files this could be slow, but for the current single-file setup it's acceptable.

### 3.6 `verifySchema()` Is a No-Op
`DatabaseManager::verifySchema()` (line 246–249) returns `true` unconditionally with a comment saying verification is skipped "because there is only 1 schema." If additional schema versions are added, this should be re-implemented to actually validate the schema state.

### 3.7 `m_isFirstRun` Semantics
`m_isFirstRun` is set to `false` at the end of `migrate()` regardless of whether any migration actually occurred (line 240). If `databaseVersion >= lastVersion` (short-circuit at line 165), `m_isFirstRun` is **not** set to `false`. This means:
- A DB that is already up-to-date when `migrate()` is called will still have `m_isFirstRun == true`.
- `initializeFromSetup()` calls `migrate()` and then doesn't check `m_isFirstRun` itself, but other code may rely on this flag.

This is a minor inconsistency but could cause unexpected behavior if `isFirstRun()` is queried after a successful `initializeFromSetup()` on an already-migrated DB.

### 3.8 Single Schema File Assumption
The current `database_resources.qrc` only registers `schema/v1.sql`. The migration loop is designed for multiple version files, but only one exists. Adding `v2.sql` requires:
1. Creating the file at `resources/schema/v2.sql`.
2. Registering it in `database_resources.qrc`.
3. Rebuilding the Qt resource.

If step 2 or 3 is missed, `QFile::exists(":/schema/schema/v2.sql")` returns `false`, the version map stops at 1, and `lastVersion = 1`. The migration would not apply v2 changes.

### 3.9 No Logging of Successful Migration Steps
The code logs failures in detail (version range, segment index, error text) but logs nothing when a migration step succeeds. For debugging migration issues in the field, it would be helpful to log each version transition.

---

## 4. Testing Observations

The existing test `testDatabaseMigrate` (in `tests/testDatabase.cpp`):
- Verifies `v1.sql` exists as a resource.
- Checks `isFirstRun()` is true before migration.
- Calls `initializeFromSetup(":memory:", "root", "000000")`.
- Verifies it returns `true` and `isFirstRun()` becomes `false`.

This tests the **happy path** for a fresh in-memory database. It does **not** test:
- Pre-existing database scenario (the `admins`-exists heuristic).
- Gap in schema files.
- Failed migration (e.g., corrupt SQL in a schema file).
- Database already at latest version (skip scenario).
- The `testDatabaseMigrateAlready` test uses a hardcoded path to `LAdmins.db` which is gitignored — this test may fail in a fresh clone.

---

## 5. Recommendations

1. **Fix the pre-existing DB heuristic**: Instead of inferring version from the presence of `admins`, either:
   - Always start at version 0 when `meta` is missing (and let v1.sql recreate `admins` — but this would fail if `admins` already exists), or
   - Query the actual schema state to determine which version's changes are already applied.

2. **Remove the inner `BEGIN TRANSACTION` / `COMMIT` from schema files** and rely solely on `migrate()`'s transaction. The schema files should contain only DDL statements separated by `---- SEP`, without their own transaction wrappers.

3. **Add an upper-bound check**: If `databaseVersion > lastVersion`, log a warning and either refuse to proceed or clamp to `lastVersion`.

4. **Implement `verifySchema()`** to actually validate that the schema matches the expected state for the current version.

5. **Log successful version transitions** for observability.

6. **Add tests** for the pre-existing DB path, failed migration, and gap detection.

---

*End of analysis.*
