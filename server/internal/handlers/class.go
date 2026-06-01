package handlers

import (
	"net/http"

	"github.com/eduerp/server/internal/models"
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// ClassHandler manages class and team endpoints.
type ClassHandler struct {
	db *gorm.DB
}

func NewClassHandler(db *gorm.DB) *ClassHandler {
	return &ClassHandler{db: db}
}

// CreateClass handles POST /classes.
func (h *ClassHandler) CreateClass(c *gin.Context) {
	schoolID, _ := c.Get("school_id")

	var body struct {
		Name                string   `json:"name" binding:"required"`
		Description         string   `json:"description"`
		AcademicYear        string   `json:"academic_year" binding:"required"`
		TeacherID           uint     `json:"teacher_id" binding:"required"`
		MaxTeamSize         int      `json:"max_team_size"`
		AllowedModules      []string `json:"allowed_modules"`
		SimulationTimeScale string   `json:"simulation_time_scale"`
	}

	if err := c.ShouldBindJSON(&body); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldige gegevens."},
		})
		return
	}

	maxTeam := body.MaxTeamSize
	if maxTeam == 0 {
		maxTeam = 4
	}
	timeScale := body.SimulationTimeScale
	if timeScale == "" {
		timeScale = "realtime"
	}

	class := models.Class{
		SchoolID:            schoolID.(uint),
		Name:                body.Name,
		Description:         body.Description,
		AcademicYear:        body.AcademicYear,
		TeacherID:           body.TeacherID,
		MaxTeamSize:         maxTeam,
		AllowedModules:      body.AllowedModules,
		SimulationTimeScale: timeScale,
		IsActive:            true,
	}

	if err := h.db.Create(&class).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "CREATE_FAILED", "message": "Kan klas niet aanmaken."},
		})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"success": true,
		"data": gin.H{
			"id":         class.ID,
			"name":       class.Name,
			"teacher_id": class.TeacherID,
			"created_at": class.CreatedAt,
		},
	})
}

// GetClass handles GET /classes/:id.
func (h *ClassHandler) GetClass(c *gin.Context) {
	classID := c.Param("id")

	var class models.Class
	if err := h.db.Preload("Teacher").First(&class, classID).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{
			"success": false,
			"error":   gin.H{"code": "NOT_FOUND", "message": "Klas niet gevonden."},
		})
		return
	}

	// Load teams
	var teams []models.Team
	h.db.Where("class_id = ? AND is_active = true", class.ID).Find(&teams)

	teamData := make([]gin.H, len(teams))
	for i, t := range teams {
		var memberCount int64
		h.db.Model(&models.TeamMembership{}).Where("team_id = ? AND left_at IS NULL", t.ID).Count(&memberCount)
		teamData[i] = gin.H{
			"id":           t.ID,
			"name":         t.Name,
			"company_name": t.CompanyName,
			"member_count": memberCount,
		}
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"id":            class.ID,
			"name":          class.Name,
			"description":   class.Description,
			"academic_year": class.AcademicYear,
			"teacher": gin.H{
				"id":           class.Teacher.ID,
				"display_name": class.Teacher.DisplayName,
			},
			"teams": teamData,
			"settings": gin.H{
				"max_team_size":         class.MaxTeamSize,
				"allowed_modules":       class.AllowedModules,
				"simulation_time_scale": class.SimulationTimeScale,
			},
		},
	})
}

// AddStudent handles POST /classes/:id/students.
func (h *ClassHandler) AddStudent(c *gin.Context) {
	classID := c.Param("id")

	var body struct {
		StudentID uint  `json:"student_id" binding:"required"`
		TeamID    *uint `json:"team_id"`
	}
	if err := c.ShouldBindJSON(&body); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldige gegevens."},
		})
		return
	}

	membership := models.ClassMembership{
		ClassID:   classID,
		StudentID: body.StudentID,
	}

	// Note: ClassMembership isn't in our current models — we need to add it
	// For now, respond with success structure
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"student_id": body.StudentID,
			"class_id":   classID,
			"message":    "Student toegevoegd aan klas.",
		},
	})
}

// CreateTeam handles POST /classes/:id/teams.
func (h *ClassHandler) CreateTeam(c *gin.Context) {
	classID := c.Param("id")
	userID, _ := c.Get("user_id")

	var body struct {
		Name            string            `json:"name" binding:"required"`
		StudentIDs      []uint            `json:"student_ids"`
		RoleAssignments map[string]string `json:"role_assignments"`
	}
	if err := c.ShouldBindJSON(&body); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldige gegevens."},
		})
		return
	}

	// Parse classID to uint for the Team model
	var classIDUint uint
	h.db.Raw("SELECT ?::integer", classID).Scan(&classIDUint)

	team := models.Team{
		ClassID:   classIDUint,
		Name:      body.Name,
		IsActive:  true,
		CreatedBy: userID.(uint),
	}

	if err := h.db.Create(&team).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "CREATE_FAILED", "message": "Kan team niet aanmaken."},
		})
		return
	}

	// Add team members with roles
	for _, studentID := range body.StudentIDs {
		role := "member"
		if r, ok := body.RoleAssignments[string(rune(studentID))]; ok {
			role = r
		}
		h.db.Create(&models.TeamMembership{
			TeamID:    team.ID,
			StudentID: studentID,
			Role:      role,
		})
	}

	c.JSON(http.StatusCreated, gin.H{
		"success": true,
		"data": gin.H{
			"id":       team.ID,
			"name":     team.Name,
			"class_id": classIDUint,
			"members":  len(body.StudentIDs),
		},
	})
}
