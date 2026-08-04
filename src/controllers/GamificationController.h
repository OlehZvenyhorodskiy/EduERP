#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <spdlog/spdlog.h>

#include "src/services/gamification/GamificationService.h"

namespace eduerp::ctrl {

/**
 * @brief QML-facing controller for gamification data.
 *        Exposes XP, level, streak, and achievements to QML views.
 */
class GamificationController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // XP & Levels
    Q_PROPERTY(int currentXP READ currentXP NOTIFY dataChanged)
    Q_PROPERTY(int currentLevel READ currentLevel NOTIFY dataChanged)
    Q_PROPERTY(int xpForNextLevel READ xpForNextLevel NOTIFY dataChanged)
    Q_PROPERTY(double levelProgress READ levelProgress NOTIFY dataChanged)
    Q_PROPERTY(QString levelTitle READ levelTitle NOTIFY dataChanged)

    // Streaks
    Q_PROPERTY(int currentStreak READ currentStreak NOTIFY dataChanged)
    Q_PROPERTY(int longestStreak READ longestStreak NOTIFY dataChanged)
    Q_PROPERTY(int totalLoginDays READ totalLoginDays NOTIFY dataChanged)

    // Achievements
    Q_PROPERTY(int unlockedAchievements READ unlockedAchievements NOTIFY dataChanged)
    Q_PROPERTY(int totalAchievements READ totalAchievements NOTIFY dataChanged)
    Q_PROPERTY(QJsonArray achievements READ achievements NOTIFY dataChanged)

public:
    explicit GamificationController(QObject* parent = nullptr) : QObject(parent) {}

    void setService(services::GamificationService* service) {
        m_service = service;
        connect(m_service, &services::GamificationService::xpChanged, this, &GamificationController::dataChanged);
        connect(m_service, &services::GamificationService::streakChanged, this, &GamificationController::dataChanged);
        connect(m_service, &services::GamificationService::leveledUp, this, [this](int level) {
            emit levelUp(level, levelTitle());
        });
        connect(m_service, &services::GamificationService::achievementUnlocked, this, [this](const QString& id, const QString& icon) {
            emit achievementUnlocked(id, icon);
            emit dataChanged();
        });
    }

    // Getters
    int currentXP() const { return m_service ? m_service->currentXP() : 0; }
    int currentLevel() const { return m_service ? m_service->currentLevel() : 1; }
    int xpForNextLevel() const { return m_service ? m_service->xpForNextLevel() : 100; }
    double levelProgress() const { return m_service ? m_service->levelProgress() : 0.0; }
    int currentStreak() const { return m_service ? m_service->currentStreak() : 0; }
    int longestStreak() const { return m_service ? m_service->longestStreak() : 0; }
    int totalLoginDays() const { return m_service ? m_service->totalLoginDays() : 0; }
    int unlockedAchievements() const { return m_service ? m_service->unlockedCount() : 0; }
    int totalAchievements() const { return m_service ? m_service->totalCount() : 0; }

    QString levelTitle() const {
        int level = currentLevel();
        if (level >= 20) return "🏆 ERP Master";
        if (level >= 15) return "⭐ Senior Manager";
        if (level >= 10) return "📈 Manager";
        if (level >= 7)  return "💼 Ondernemer";
        if (level >= 5)  return "📊 Analist";
        if (level >= 3)  return "📋 Medewerker";
        return "🌱 Beginner";
    }

    QJsonArray achievements() const {
        QJsonArray arr;
        if (!m_service) return arr;

        for (const auto& a : m_service->achievements()) {
            QJsonObject obj;
            obj["id"] = a.id;
            obj["title_key"] = a.titleKey;
            obj["description_key"] = a.descriptionKey;
            obj["icon"] = a.icon;
            obj["category"] = a.category;
            obj["unlocked"] = a.unlocked;
            obj["unlocked_date"] = a.unlockedDate;
            obj["required_value"] = a.requiredValue;
            arr.append(obj);
        }
        return arr;
    }

    // Actions callable from QML
    Q_INVOKABLE void registerLogin() {
        if (m_service) m_service->registerDailyLogin();
    }

signals:
    void dataChanged();
    void levelUp(int level, const QString& title);
    void achievementUnlocked(const QString& id, const QString& icon);

private:
    services::GamificationService* m_service = nullptr;
};

} // namespace eduerp::ctrl