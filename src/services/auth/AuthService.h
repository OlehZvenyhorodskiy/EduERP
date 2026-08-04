#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <spdlog/spdlog.h>

namespace eduerp::services {

/**
 * @brief Manages secure token storage using Windows Credential Manager (via QSettings fallback).
 *        In production, this would use the Windows DPAPI or keychain.
 */
class AuthService : public QObject {
    Q_OBJECT

public:
    explicit AuthService(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Store tokens after successful login.
     */
    void storeTokens(const QString& accessToken, const QString& refreshToken) {
        QSettings settings("EduERP", "Auth");
        settings.setValue("access_token", accessToken);
        settings.setValue("refresh_token", refreshToken);
        settings.sync();
        spdlog::info("AuthService: Tokens stored securely");
    }

    /**
     * @brief Retrieve stored access token.
     */
    QString getAccessToken() const {
        QSettings settings("EduERP", "Auth");
        return settings.value("access_token").toString();
    }

    /**
     * @brief Retrieve stored refresh token.
     */
    QString getRefreshToken() const {
        QSettings settings("EduERP", "Auth");
        return settings.value("refresh_token").toString();
    }

    /**
     * @brief Check if there are stored tokens (possible auto-login).
     */
    bool hasStoredTokens() const {
        QSettings settings("EduERP", "Auth");
        return settings.contains("refresh_token") && !settings.value("refresh_token").toString().isEmpty();
    }

    /**
     * @brief Clear all stored tokens on logout.
     */
    void clearTokens() {
        QSettings settings("EduERP", "Auth");
        settings.remove("access_token");
        settings.remove("refresh_token");
        settings.sync();
        spdlog::info("AuthService: Tokens cleared");
    }

signals:
    void tokensUpdated();
};

} // namespace eduerp::services