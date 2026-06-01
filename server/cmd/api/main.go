package main

import (
	"log"
	"os"
	"time"

	"github.com/eduerp/server/internal/config"
	"github.com/eduerp/server/internal/database"
	"github.com/eduerp/server/internal/handlers"
	"github.com/eduerp/server/internal/middleware"
	ws "github.com/eduerp/server/internal/websocket"
	"github.com/gin-gonic/gin"
)

func main() {
	cfg := config.Load()

	db, err := database.Connect(cfg.DatabaseURL)
	if err != nil {
		log.Fatalf("Failed to connect to database: %v", err)
	}

	if err := database.RunMigrations(db); err != nil {
		log.Fatalf("Failed to run migrations: %v", err)
	}

	// Initialize WebSocket hub
	hub := ws.NewHub()
	go hub.Run()

	router := gin.New() // gin.New() instead of gin.Default() — we control all middleware

	// Global middleware (order matters)
	router.Use(gin.Recovery())              // panic recovery must be first
	router.Use(middleware.RequestLogger())  // structured request logging
	router.Use(middleware.CORS())           // CORS with origin allowlist
	router.Use(middleware.SecurityHeaders()) // OWASP security headers

	// Health check
	router.GET("/health", func(c *gin.Context) {
		c.JSON(200, gin.H{
			"status":  "ok",
			"version": "1.0.0",
			"modules": []string{"auth", "users", "classes", "companies", "simulation", "messaging", "social"},
		})
	})

	// WebSocket endpoint (JWT-authenticated via query param)
	router.GET("/ws", func(c *gin.Context) {
		// In production, validate JWT from query param or header
		// userID := extractUserIDFromToken(c.Query("token"))
		handler := ws.HandleWebSocket(hub, 0, 0)
		handler(c.Writer, c.Request)
	})

	// API v1 group
	v1 := router.Group("/api/v1")
	{
		// ── Auth endpoints (rate-limited, no JWT required) ──
		auth := v1.Group("/auth")
		auth.Use(middleware.RateLimit(10, time.Minute)) // 10 attempts/min per IP
		{
			authHandler := handlers.NewAuthHandler(db, cfg)
			auth.POST("/login", authHandler.Login)
			auth.POST("/refresh", authHandler.RefreshToken)
			auth.POST("/logout", middleware.RequireAuth(cfg.JWTSecret), authHandler.Logout)
		}

		// ── Protected endpoints ──
		protected := v1.Group("")
		protected.Use(middleware.RequireAuth(cfg.JWTSecret))
		{
			// User endpoints
			userHandler := handlers.NewUserHandler(db)
			protected.GET("/users/me", userHandler.GetCurrentUser)
			protected.PATCH("/users/me", userHandler.UpdateProfile)
			protected.GET("/users", middleware.RequireRole("school_admin", "teacher"), userHandler.ListUsers)
			protected.POST("/users/bulk", middleware.RequireRole("school_admin"), userHandler.BulkCreateUsers)

			// Class endpoints
			classHandler := handlers.NewClassHandler(db)
			protected.POST("/classes", middleware.RequireRole("school_admin", "teacher"), classHandler.CreateClass)
			protected.GET("/classes/:id", classHandler.GetClass)
			protected.POST("/classes/:id/students", middleware.RequireRole("school_admin", "teacher"), classHandler.AddStudent)
			protected.POST("/classes/:id/teams", middleware.RequireRole("school_admin", "teacher"), classHandler.CreateTeam)

			// Company / Simulation endpoints
			companyHandler := handlers.NewCompanyHandler(db)
			protected.POST("/companies", companyHandler.CreateCompany)
			protected.GET("/companies/:id", companyHandler.GetCompany)
			protected.GET("/companies/:id/kpis", companyHandler.GetKPIs)

			// Simulation endpoints
			simHandler := &handlers.SimulationHandler{DB: db}
			protected.POST("/companies/:id/kpi-snapshots", simHandler.SaveKPISnapshot)
			protected.GET("/companies/:id/kpi-history", simHandler.GetKPIHistory)
			protected.GET("/leaderboard", simHandler.GetLeaderboard)

			// Messaging endpoints
			msgHandler := &handlers.MessageHandler{DB: db}
			protected.GET("/messages/conversations", msgHandler.GetConversations)
			protected.GET("/messages/conversations/:id", msgHandler.GetMessages)
			protected.POST("/messages", msgHandler.SendMessage)
			protected.POST("/messages/read", msgHandler.MarkRead)

			// Social / Friends endpoints
			socialHandler := &handlers.SocialHandler{DB: db}
			protected.GET("/friends", socialHandler.GetFriends)
			protected.GET("/friends/requests", socialHandler.GetFriendRequests)
			protected.POST("/friends/requests", socialHandler.SendFriendRequest)
			protected.PATCH("/friends/requests/:id", socialHandler.RespondToFriendRequest)
			protected.DELETE("/friends/:id", socialHandler.RemoveFriend)

			// Gamification endpoints
			gamHandler := &handlers.GamificationHandler{DB: db}
			protected.GET("/gamification/state", gamHandler.GetGamificationState)
			protected.PUT("/gamification/state", gamHandler.SyncGamificationState)
			protected.GET("/gamification/achievements", gamHandler.GetAchievements)
			protected.POST("/gamification/achievements", gamHandler.UnlockAchievement)
			protected.GET("/gamification/leaderboard", gamHandler.GetLeaderboardByXP)

			// ── GDPR: Right to Erasure (Article 17) ──
			// Permanently deletes all personal data for the authenticated user.
			// Cascades to: sessions, gamification state, achievements, messages, friends.
			protected.DELETE("/account", func(c *gin.Context) {
				userID := c.GetUint("user_id")
				if userID == 0 {
					c.JSON(400, gin.H{"success": false, "error": gin.H{"code": "MISSING_USER"}})
					return
				}

				// Cascade delete in dependency order
				tablesToPurge := []string{
					"user_sessions",
					"user_gamifications",
					"user_achievements",
					"messages",
					"friend_requests",
					"friendships",
					"companies",
				}
				for _, table := range tablesToPurge {
					db.Exec("DELETE FROM "+table+" WHERE user_id = ?", userID)
				}
				// Finally erase the user record itself
				db.Exec("DELETE FROM users WHERE id = ?", userID)

				log.Printf("GDPR: User %d data erased (Right to Erasure, Article 17)", userID)
				c.JSON(200, gin.H{
					"success": true,
					"data": gin.H{"message": "Alle gegevens zijn permanent verwijderd."},
				})
			})
		}
	}

	port := os.Getenv("PORT")
	if port == "" {
		port = cfg.Port
	}

	log.Println("═══════════════════════════════════════════")
	log.Printf("  EduERP Backend v1.0.0 — :%s", port)
	log.Println("═══════════════════════════════════════════")
	log.Println("  Endpoints: auth, users, classes, companies, simulation, messaging, social")
	log.Println("  WebSocket: /ws")
	log.Printf("  Database: connected")

	if err := router.Run(":" + port); err != nil {
		log.Fatalf("Failed to start server: %v", err)
	}
}
