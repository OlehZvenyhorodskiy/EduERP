#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <spdlog/spdlog.h>

namespace eduerp::infra {

/**
 * @brief WebSocket client for real-time collaboration.
 *        Handles connection, reconnection with exponential backoff,
 *        and heartbeat ping/pong per the spec (30s interval).
 */
class WebSocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebSocketClient(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(&m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
        connect(&m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
        connect(&m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);

        // Heartbeat timer (30 seconds per spec)
        m_heartbeatTimer.setInterval(30000);
        connect(&m_heartbeatTimer, &QTimer::timeout, this, [this]() {
            if (m_socket.isValid()) {
                m_socket.ping();
            }
        });

        // Reconnect timer
        connect(&m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::attemptReconnect);
    }

    void connectToServer(const QString& url, const QString& jwt) {
        m_url = url;
        m_jwt = jwt;
        m_reconnectAttempts = 0;

        QNetworkRequest request{QUrl(url)};
        request.setRawHeader("Authorization", ("Bearer " + jwt).toUtf8());
        m_socket.open(request);
    }

    void disconnect() {
        m_heartbeatTimer.stop();
        m_reconnectTimer.stop();
        m_socket.close();
    }

    void sendMessage(const QJsonObject& message) {
        if (m_socket.isValid()) {
            m_socket.sendTextMessage(QJsonDocument(message).toJson(QJsonDocument::Compact));
        }
    }

    void joinChannel(const QString& channel) {
        QJsonObject msg;
        msg["type"] = "subscribe";
        msg["channel"] = channel;
        sendMessage(msg);
    }

signals:
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject& message);
    void connectionError(const QString& error);

private slots:
    void onConnected() {
        spdlog::info("WebSocket: Connected to server");
        m_reconnectAttempts = 0;
        m_heartbeatTimer.start();
        emit connected();
    }

    void onDisconnected() {
        spdlog::warn("WebSocket: Disconnected from server");
        m_heartbeatTimer.stop();
        scheduleReconnect();
        emit disconnected();
    }

    void onTextMessageReceived(const QString& message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (doc.isObject()) {
            emit messageReceived(doc.object());
        }
    }

    void attemptReconnect() {
        spdlog::info("WebSocket: Reconnection attempt #{}", m_reconnectAttempts + 1);
        QNetworkRequest request{QUrl(m_url)};
        request.setRawHeader("Authorization", ("Bearer " + m_jwt).toUtf8());
        m_socket.open(request);
    }

private:
    void scheduleReconnect() {
        // Exponential backoff: 1s, 2s, 4s, 8s, max 30s
        int delay = std::min(1000 * (1 << m_reconnectAttempts), 30000);
        m_reconnectAttempts++;
        spdlog::info("WebSocket: Scheduling reconnect in {}ms", delay);
        m_reconnectTimer.start(delay);
    }

    QWebSocket m_socket;
    QTimer m_heartbeatTimer;
    QTimer m_reconnectTimer;
    QString m_url;
    QString m_jwt;
    int m_reconnectAttempts = 0;
};

} // namespace eduerp::infra