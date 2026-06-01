package handlers

import (
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// GamificationHandler manages XP, streaks, and achievements via the API.
type GamificationHandler struct {
	DB *gorm.DB
}

// UserGamification represents a user's gamification state.
type UserGamification struct {
	ID             uint      `gorm:"primaryKey" json:"id"`
	UserID         uint      `json:"user_id" gorm:"uniqueIndex"`
	CurrentXP      int       `json:"current_xp" gorm:"default:0"`
	CurrentLevel   int       `json:"current_level" gorm:"default:1"`
	CurrentStreak  int       `json:"current_streak" gorm:"default:0"`
	LongestStreak  int       `json:"longest_streak" gorm:"default:0"`
	TotalLoginDays int       `json:"total_login_days" gorm:"default:0"`
	LastLoginDate  string    `json:"last_login_date"`
	UpdatedAt      time.Time `json:"updated_at"`
}

// UserAchievement represents an unlocked achievement.
type UserAchievement struct {
	ID            uint      `gorm:"primaryKey" json:"id"`
	UserID        uint      `json:"user_id" gorm:"index"`
	AchievementID string    `json:"achievement_id"`
	UnlockedAt    time.Time `json:"unlocked_at"`
}

// GetGamificationState returns the user's current XP, level, and streak stats.
func (h *GamificationHandler) GetGamificationState(c *gin.Context) {
	userID := c.GetUint("user_id")

	var state UserGamification
	result := h.DB.Where("user_id = ?", userID).First(&state)
	if result.Error != nil {
		// First time — create initial state
		state = UserGamification{
			UserID:       userID,
			CurrentXP:    0,
			CurrentLevel: 1,
		}
		h.DB.Create(&state)
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    state,
	})
}

// SyncGamificationState syncs client-side state back to server.
func (h *GamificationHandler) SyncGamificationState(c *gin.Context) {
	userID := c.GetUint("user_id")

	var input struct {
		CurrentXP      int    `json:"current_xp"`
		CurrentLevel   int    `json:"current_level"`
		CurrentStreak  int    `json:"current_streak"`
		LongestStreak  int    `json:"longest_streak"`
		TotalLoginDays int    `json:"total_login_days"`
		LastLoginDate  string `json:"last_login_date"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldige gamificatie-gegevens"},
		})
		return
	}

	updates := map[string]interface{}{
		"current_xp":      input.CurrentXP,
		"current_level":   input.CurrentLevel,
		"current_streak":  input.CurrentStreak,
		"longest_streak":  input.LongestStreak,
		"total_login_days": input.TotalLoginDays,
		"last_login_date": input.LastLoginDate,
	}

	h.DB.Model(&UserGamification{}).Where("user_id = ?", userID).Updates(updates)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    gin.H{"synced": true},
	})
}

// GetAchievements returns all unlocked achievements for a user.
func (h *GamificationHandler) GetAchievements(c *gin.Context) {
	userID := c.GetUint("user_id")

	var achievements []UserAchievement
	h.DB.Where("user_id = ?", userID).
		Order("unlocked_at DESC").
		Find(&achievements)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"achievements": achievements,
			"count":        len(achievements),
		},
	})
}

// UnlockAchievement records a newly unlocked achievement.
func (h *GamificationHandler) UnlockAchievement(c *gin.Context) {
	userID := c.GetUint("user_id")

	var input struct {
		AchievementID string `json:"achievement_id" binding:"required"`
	}

	if err := c.ShouldBindJSON(&input); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Geef een achievement ID op"},
		})
		return
	}

	// Check for duplicate
	var existing UserAchievement
	if h.DB.Where("user_id = ? AND achievement_id = ?", userID, input.AchievementID).First(&existing).Error == nil {
		c.JSON(http.StatusOK, gin.H{
			"success": true,
			"data":    gin.H{"already_unlocked": true},
		})
		return
	}

	achievement := UserAchievement{
		UserID:        userID,
		AchievementID: input.AchievementID,
		UnlockedAt:    time.Now(),
	}

	h.DB.Create(&achievement)

	c.JSON(http.StatusCreated, gin.H{
		"success": true,
		"data":    achievement,
	})
}

// GetLeaderboardByXP returns top users by XP within a school.
func (h *GamificationHandler) GetLeaderboardByXP(c *gin.Context) {
	schoolID := c.GetUint("school_id")
	limit := 20
	if l, err := strconv.Atoi(c.DefaultQuery("limit", "20")); err == nil && l > 0 && l <= 100 {
		limit = l
	}

	type XPLeaderboardEntry struct {
		UserID       uint   `json:"user_id"`
		DisplayName  string `json:"display_name"`
		CurrentXP    int    `json:"current_xp"`
		CurrentLevel int    `json:"current_level"`
		Streak       int    `json:"current_streak"`
		Rank         int    `json:"rank"`
	}

	var entries []XPLeaderboardEntry
	h.DB.Raw(`
		SELECT ug.user_id, u.display_name, ug.current_xp, ug.current_level, ug.current_streak
		FROM user_gamifications ug
		JOIN users u ON u.id = ug.user_id
		WHERE u.school_id = ?
		ORDER BY ug.current_xp DESC
		LIMIT ?
	`, schoolID, limit).Scan(&entries)

	for i := range entries {
		entries[i].Rank = i + 1
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"leaderboard": entries,
		},
	})
}
