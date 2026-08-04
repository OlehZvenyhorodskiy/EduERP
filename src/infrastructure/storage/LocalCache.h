#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

namespace eduerp::infra {

/**
 * @brief Local SQLite cache for offline-first synchronization.
 *        Mirrors server data locally and tracks sync state.
 */
class LocalCache {
private:
    std::unique_ptr<SQLite::Database> m_db;

public:
    explicit LocalCache(const std::string& dbPath = "eduerp_cache.db") {
        m_db = std::make_unique<SQLite::Database>(
            dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        spdlog::info("LocalCache: Opened SQLite database at '{}'", dbPath);
    }

    /**
     * @brief Run the local schema migrations.
     */
    void migrate() {
        m_db->exec(R"SQL(
            CREATE TABLE IF NOT EXISTS sync_state (
                entity_type TEXT NOT NULL,
                entity_id   INTEGER NOT NULL,
                last_synced TEXT NOT NULL,
                is_dirty    INTEGER DEFAULT 0,
                PRIMARY KEY (entity_type, entity_id)
            );

            CREATE TABLE IF NOT EXISTS cached_users (
                id           INTEGER PRIMARY KEY,
                school_id    INTEGER NOT NULL,
                email        TEXT NOT NULL,
                display_name TEXT,
                role         TEXT,
                avatar_url   TEXT,
                cached_at    TEXT DEFAULT (datetime('now'))
            );

            CREATE TABLE IF NOT EXISTS cached_companies (
                id                INTEGER PRIMARY KEY,
                name              TEXT NOT NULL,
                industry_template TEXT,
                status            TEXT DEFAULT 'active',
                initial_budget    REAL DEFAULT 100000.0,
                cached_at         TEXT DEFAULT (datetime('now'))
            );

            CREATE TABLE IF NOT EXISTS cached_messages (
                id           INTEGER PRIMARY KEY,
                sender_id    INTEGER NOT NULL,
                recipient_id INTEGER,
                team_id      INTEGER,
                content      TEXT NOT NULL,
                sent_at      TEXT NOT NULL,
                is_read      INTEGER DEFAULT 0,
                cached_at    TEXT DEFAULT (datetime('now'))
            );

            CREATE TABLE IF NOT EXISTS pending_actions (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                action     TEXT NOT NULL,
                payload    TEXT NOT NULL,
                created_at TEXT DEFAULT (datetime('now')),
                synced_at  TEXT
            );
        )SQL");
        spdlog::info("LocalCache: Migrations complete");
    }

    /**
     * @brief Queue an action to be synced to the server when online.
     */
    void queueAction(const std::string& action, const std::string& payload) {
        SQLite::Statement stmt(*m_db, "INSERT INTO pending_actions (action, payload) VALUES (?, ?)");
        stmt.bind(1, action);
        stmt.bind(2, payload);
        stmt.exec();
    }

    /**
     * @brief Get count of pending (unsynced) actions.
     */
    int pendingActionCount() {
        SQLite::Statement stmt(*m_db, "SELECT COUNT(*) FROM pending_actions WHERE synced_at IS NULL");
        stmt.executeStep();
        return stmt.getColumn(0).getInt();
    }

    SQLite::Database& db() { return *m_db; }
};

} // namespace eduerp::infra