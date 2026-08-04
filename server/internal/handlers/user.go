package handlers

import (
	"net/http"

	"github.com/eduerp/server/internal/models"
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// UserHandler manages user-related endpoints.
type UserHandler struct {
	db *gorm.DB
}

func NewUserHandler(db *gorm.DB) *UserHandler {
	return &UserHandler{db: db}
}

// GetCurrentUser returns the authenticated user's full profile (GET /users/me).
func (h *UserHandler) GetCurrentUser(c *gin.Context) {
	userID, _ := c.Get("user_id")

	var user models.User
	if err := h.db.Preload("School").First(&user, userID).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{
			"success": false,
			"error":   gin.H{"code": "USER_NOT_FOUND", "message": "Gebruiker niet gevonden."},
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"id":           user.ID,
			"email":        user.Email,
			"display_name": user.DisplayName,
			"username":     user.Username,
			"avatar_url":   user.AvatarURL,
			"banner_url":   user.BannerURL,
			"bio":          user.Bio,
			"role":         user.Role,
			"school_id":    user.SchoolID,
			"settings": gin.H{
				"language":             user.PreferredLang,
				"theme":                user.ThemePreference,
				"font_size":            user.FontSize,
				"animation_preference": user.AnimationPref,
				"energy_saving_mode":   user.EnergySaving,
			},
			"privacy": gin.H{
				"profile_visibility":    user.ProfileVisibility,
				"friend_requests_allowed": user.FriendRequests,
			},
			"is_active":     user.IsActive,
			"last_login_at": user.LastLoginAt,
			"created_at":    user.CreatedAt,
		},
	})
}

// UpdateProfile partially updates the current user's profile (PATCH /users/me).
func (h *UserHandler) UpdateProfile(c *gin.Context) {
	userID, _ := c.Get("user_id")

	var body map[string]interface{}
	if err := c.ShouldBindJSON(&body); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "De opgegeven gegevens zijn ongeldig."},
		})
		return
	}

	// Field whitelist to prevent updating protected fields
	allowed := map[string]string{
		"display_name": "display_name",
		"bio":          "bio",
	}

	updates := map[string]interface{}{}
	for key, col := range allowed {
		if val, ok := body[key]; ok {
			updates[col] = val
		}
	}

	// Handle nested settings
	if settings, ok := body["settings"].(map[string]interface{}); ok {
		if v, ok := settings["theme"]; ok {
			updates["theme_preference"] = v
		}
		if v, ok := settings["font_size"]; ok {
			updates["font_size"] = v
		}
		if v, ok := settings["animation_preference"]; ok {
			updates["animation_pref"] = v
		}
		if v, ok := settings["energy_saving_mode"]; ok {
			updates["energy_saving"] = v
		}
		if v, ok := settings["language"]; ok {
			updates["preferred_lang"] = v
		}
	}

	// Handle nested privacy
	if privacy, ok := body["privacy"].(map[string]interface{}); ok {
		if v, ok := privacy["profile_visibility"]; ok {
			updates["profile_visibility"] = v
		}
		if v, ok := privacy["friend_requests_allowed"]; ok {
			updates["friend_requests"] = v
		}
	}

	if len(updates) == 0 {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "NO_CHANGES", "message": "Geen wijzigingen opgegeven."},
		})
		return
	}

	if err := h.db.Model(&models.User{}).Where("id = ?", userID).Updates(updates).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "UPDATE_FAILED", "message": "Kan profiel niet bijwerken."},
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{"success": true, "data": gin.H{"message": "Profiel bijgewerkt"}})
}

// ListUsers returns paginated users for admins/teachers (GET /users).
func (h *UserHandler) ListUsers(c *gin.Context) {
	schoolID, _ := c.Get("school_id")

	query := h.db.Where("school_id = ?", schoolID)

	if role := c.Query("role"); role != "" {
		query = query.Where("role = ?", role)
	}
	if search := c.Query("search"); search != "" {
		query = query.Where("display_name ILIKE ? OR email ILIKE ?", "%"+search+"%", "%"+search+"%")
	}

	var users []models.User
	query.Limit(20).Find(&users)

	items := make([]gin.H, len(users))
	for i, u := range users {
		items[i] = gin.H{
			"id":           u.ID,
			"email":        u.Email,
			"display_name": u.DisplayName,
			"role":         u.Role,
			"is_active":    u.IsActive,
			"last_login_at": u.LastLoginAt,
		}
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"items":      items,
			"pagination": gin.H{"has_more": len(users) == 20},
		},
	})
}

// BulkCreateUsers creates multiple user accounts (POST /users/bulk).
func (h *UserHandler) BulkCreateUsers(c *gin.Context) {
	schoolID, _ := c.Get("school_id")

	var body struct {
		Users []struct {
			Email       string `json:"email" binding:"required,email"`
			DisplayName string `json:"display_name" binding:"required"`
			ClassID     *uint  `json:"class_id"`
		} `json:"users" binding:"required"`
	}

	if err := c.ShouldBindJSON(&body); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldige gegevens."},
		})
		return
	}

	results := make([]gin.H, 0, len(body.Users))
	created := 0

	for _, u := range body.Users {
		user := models.User{
			SchoolID:      schoolID.(uint),
			Email:         u.Email,
			OAuthProvider: "pending",
			OAuthSubject:  "pending-" + u.Email,
			Role:          "student",
			DisplayName:   u.DisplayName,
			IsActive:      true,
		}

		if err := h.db.Create(&user).Error; err != nil {
			results = append(results, gin.H{"email": u.Email, "status": "failed", "error": err.Error()})
			continue
		}

		created++
		results = append(results, gin.H{"email": u.Email, "status": "created", "user_id": user.ID})
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"created": created,
			"failed":  len(body.Users) - created,
			"results": results,
		},
	})
}