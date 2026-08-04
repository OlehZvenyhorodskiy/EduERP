package handlers

import (
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

// SimulationHandler manages simulation state endpoints.
type SimulationHandler struct {
	DB *gorm.DB
}

// KPISnapshot represents a point-in-time snapshot of company KPIs.
type KPISnapshot struct {
	ID                   uint      `gorm:"primaryKey" json:"id"`
	CompanyID            uint      `json:"company_id"`
	SnapshotDate         string    `json:"snapshot_date"`
	Revenue              float64   `json:"revenue"`
	Expenses             float64   `json:"expenses"`
	NetProfit            float64   `json:"net_profit"`
	CashOnHand           float64   `json:"cash_on_hand"`
	EmployeeCount        int       `json:"employee_count"`
	CustomerSatisfaction float64   `json:"customer_satisfaction"`
	EmployeeSatisfaction float64   `json:"employee_satisfaction"`
	BrandAwareness       float64   `json:"brand_awareness"`
	InventoryValue       float64   `json:"inventory_value"`
	ActiveShipments      int       `json:"active_shipments"`
	CreatedAt            time.Time `json:"created_at"`
}

// SaveKPISnapshot stores a KPI snapshot for historical tracking.
func (h *SimulationHandler) SaveKPISnapshot(c *gin.Context) {
	companyID, err := strconv.ParseUint(c.Param("id"), 10, 32)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldig bedrijf-ID"},
		})
		return
	}

	var snapshot KPISnapshot
	if err := c.ShouldBindJSON(&snapshot); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldige KPI-gegevens"},
		})
		return
	}

	snapshot.CompanyID = uint(companyID)
	if err := h.DB.Create(&snapshot).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "DB_ERROR", "message": "KPI-snapshot kon niet worden opgeslagen"},
		})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"success": true,
		"data":    snapshot,
	})
}

// GetKPIHistory returns historical KPI snapshots for a company.
func (h *SimulationHandler) GetKPIHistory(c *gin.Context) {
	companyID, err := strconv.ParseUint(c.Param("id"), 10, 32)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error":   gin.H{"code": "VALIDATION_ERROR", "message": "Ongeldig bedrijf-ID"},
		})
		return
	}

	limit := 30
	if l, err := strconv.Atoi(c.DefaultQuery("limit", "30")); err == nil && l > 0 && l <= 365 {
		limit = l
	}

	var snapshots []KPISnapshot
	h.DB.Where("company_id = ?", companyID).
		Order("snapshot_date DESC").
		Limit(limit).
		Find(&snapshots)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"snapshots": snapshots,
			"count":     len(snapshots),
		},
	})
}

// GetLeaderboard returns the top companies by net profit within a school.
func (h *SimulationHandler) GetLeaderboard(c *gin.Context) {
	schoolID := c.GetUint("school_id")

	type LeaderboardEntry struct {
		CompanyID   uint    `json:"company_id"`
		CompanyName string  `json:"company_name"`
		TeamName    string  `json:"team_name"`
		NetProfit   float64 `json:"net_profit"`
		Revenue     float64 `json:"revenue"`
		Rank        int     `json:"rank"`
	}

	// Get latest KPI snapshot per company, joined with company data
	var entries []LeaderboardEntry
	h.DB.Raw(`
		SELECT DISTINCT ON (k.company_id)
			k.company_id, sc.name as company_name, t.name as team_name,
			k.net_profit, k.revenue
		FROM kpi_snapshots k
		JOIN simulation_companies sc ON sc.id = k.company_id
		LEFT JOIN teams t ON t.id = sc.team_id
		WHERE sc.school_id = ?
		ORDER BY k.company_id, k.snapshot_date DESC
	`, schoolID).Scan(&entries)

	// Sort by net profit (descending) and assign ranks
	for i := range entries {
		entries[i].Rank = i + 1
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"leaderboard": entries,
			"school_id":   schoolID,
		},
	})
}