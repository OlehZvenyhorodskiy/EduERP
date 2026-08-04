package handlers

import (
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// SocialHandler manages friend system endpoints.
type SocialHandler struct {
	DB *gorm.DB
}

// Friendship represents a friend connection between two users.
type Friendship struct {
	ID         uint      `gorm:"primaryKey" json:"id"`
	RequesterID uint     `json:"requester_id"`
	ReceiverID  uint     `json:"receiver_id"`
	Status      string   `json:"status" gorm:"default:pending"` // pending, accepted, rejected
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// GetFriends returns the user's accepted friends.
func (h *SocialHandler) GetFriends(c *gin.Context) {
	userID := c.GetUint("user_id")

	var friendships []Friendship
	h.DB.Where("(requester_id = ? OR receiver_id = ?) AND status = ?", userID, userID, "accepted").
		Find(&friendships)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"friends": friendships,
			"count":   len(friendships),
		},
	})
}

// GetFriendRequests returns pending friend requests for the user.
func (h *SocialHandler) GetFriendRequests(c *gin.Context) {
	userID := c.GetUint("user_id")

	var requests []Friendship
	h.DB.Where("receiver_id = ? AND status = ?", userID, "pending").
		Order("created_at DESC").
		Find(&requests)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"requests": requests,
			"count":    len(requests),
		},
	})
}

// SendFriendRequest sends a friend request to another user.
func (h *SocialHandler) SendFriendRequest(c *gin.Context) {
	userID := c.GetUint("user_id")

	var input struct {
		ReceiverID uint `json:"receiver_id" binding:"required"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Geef een geldige gebruiker-ID op"},
		})
		return
	}

	if input.ReceiverID == userID {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Je kunt jezelf niet als vriend toevoegen"},
		})
		return
	}

	// Check for existing friendship
	var existing Friendship
	err := h.DB.Where(
		"(requester_id = ? AND receiver_id = ?) OR (requester_id = ? AND receiver_id = ?)",
		userID, input.ReceiverID, input.ReceiverID, userID,
	).First(&existing).Error

	if err == nil {
		if existing.Status == "accepted" {
			c.JSON(http.StatusConflict, gin.H{
				"success": false,
				"error":   gin.H{"code": "ALREADY_FRIENDS", "message": "Jullie zijn al vrienden"},
			})
		} else {
			c.JSON(http.StatusConflict, gin.H{
				"success": false,
				"error":   gin.H{"code": "REQUEST_EXISTS", "message": "Er is al een verzoek verstuurd"},
			})
		}
		return
	}

	friendship := Friendship{
		RequesterID: userID,
		ReceiverID:  input.ReceiverID,
		Status:      "pending",
	}

	if err := h.DB.Create(&friendship).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "DB_ERROR", "message": "Vriendschapsverzoek mislukt"},
		})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"success": true,
		"data":    friendship,
	})
}

// RespondToFriendRequest accepts or rejects a friend request.
func (h *SocialHandler) RespondToFriendRequest(c *gin.Context) {
	userID := c.GetUint("user_id")
	requestID, err := strconv.ParseUint(c.Param("id"), 10, 32)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldig verzoek-ID"},
		})
		return
	}

	var input struct {
		Action string `json:"action" binding:"required,oneof=accept reject"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Kies 'accept' of 'reject'"},
		})
		return
	}

	var friendship Friendship
	if err := h.DB.Where("id = ? AND receiver_id = ? AND status = ?", requestID, userID, "pending").
		First(&friendship).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{
			"success": false,
			"error":   gin.H{"code": "NOT_FOUND", "message": "Vriendschapsverzoek niet gevonden"},
		})
		return
	}

	newStatus := "rejected"
	if input.Action == "accept" {
		newStatus = "accepted"
	}

	h.DB.Model(&friendship).Update("status", newStatus)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"status": newStatus,
		},
	})
}

// RemoveFriend removes an existing friendship.
func (h *SocialHandler) RemoveFriend(c *gin.Context) {
	userID := c.GetUint("user_id")
	friendID, err := strconv.ParseUint(c.Param("id"), 10, 32)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldig vriend-ID"},
		})
		return
	}

	result := h.DB.Where(
		"((requester_id = ? AND receiver_id = ?) OR (requester_id = ? AND receiver_id = ?)) AND status = ?",
		userID, friendID, friendID, userID, "accepted",
	).Delete(&Friendship{})

	if result.RowsAffected == 0 {
		c.JSON(http.StatusNotFound, gin.H{
			"success": false,
			"error":   gin.H{"code": "NOT_FOUND", "message": "Vriendschap niet gevonden"},
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    gin.H{"removed": true},
	})
}