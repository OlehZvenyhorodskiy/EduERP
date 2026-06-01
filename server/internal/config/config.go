package config

import "os"

// Config holds all application configuration values.
type Config struct {
	Port        string
	DatabaseURL string
	JWTSecret   string

	GoogleClientID     string
	GoogleClientSecret string

	MicrosoftClientID     string
	MicrosoftClientSecret string
	MicrosoftTenantID     string

	CloudStorageBucket string
}

// Load reads configuration from environment variables with sensible defaults.
func Load() *Config {
	return &Config{
		Port:        getEnv("PORT", "8080"),
		DatabaseURL: getEnv("DATABASE_URL", "postgres://eduerp:eduerp@localhost:5432/eduerp?sslmode=disable"),
		JWTSecret:   getEnv("JWT_SECRET", "dev-secret-change-in-production"),

		GoogleClientID:     getEnv("GOOGLE_CLIENT_ID", ""),
		GoogleClientSecret: getEnv("GOOGLE_CLIENT_SECRET", ""),

		MicrosoftClientID:     getEnv("MICROSOFT_CLIENT_ID", ""),
		MicrosoftClientSecret: getEnv("MICROSOFT_CLIENT_SECRET", ""),
		MicrosoftTenantID:     getEnv("MICROSOFT_TENANT_ID", ""),

		CloudStorageBucket: getEnv("CLOUD_STORAGE_BUCKET", "eduerp-files"),
	}
}

func getEnv(key, fallback string) string {
	if val, ok := os.LookupEnv(key); ok {
		return val
	}
	return fallback
}
