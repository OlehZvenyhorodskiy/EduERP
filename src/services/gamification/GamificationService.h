#pragma once

#include <QObject>
#include <QDate>
#include <QSettings>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <vector>
#include <spdlog/spdlog.h>

namespace eduerp::services {

/**
 * @brief Achievement/Badge definition.
 */
struct Achievement {
    QString id;
    QString titleKey;       // i18n key for title
    QString descriptionKey; // i18n key for description
    QString icon;
    QString category;       // streak, simulation, social, learning
    int requiredValue = 0;
    bool unlocked = false;
    QString unlockedDate;
};

/**
 * @brief Manages XP, levels, streaks, and achievements.
 *        Persists state via QSettings for offline-first support.
 *        Syncs with backend when connection is available.
 */
class GamificationService : public QObject {
    Q_OBJECT

public:
    explicit GamificationService(QObject* parent = nullptr) : QObject(parent) {
        loadState();
        initializeAchievements();
    }

    // ── XP & Levels ──

    int currentXP() const { return m_currentXP; }
    int currentLevel() const { return m_currentLevel; }
    int xpForNextLevel() const { return xpRequiredForLevel(m_currentLevel + 1); }
    int xpProgressInLevel() const { return m_currentXP - xpRequiredForLevel(m_currentLevel); }
    double levelProgress() const {
        int needed = xpForNextLevel() - xpRequiredForLevel(m_currentLevel);
        if (needed <= 0) return 1.0;
        return static_cast<double>(xpProgressInLevel()) / needed;
    }

    /**
     * @brief Award XP for an action. Auto-levels-up if threshold crossed.
     */
    void awardXP(int amount, const QString& reason) {
        m_currentXP += amount;
        spdlog::info("Gamification: +{} XP for '{}' (total: {})", amount, reason.toStdString(), m_currentXP);

        // Check for level up
        while (m_currentXP >= xpRequiredForLevel(m_currentLevel + 1)) {
            m_currentLevel++;
            spdlog::info("Gamification: LEVEL UP! Now level {}", m_currentLevel);
            emit leveledUp(m_currentLevel);
        }

        saveState();
        emit xpChanged();
    }

    // ── Streaks ──

    int currentStreak() const { return m_currentStreak; }
    int longestStreak() const { return m_longestStreak; }
    int totalLoginDays() const { return m_totalLoginDays; }

    /**
     * @brief Call this on app launch to register a daily login.
     */
    void registerDailyLogin() {
        QDate today = QDate::currentDate();

        if (m_lastLoginDate == today) {
            spdlog::debug("Gamification: Already logged in today");
            return; // Already counted today
        }

        QDate yesterday = today.addDays(-1);

        if (m_lastLoginDate == yesterday) {
            // Consecutive day — streak continues
            m_currentStreak++;
        } else if (m_lastLoginDate.isValid() && m_lastLoginDate != today) {
            // Streak broken
            spdlog::info("Gamification: Streak broken (last login: {})", m_lastLoginDate.toString("yyyy-MM-dd").toStdString());
            m_currentStreak = 1;
        } else {
            // First ever login
            m_currentStreak = 1;
        }

        m_lastLoginDate = today;
        m_totalLoginDays++;

        if (m_currentStreak > m_longestStreak) {
            m_longestStreak = m_currentStreak;
        }

        // XP rewards for streaks
        int streakXP = 10; // base daily login XP
        if (m_currentStreak >= 7) streakXP = 25;
        if (m_currentStreak >= 14) streakXP = 40;
        if (m_currentStreak >= 30) streakXP = 75;

        awardXP(streakXP, QString("Dag %1 loginstreak").arg(m_currentStreak));

        // Check streak achievements
        checkStreakAchievements();

        saveState();
        emit streakChanged();
    }

    // ── Achievements ──

    const std::vector<Achievement>& achievements() const { return m_achievements; }

    int unlockedCount() const {
        int count = 0;
        for (const auto& a : m_achievements) {
            if (a.unlocked) count++;
        }
        return count;
    }

    int totalCount() const { return static_cast<int>(m_achievements.size()); }

    /**
     * @brief Check and unlock an achievement by ID.
     */
    bool tryUnlockAchievement(const QString& id) {
        for (auto& a : m_achievements) {
            if (a.id == id && !a.unlocked) {
                a.unlocked = true;
                a.unlockedDate = QDate::currentDate().toString("yyyy-MM-dd");
                spdlog::info("Gamification: Achievement unlocked — {}", id.toStdString());
                emit achievementUnlocked(a.id, a.icon);
                saveState();
                return true;
            }
        }
        return false;
    }

    // ── Action triggers (called from controllers) ──

    void onCompanyCreated() {
        awardXP(50, "Bedrijf aangemaakt");
        tryUnlockAchievement("first_company");
    }

    void onFirstTransaction() {
        awardXP(20, "Eerste transactie");
        tryUnlockAchievement("first_transaction");
    }

    void onFirstHire() {
        awardXP(15, "Eerste werknemer");
        tryUnlockAchievement("first_hire");
    }

    void onCampaignLaunched() {
        awardXP(20, "Campagne gestart");
        tryUnlockAchievement("first_campaign");
    }

    void onProfitMilestone(double profit) {
        awardXP(30, "Winstmijlpaal bereikt");
        if (profit >= 1000) tryUnlockAchievement("profit_1k");
        if (profit >= 10000) tryUnlockAchievement("profit_10k");
        if (profit >= 100000) tryUnlockAchievement("profit_100k");
    }

    void onFriendAdded() {
        awardXP(10, "Vriend toegevoegd");
        tryUnlockAchievement("first_friend");
    }

