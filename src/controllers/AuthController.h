#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <spdlog/spdlog.h>

namespace eduerp::ctrl {

/**
 * @brief Manages the OAuth 2.0 + PKCE authentication flow.
 *        Exposes login/logout/token refresh to QML.
 */
class AuthController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY authStateChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY authStateChanged)
    Q_PROPERTY(QString userRole READ userRole NOTIFY authStateChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)

public:
    explicit AuthController(QObject* parent = nullptr)
        : QObject(parent)
        , m_networkManager(new QNetworkAccessManager(this))
    {}

    bool isLoggedIn() const { return m_isLoggedIn; }
    QString userName() const { return m_userName; }
    QString userRole() const { return m_userRole; }
    bool isLoading() const { return m_isLoading; }
    QString errorMessage() const { return m_errorMessage; }

    /**
     * @brief Initiate OAuth login with the given provider (google/microsoft).
     */
    Q_INVOKABLE void loginWithProvider(const QString& provider) {
        spdlog::info("AuthController: Initiating OAuth login with '{}'", provider.toStdString());
        setLoading(true);
        clearError();

        // Generate PKCE code_verifier and code_challenge
        m_codeVerifier = generateCodeVerifier();
        QString codeChallenge = generateCodeChallenge(m_codeVerifier);

        // Build the OAuth authorization URL
        QUrl authUrl;
        if (provider == "google") {
            authUrl = QUrl("https://accounts.google.com/o/oauth2/v2/auth");
            authUrl.setQuery(QString(
                "client_id=%1"
                "&redirect_uri=%2"
                "&response_type=code"
                "&scope=openid%20email%20profile"
                "&code_challenge=%3"
                "&code_challenge_method=S256"
                "&prompt=select_account"
            ).arg(m_googleClientId, m_redirectUri, codeChallenge));
        } else if (provider == "microsoft") {
            authUrl = QUrl(QString("https://login.microsoftonline.com/%1/oauth2/v2.0/authorize").arg(m_microsoftTenantId));
            authUrl.setQuery(QString(
                "client_id=%1"
                "&redirect_uri=%2"
                "&response_type=code"
                "&scope=openid%20email%20profile"
                "&code_challenge=%3"
                "&code_challenge_method=S256"
                "&prompt=select_account"
            ).arg(m_microsoftClientId, m_redirectUri, codeChallenge));
        }

        m_currentProvider = provider;
        QDesktopServices::openUrl(authUrl);

        // The callback will be handled by a local HTTP server or deep link
        spdlog::info("AuthController: Opened OAuth URL in browser");
    }

    /**
     * @brief Handle the OAuth callback with authorization code.
     */
    Q_INVOKABLE void handleOAuthCallback(const QString& code) {
        spdlog::info("AuthController: Received OAuth callback code");
        setLoading(true);

        QJsonObject body;
        body["provider"] = m_currentProvider;
        body["code"] = code;
        body["code_verifier"] = m_codeVerifier;
        body["redirect_uri"] = m_redirectUri;

        QNetworkRequest request(QUrl(m_apiBaseUrl + "/auth/login"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        auto* reply = m_networkManager->post(request, QJsonDocument(body).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            handleLoginResponse(reply);
            reply->deleteLater();
        });
    }

    /**
     * @brief Refresh the access token using the stored refresh token.
     */
    Q_INVOKABLE void refreshToken() {
        if (m_refreshToken.isEmpty()) {
            setError("Geen refresh token beschikbaar");
            return;
        }

        QNetworkRequest request(QUrl(m_apiBaseUrl + "/auth/refresh"));
        request.setRawHeader("Authorization", ("Bearer " + m_refreshToken).toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        auto* reply = m_networkManager->post(request, QByteArray("{}"));
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                auto doc = QJsonDocument::fromJson(reply->readAll());
                auto data = doc.object()["data"].toObject();
                m_accessToken = data["access_token"].toString();
                spdlog::info("AuthController: Access token refreshed");
                emit tokenRefreshed();
            } else {
                spdlog::warn("AuthController: Token refresh failed");
                logout();
            }
            reply->deleteLater();
        });
    }

    /**
     * @brief Log out and revoke the session.
     */
    Q_INVOKABLE void logout(bool revokeAll = false) {
        spdlog::info("AuthController: Logging out (revokeAll={})", revokeAll);

        if (!m_accessToken.isEmpty()) {
            QJsonObject body;
            body["revoke_all"] = revokeAll;

            QNetworkRequest request(QUrl(m_apiBaseUrl + "/auth/logout"));
            request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            m_networkManager->post(request, QJsonDocument(body).toJson());
        }

        m_accessToken.clear();
        m_refreshToken.clear();
        m_isLoggedIn = false;
        m_userName.clear();
        m_userRole.clear();
        emit authStateChanged();
    }

    QString accessToken() const { return m_accessToken; }

signals:
    void authStateChanged();
    void loadingChanged();
    void errorChanged();
    void loginSuccessful();
    void tokenRefreshed();

private:
    void handleLoginResponse(QNetworkReply* reply) {
        setLoading(false);

        if (reply->error() != QNetworkReply::NoError) {
            auto doc = QJsonDocument::fromJson(reply->readAll());
            auto error = doc.object()["error"].toObject();
            setError(error["message"].toString("Inloggen mislukt"));
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());
        auto data = doc.object()["data"].toObject();

        m_accessToken = data["access_token"].toString();
        m_refreshToken = data["refresh_token"].toString();

        auto user = data["user"].toObject();
        m_userName = user["display_name"].toString();
        m_userRole = user["role"].toString();
        m_isLoggedIn = true;

        spdlog::info("AuthController: Login successful for '{}'", m_userName.toStdString());

        emit authStateChanged();
        emit loginSuccessful();
    }

    void setLoading(bool loading) {
        if (m_isLoading != loading) {
            m_isLoading = loading;
            emit loadingChanged();
        }
    }

    void setError(const QString& msg) {
        m_errorMessage = msg;
        emit errorChanged();
    }

    void clearError() {
        if (!m_errorMessage.isEmpty()) {
            m_errorMessage.clear();
            emit errorChanged();
        }
    }

    QString generateCodeVerifier() {
        // Cryptographically random 43-128 char string
        const int length = 64;
        const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
        QString verifier;
        verifier.reserve(length);
        for (int i = 0; i < length; ++i) {
            verifier.append(charset[QRandomGenerator::global()->bounded(static_cast<int>(sizeof(charset) - 1))]);
        }
        return verifier;
    }

    QString generateCodeChallenge(const QString& verifier) {
        auto hash = QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
        return hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    }

    QNetworkAccessManager* m_networkManager;
    QString m_accessToken;
    QString m_refreshToken;
    QString m_codeVerifier;
    QString m_currentProvider;
    QString m_userName;
    QString m_userRole;
    bool m_isLoggedIn = false;
    bool m_isLoading = false;
    QString m_errorMessage;

    // These would be loaded from config
    QString m_apiBaseUrl = "http://localhost:8080/api/v1";
    QString m_redirectUri = "http://localhost:8765/callback";
    QString m_googleClientId;
    QString m_microsoftClientId;
    QString m_microsoftTenantId;
};

} // namespace eduerp::ctrl