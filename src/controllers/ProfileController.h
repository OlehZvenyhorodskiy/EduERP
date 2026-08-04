#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <spdlog/spdlog.h>

namespace eduerp::ctrl {

/**
 * @brief Manages the current user's profile data in QML.
 */
class ProfileController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString displayName READ displayName NOTIFY profileChanged)
    Q_PROPERTY(QString email READ email NOTIFY profileChanged)
    Q_PROPERTY(QString role READ role NOTIFY profileChanged)
    Q_PROPERTY(QString username READ username NOTIFY profileChanged)
    Q_PROPERTY(QString bio READ bio NOTIFY profileChanged)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY profileChanged)
    Q_PROPERTY(QString language READ language NOTIFY settingsChanged)
    Q_PROPERTY(QString theme READ theme NOTIFY settingsChanged)
    Q_PROPERTY(bool energySaving READ energySaving NOTIFY settingsChanged)

public:
    explicit ProfileController(QObject* parent = nullptr)
        : QObject(parent)
        , m_networkManager(new QNetworkAccessManager(this))
    {}

    QString displayName() const { return m_displayName; }
    QString email() const { return m_email; }
    QString role() const { return m_role; }
    QString username() const { return m_username; }
    QString bio() const { return m_bio; }
    QString avatarUrl() const { return m_avatarUrl; }
    QString language() const { return m_language; }
    QString theme() const { return m_theme; }
    bool energySaving() const { return m_energySaving; }

    Q_INVOKABLE void fetchProfile() {
        QNetworkRequest request(QUrl(m_apiBaseUrl + "/users/me"));
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

        auto* reply = m_networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                auto doc = QJsonDocument::fromJson(reply->readAll());
                auto data = doc.object()["data"].toObject();
                m_displayName = data["display_name"].toString();
                m_email = data["email"].toString();
                m_role = data["role"].toString();
                m_username = data["username"].toString();
                m_bio = data["bio"].toString();
                m_avatarUrl = data["avatar_url"].toString();

                auto settings = data["settings"].toObject();
                m_language = settings["language"].toString("nl-BE");
                m_theme = settings["theme"].toString("system");
                m_energySaving = settings["energy_saving_mode"].toBool(false);

                emit profileChanged();
                emit settingsChanged();
            }
            reply->deleteLater();
        });
    }

    Q_INVOKABLE void updateProfile(const QString& displayName, const QString& bio) {
        QJsonObject body;
        body["display_name"] = displayName;
        body["bio"] = bio;

        QNetworkRequest request(QUrl(m_apiBaseUrl + "/users/me"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

        auto* reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                fetchProfile(); // Reload from server
            }
            reply->deleteLater();
        });
    }

    Q_INVOKABLE void updateSettings(const QString& language, const QString& theme, bool energySaving) {
        QJsonObject body;
        QJsonObject settings;
        settings["language"] = language;
        settings["theme"] = theme;
        settings["energy_saving_mode"] = energySaving;
        body["settings"] = settings;

        QNetworkRequest request(QUrl(m_apiBaseUrl + "/users/me"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

        auto* reply = m_networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                fetchProfile();
            }
            reply->deleteLater();
        });
    }

    void setAccessToken(const QString& token) { m_accessToken = token; }

signals:
    void profileChanged();
    void settingsChanged();

private:
    QNetworkAccessManager* m_networkManager;
    QString m_displayName;
    QString m_email;
    QString m_role;
    QString m_username;
    QString m_bio;
    QString m_avatarUrl;
    QString m_language = "nl-BE";
    QString m_theme = "system";
    bool m_energySaving = false;
    QString m_accessToken;
    QString m_apiBaseUrl = "http://localhost:8080/api/v1";
};

} // namespace eduerp::ctrl