package handlers

import (
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// MessageHandler manages the messaging API endpoints.
type MessageHandler struct {
	DB *gorm.DB
}

// Message represents a chat message.
type Message struct {
	ID          uint      `gorm:"primaryKey" json:"id"`
	SenderID    uint      `json:"sender_id"`
	ReceiverID  *uint     `json:"receiver_id,omitempty"`
	TeamID      *uint     `json:"team_id,omitempty"`
	Content     string    `json:"content" binding:"required,max=2000"`
	MessageType string    `json:"message_type" gorm:"default:text"`
	IsRead      bool      `json:"is_read" gorm:"default:false"`
	CreatedAt   time.Time `json:"created_at"`
}

// Conversation aggregates messages between users or within a team.
type Conversation struct {
	ID              uint   `json:"id"`
	ConversationType string `json:"conversation_type"` // direct, team
	Title           string `json:"title"`
	ParticipantIDs  []uint `json:"participant_ids"`
	LastMessage     string `json:"last_message"`
	LastMessageAt   string `json:"last_message_at"`
	UnreadCount     int    `json:"unread_count"`
}

// GetConversations returns the user's active conversations.
func (h *MessageHandler) GetConversations(c *gin.Context) {
	userID := c.GetUint("user_id")

	// Direct messages + team chats where user is a member
	// Simplified: return recent direct message threads
	var messages []Message
	h.DB.Where("sender_id = ? OR receiver_id = ?", userID, userID).
		Order("created_at DESC").
		Limit(50).
		Find(&messages)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"conversations": messages,
		},
	})
}

// GetMessages returns messages for a specific conversation.
func (h *MessageHandler) GetMessages(c *gin.Context) {
	conversationID, err := strconv.ParseUint(c.Param("id"), 10, 32)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldig gesprek-ID"},
		})
		return
	}

	limit := 50
	if l, err := strconv.Atoi(c.DefaultQuery("limit", "50")); err == nil && l > 0 && l <= 100 {
		limit = l
	}

	var messages []Message
	h.DB.Where("team_id = ? OR (sender_id = ? AND receiver_id = ?) OR (sender_id = ? AND receiver_id = ?)",
		conversationID, c.GetUint("user_id"), conversationID, conversationID, c.GetUint("user_id")).
		Order("created_at ASC").
		Limit(limit).
		Find(&messages)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"messages": messages,
			"count":    len(messages),
		},
	})
}

// SendMessage sends a new message to a user or team.
func (h *MessageHandler) SendMessage(c *gin.Context) {
	userID := c.GetUint("user_id")

	var input struct {
		ReceiverID  *uint  `json:"receiver_id"`
		TeamID      *uint  `json:"team_id"`
		Content     string `json:"content" binding:"required,max=2000"`
		MessageType string `json:"message_type"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Bericht mag niet leeg zijn (max 2000 tekens)"},
		})
		return
	}

	if input.ReceiverID == nil && input.TeamID == nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Geef een ontvanger of team op"},
		})
		return
	}

	msgType := "text"
	if input.MessageType != "" {
		msgType = input.MessageType
	}

	msg := Message{
		SenderID:    userID,
		ReceiverID:  input.ReceiverID,
		TeamID:      input.TeamID,
		Content:     input.Content,
		MessageType: msgType,
	}

	if err := h.DB.Create(&msg).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "DB_ERROR", "message": "Bericht kon niet worden verzonden"},
		})
		return
	}

	// TODO: Push via WebSocket hub to receiver

	c.JSON(http.StatusCreated, gin.H{
		"success": true,
		"data":    msg,
	})
}

// MarkRead marks messages as read.
func (h *MessageHandler) MarkRead(c *gin.Context) {
	userID := c.GetUint("user_id")

	var input struct {
		MessageIDs []uint `json:"message_ids" binding:"required"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Geef bericht-IDs op"},
		})
		return
	}

	h.DB.Model(&Message{}).
		Where("id IN ? AND receiver_id = ?", input.MessageIDs, userID).
		Update("is_read", true)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    gin.H{"marked_read": len(input.MessageIDs)},
	})
}