    void onMessageSent() {
        awardXP(5, "Bericht verstuurd");
        tryUnlockAchievement("first_message");
    }

    void onSimulationDayCompleted(int totalDays) {
        awardXP(5, "Simulatiedag voltooid");
        if (totalDays >= 30) tryUnlockAchievement("sim_30_days");
        if (totalDays >= 100) tryUnlockAchievement("sim_100_days");
        if (totalDays >= 365) tryUnlockAchievement("sim_365_days");
    }

signals:
    void xpChanged();
    void streakChanged();
    void leveledUp(int newLevel);
    void achievementUnlocked(const QString& id, const QString& icon);

private:
    /**
     * @brief XP required to reach a given level.
     *        Follows a gentle curve: Level N needs N * 100 total XP.
     *        Level 1 = 0 XP, Level 2 = 100 XP, Level 5 = 400 XP, Level 10 = 900 XP.
     */
    static int xpRequiredForLevel(int level) {
        if (level <= 1) return 0;
        return (level - 1) * 100;
    }

    void checkStreakAchievements() {
        if (m_currentStreak >= 3) tryUnlockAchievement("streak_3");
        if (m_currentStreak >= 7) tryUnlockAchievement("streak_7");
        if (m_currentStreak >= 14) tryUnlockAchievement("streak_14");
        if (m_currentStreak >= 30) tryUnlockAchievement("streak_30");
        if (m_currentStreak >= 100) tryUnlockAchievement("streak_100");
    }

    void initializeAchievements() {
        m_achievements = {
            // Streak achievements
            {"streak_3",     "achievement.streak_3",     "achievement.streak_3_desc",     "🔥", "streak", 3},
            {"streak_7",     "achievement.streak_7",     "achievement.streak_7_desc",     "🔥", "streak", 7},
            {"streak_14",    "achievement.streak_14",    "achievement.streak_14_desc",    "🔥", "streak", 14},
            {"streak_30",    "achievement.streak_30",    "achievement.streak_30_desc",    "💎", "streak", 30},
            {"streak_100",   "achievement.streak_100",   "achievement.streak_100_desc",   "👑", "streak", 100},

            // Simulation achievements
            {"first_company",    "achievement.first_company",    "achievement.first_company_desc",    "🏢", "simulation", 1},
            {"first_transaction","achievement.first_transaction","achievement.first_transaction_desc","💰", "simulation", 1},
            {"first_hire",       "achievement.first_hire",       "achievement.first_hire_desc",       "👤", "simulation", 1},
            {"first_campaign",   "achievement.first_campaign",   "achievement.first_campaign_desc",   "📢", "simulation", 1},
            {"profit_1k",        "achievement.profit_1k",        "achievement.profit_1k_desc",        "💵", "simulation", 1000},
            {"profit_10k",       "achievement.profit_10k",       "achievement.profit_10k_desc",       "💶", "simulation", 10000},
            {"profit_100k",      "achievement.profit_100k",      "achievement.profit_100k_desc",      "🤑", "simulation", 100000},
            {"sim_30_days",      "achievement.sim_30_days",      "achievement.sim_30_days_desc",      "📅", "simulation", 30},
            {"sim_100_days",     "achievement.sim_100_days",     "achievement.sim_100_days_desc",     "📆", "simulation", 100},
            {"sim_365_days",     "achievement.sim_365_days",     "achievement.sim_365_days_desc",     "🗓️", "simulation", 365},

            // Social achievements
            {"first_friend",  "achievement.first_friend",  "achievement.first_friend_desc",  "🤝", "social", 1},
            {"first_message", "achievement.first_message", "achievement.first_message_desc", "💬", "social", 1},
        };

        // Restore unlocked state from storage
        QSettings settings("EduERP", "Gamification");
        for (auto& a : m_achievements) {
            a.unlocked = settings.value("achievement_" + a.id, false).toBool();
            a.unlockedDate = settings.value("achievement_date_" + a.id, "").toString();
        }
    }

    void saveState() {
        QSettings settings("EduERP", "Gamification");
        settings.setValue("current_xp", m_currentXP);
        settings.setValue("current_level", m_currentLevel);
        settings.setValue("current_streak", m_currentStreak);
        settings.setValue("longest_streak", m_longestStreak);
        settings.setValue("total_login_days", m_totalLoginDays);
        settings.setValue("last_login_date", m_lastLoginDate.toString("yyyy-MM-dd"));

        for (const auto& a : m_achievements) {
            settings.setValue("achievement_" + a.id, a.unlocked);
            settings.setValue("achievement_date_" + a.id, a.unlockedDate);
        }

        settings.sync();
    }

    void loadState() {
        QSettings settings("EduERP", "Gamification");
        m_currentXP = settings.value("current_xp", 0).toInt();
        m_currentLevel = settings.value("current_level", 1).toInt();
        m_currentStreak = settings.value("current_streak", 0).toInt();
        m_longestStreak = settings.value("longest_streak", 0).toInt();
        m_totalLoginDays = settings.value("total_login_days", 0).toInt();

        QString lastDate = settings.value("last_login_date", "").toString();
        if (!lastDate.isEmpty()) {
            m_lastLoginDate = QDate::fromString(lastDate, "yyyy-MM-dd");
        }
    }

    int m_currentXP = 0;
    int m_currentLevel = 1;
    int m_currentStreak = 0;
    int m_longestStreak = 0;
    int m_totalLoginDays = 0;
    QDate m_lastLoginDate;
    std::vector<Achievement> m_achievements;
};

} // namespace eduerp::services
