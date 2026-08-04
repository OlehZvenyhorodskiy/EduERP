package handlers

import (
	"net/http"

	"github.com/eduerp/server/internal/models"
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// CompanyHandler manages simulation company endpoints.
type CompanyHandler struct {
	db *gorm.DB
}

func NewCompanyHandler(db *gorm.DB) *CompanyHandler {
	return &CompanyHandler{db: db}
}

// CreateCompany handles POST /companies.
func (h *CompanyHandler) CreateCompany(c *gin.Context) {
	userID, _ := c.Get("user_id")
	schoolID, _ := c.Get("school_id")

	var body struct {
		Name             string  `json:"name" binding:"required"`
		IndustryTemplate string  `json:"industry_template" binding:"required"`
		InitialBudget    float64 `json:"initial_budget"`
		TeamID           *uint   `json:"team_id"`
	}

	if err := c.ShouldBindJSON(&body); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldige gegevens."},
		})
		return
	}

	budget := body.InitialBudget
	if budget == 0 {
		budget = 100000.00
	}

	company := models.SimulationCompany{
		SchoolID:         schoolID.(uint),
		TeamID:           body.TeamID,
		CreatorID:        userID.(uint),
		Name:             body.Name,
		IndustryTemplate: body.IndustryTemplate,
		InitialBudget:    budget,
		CurrencyCode:     "EUR",
		Status:           "active",
	}

	if err := h.db.Create(&company).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "CREATE_FAILED", "message": "Kan bedrijf niet aanmaken."},
		})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"success": true,
		"data": gin.H{
			"id":                company.ID,
			"name":              company.Name,
			"industry_template": company.IndustryTemplate,
			"initial_budget":    company.InitialBudget,
			"status":            company.Status,
			"created_at":        company.CreatedAt,
		},
	})
}

// GetCompany handles GET /companies/:id.
func (h *CompanyHandler) GetCompany(c *gin.Context) {
	companyID := c.Param("id")

	var company models.SimulationCompany
	if err := h.db.First(&company, companyID).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{
			"success": false,
			"error":   gin.H{"code": "NOT_FOUND", "message": "Bedrijf niet gevonden."},
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    company,
	})
}

// GetKPIs handles GET /companies/:id/kpis.
func (h *CompanyHandler) GetKPIs(c *gin.Context) {
	companyID := c.Param("id")

	// Return stub KPI data — will be populated by the simulation engine
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"company_id": companyID,
			"kpis": gin.H{
				"revenue":               0,
				"expenses":              0,
				"net_profit":            0,
				"profit_margin":         0,
				"cash_on_hand":          100000.00,
				"customer_satisfaction": 0.5,
				"employee_satisfaction": 0.5,
				"market_share":          0,
			},
		},
	})
}