#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <spdlog/spdlog.h>

namespace eduerp::ctrl {

/**
 * @brief Aggregates data from multiple sources into the dashboard view.
 *        Fetches recent activity, quick stats, and notifications from the API.
 */
class DashboardController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString welcomeMessage READ welcomeMessage NOTIFY dataChanged)
    Q_PROPERTY(int unreadMessages READ unreadMessages NOTIFY dataChanged)
    Q_PROPERTY(int pendingTasks READ pendingTasks NOTIFY dataChanged)
    Q_PROPERTY(int companiesOwned READ companiesOwned NOTIFY dataChanged)
    Q_PROPERTY(int currentStreak READ currentStreak NOTIFY dataChanged)
    Q_PROPERTY(QJsonArray recentActivity READ recentActivity NOTIFY dataChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)

public:
    explicit DashboardController(QObject* parent = nullptr)
        : QObject(parent)
        , m_networkManager(new QNetworkAccessManager(this))
    {}

    QString welcomeMessage() const { return m_welcomeMessage; }
    int unreadMessages() const { return m_unreadMessages; }
    int pendingTasks() const { return m_pendingTasks; }
    int companiesOwned() const { return m_companiesOwned; }
    int currentStreak() const { return m_currentStreak; }
    QJsonArray recentActivity() const { return m_recentActivity; }
    bool isLoading() const { return m_isLoading; }

    Q_INVOKABLE void refresh() {
        spdlog::info("DashboardController: Refreshing dashboard data");
        m_isLoading = true; emit loadingChanged();

        // For now, use placeholder data
        // In production, this fetches from /api/v1/dashboard
        m_welcomeMessage = "Welkom terug! 👋";
        m_unreadMessages = 0;
        m_pendingTasks = 0;
        m_companiesOwned = 0;
        m_currentStreak = 0;

        m_recentActivity = QJsonArray();

        m_isLoading = false; emit loadingChanged();
        emit dataChanged();
    }

    Q_INVOKABLE void setUserName(const QString& name) {
        m_welcomeMessage = "Welkom terug, " + name + "! 👋";
        emit dataChanged();
    }

signals:
    void dataChanged();
    void loadingChanged();

private:
    QNetworkAccessManager* m_networkManager;
    QString m_welcomeMessage = "Welkom terug! 👋";
    int m_unreadMessages = 0;
    int m_pendingTasks = 0;
    int m_companiesOwned = 0;
    int m_currentStreak = 0;
    QJsonArray m_recentActivity;
    bool m_isLoading = false;
};

} // namespace eduerp::ctrl
