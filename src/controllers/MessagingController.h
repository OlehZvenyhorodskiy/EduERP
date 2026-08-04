#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <spdlog/spdlog.h>

namespace eduerp::ctrl {

struct ChatMessage {
    int id = 0;
    int senderId = 0;
    QString senderName;
    QString content;
    QString timestamp;
    bool isFromMe = false;
};

/**
 * @brief Manages real-time chat messages via WebSocket.
 *        Exposes chat state to QML for the ConversationsView.
 */
class MessagingController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int activeConversationId READ activeConversationId NOTIFY conversationChanged)
    Q_PROPERTY(QString activeConversationTitle READ activeConversationTitle NOTIFY conversationChanged)
    Q_PROPERTY(QJsonArray conversations READ conversations NOTIFY conversationsListChanged)
    Q_PROPERTY(QJsonArray messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged)

public:
    explicit MessagingController(QObject* parent = nullptr) : QObject(parent) {}

    int activeConversationId() const { return m_activeConversationId; }
    QString activeConversationTitle() const { return m_activeConversationTitle; }
    QJsonArray conversations() const { return m_conversations; }
    QJsonArray messages() const { return m_messages; }
    int unreadCount() const { return m_unreadCount; }

    /**
     * @brief Select a conversation to view its messages.
     */
    Q_INVOKABLE void openConversation(int conversationId, const QString& title) {
        m_activeConversationId = conversationId;
        m_activeConversationTitle = title;
        m_messages = QJsonArray(); // Would load from cache/server
        emit conversationChanged();
        emit messagesChanged();
        spdlog::info("MessagingController: Opened conversation '{}' (id={})",
                     title.toStdString(), conversationId);
    }

    /**
     * @brief Send a message in the active conversation.
     */
    Q_INVOKABLE void sendMessage(const QString& content) {
        if (m_activeConversationId <= 0 || content.trimmed().isEmpty()) return;

        QJsonObject msg;
        msg["id"] = ++m_nextMsgId;
        msg["sender_id"] = m_currentUserId;
        msg["sender_name"] = "Jij";
        msg["content"] = content.trimmed();
        msg["timestamp"] = QDateTime::currentDateTime().toString("HH:mm");
        msg["is_from_me"] = true;

        m_messages.append(msg);
        emit messagesChanged();

        // Would send via WebSocket: ws.send({ type: "team:chat_message", data: msg })
        spdlog::info("MessagingController: Sent message in conversation {}", m_activeConversationId);
    }

    /**
     * @brief Handle an incoming message from WebSocket.
     */
    void handleIncomingMessage(int conversationId, int senderId, const QString& senderName,
                               const QString& content)
    {
        if (conversationId == m_activeConversationId) {
            QJsonObject msg;
            msg["id"] = ++m_nextMsgId;
            msg["sender_id"] = senderId;
            msg["sender_name"] = senderName;
            msg["content"] = content;
            msg["timestamp"] = QDateTime::currentDateTime().toString("HH:mm");
            msg["is_from_me"] = false;

            m_messages.append(msg);
            emit messagesChanged();
        } else {
            m_unreadCount++;
            emit unreadCountChanged();
        }
    }

    void setCurrentUserId(int userId) { m_currentUserId = userId; }

signals:
    void conversationChanged();
    void conversationsListChanged();
    void messagesChanged();
    void unreadCountChanged();

private:
    int m_activeConversationId = 0;
    QString m_activeConversationTitle;
    QJsonArray m_conversations;
    QJsonArray m_messages;
    int m_unreadCount = 0;
    int m_currentUserId = 0;
    int m_nextMsgId = 0;
};

} // namespace eduerp::ctrl