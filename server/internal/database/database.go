package database

import (
	"fmt"
	"log"

	"github.com/eduerp/server/internal/models"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
	"gorm.io/gorm/logger"
)

// Connect establishes a connection to PostgreSQL using GORM.
func Connect(dsn string) (*gorm.DB, error) {
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{
		Logger: logger.Default.LogMode(logger.Info),
	})
	if err != nil {
		return nil, fmt.Errorf("failed to open database: %w", err)
	}

	sqlDB, err := db.DB()
	if err != nil {
		return nil, fmt.Errorf("failed to get underlying sql.DB: %w", err)
	}

	// Connection pool settings per spec
	sqlDB.SetMaxOpenConns(25)
	sqlDB.SetMaxIdleConns(5)

	log.Println("Database connection established")
	return db, nil
}

// RunMigrations auto-migrates all GORM models.
func RunMigrations(db *gorm.DB) error {
	log.Println("Running database migrations...")
	return db.AutoMigrate(
		&models.School{},
		&models.User{},
		&models.UserSession{},
		&models.Class{},
		&models.Team{},
		&models.TeamMembership{},
		&models.SimulationCompany{},
	)
}
