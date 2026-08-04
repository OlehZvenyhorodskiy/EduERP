#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <functional>
#include <spdlog/spdlog.h>

namespace eduerp::infra {

/**
 * @brief Centralized HTTP client wrapping QNetworkAccessManager.
 *        Automatically attaches JWT bearer token to all requests.
 *        Provides consistent error handling per the API contract.
 */
class HttpClient : public QObject {
    Q_OBJECT

public:
    using ResponseCallback = std::function<void(bool success, const QJsonObject& data, const QString& error)>;

    explicit HttpClient(QObject* parent = nullptr)
        : QObject(parent)
        , m_manager(new QNetworkAccessManager(this))
    {}

    void setBaseUrl(const QString& url) { m_baseUrl = url; }
    void setAccessToken(const QString& token) { m_accessToken = token; }

    void get(const QString& endpoint, ResponseCallback callback) {
        auto request = buildRequest(endpoint);
        auto* reply = m_manager->get(request);
        connectReply(reply, std::move(callback));
    }

    void post(const QString& endpoint, const QJsonObject& body, ResponseCallback callback) {
        auto request = buildRequest(endpoint);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        auto* reply = m_manager->post(request, QJsonDocument(body).toJson());
        connectReply(reply, std::move(callback));
    }

    void patch(const QString& endpoint, const QJsonObject& body, ResponseCallback callback) {
        auto request = buildRequest(endpoint);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        auto* reply = m_manager->sendCustomRequest(request, "PATCH", QJsonDocument(body).toJson());
        connectReply(reply, std::move(callback));
    }

    void del(const QString& endpoint, ResponseCallback callback) {
        auto request = buildRequest(endpoint);
        auto* reply = m_manager->deleteResource(request);
        connectReply(reply, std::move(callback));
    }

signals:
    void unauthorizedResponse();

private:
    QNetworkRequest buildRequest(const QString& endpoint) {
        QUrl url(m_baseUrl + endpoint);
        QNetworkRequest request(url);
        request.setRawHeader("X-Client-Version", "1.0.0");

        if (!m_accessToken.isEmpty()) {
            request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
        }

        return request;
    }

    void connectReply(QNetworkReply* reply, ResponseCallback callback) {
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb = std::move(callback)]() {
            auto doc = QJsonDocument::fromJson(reply->readAll());
            auto root = doc.object();

            if (reply->error() == QNetworkReply::NoError && root["success"].toBool()) {
                cb(true, root["data"].toObject(), "");
            } else {
                auto error = root["error"].toObject();
                QString errMsg = error["message"].toString("Onbekende fout");
                QString errCode = error["code"].toString("UNKNOWN");

                spdlog::warn("HttpClient: {} - {} ({})", reply->url().toString().toStdString(), errCode.toStdString(), errMsg.toStdString());

                if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
                    emit unauthorizedResponse();
                }

                cb(false, QJsonObject{}, errMsg);
            }

            reply->deleteLater();
        });
    }

    QNetworkAccessManager* m_manager;
    QString m_baseUrl = "http://localhost:8080/api/v1";
    QString m_accessToken;
};

} // namespace eduerp::infra