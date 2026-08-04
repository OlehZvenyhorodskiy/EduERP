package middleware

import (
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
)

// ipRateBucket tracks request counts per IP within a rolling time window.
type ipRateBucket struct {
	count     int
	windowEnd time.Time
}

var (
	rateMu      sync.Mutex
	rateBuckets = make(map[string]*ipRateBucket)
)

// CORS middleware for cross-origin requests from the desktop client.
// Only allows requests from localhost (desktop app) and the EduERP web domain.
func CORS() gin.HandlerFunc {
	allowedOrigins := map[string]bool{
		"http://localhost:8765":    true, // Qt OAuth local callback server
		"http://localhost:3000":    true, // Dev web client
		"https://app.eduerp.be":   true, // Production web client
	}

	return func(c *gin.Context) {
		origin := c.Request.Header.Get("Origin")
		if allowedOrigins[origin] {
			c.Writer.Header().Set("Access-Control-Allow-Origin", origin)
		} else if origin == "" {
			// Direct requests (e.g., from curl, Qt network manager) — allow
			c.Writer.Header().Set("Access-Control-Allow-Origin", "*")
		}
		c.Writer.Header().Set("Access-Control-Allow-Methods", "GET, POST, PATCH, DELETE, OPTIONS")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Authorization, Content-Type, X-Client-Version")
		c.Writer.Header().Set("Access-Control-Max-Age", "86400")
		c.Writer.Header().Set("Vary", "Origin")

		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(http.StatusNoContent)
			return
		}
		c.Next()
	}
}

// SecurityHeaders sets OWASP-recommended security response headers.
// Applied globally to every response.
func SecurityHeaders() gin.HandlerFunc {
	return func(c *gin.Context) {
		// Prevent clickjacking
		c.Header("X-Frame-Options", "DENY")
		// Block MIME sniffing
		c.Header("X-Content-Type-Options", "nosniff")
		// Enable XSS filter in older browsers
		c.Header("X-XSS-Protection", "1; mode=block")
		// Enforce HTTPS for 1 year (production only)
		c.Header("Strict-Transport-Security", "max-age=31536000; includeSubDomains")
		// Restrictive CSP: only self + API calls
		c.Header("Content-Security-Policy", "default-src 'self'; connect-src 'self' https://api.eduerp.be")
		// Don't leak referrer info
		c.Header("Referrer-Policy", "strict-origin-when-cross-origin")
		// Disable FLoC / interest cohort tracking
		c.Header("Permissions-Policy", "interest-cohort=()")
		c.Next()
	}
}

// RateLimit limits requests to maxReq per window per client IP.
// Intended for auth endpoints to prevent brute-force password/token attacks.
// Uses an in-memory sliding window — replace with Redis for multi-instance deployments.
func RateLimit(maxReq int, window time.Duration) gin.HandlerFunc {
	return func(c *gin.Context) {
		ip := c.ClientIP()

		rateMu.Lock()
		bucket, exists := rateBuckets[ip]
		now := time.Now()

		if !exists || now.After(bucket.windowEnd) {
			// New window
			rateBuckets[ip] = &ipRateBucket{count: 1, windowEnd: now.Add(window)}
			rateMu.Unlock()
			c.Next()
			return
		}

		bucket.count++
		if bucket.count > maxReq {
			rateMu.Unlock()
			c.Header("Retry-After", "60")
			c.AbortWithStatusJSON(http.StatusTooManyRequests, gin.H{
				"success": false,
				"error": gin.H{
					"code":    "RATE_LIMITED",
					"message": "Te veel aanvragen. Probeer het over 60 seconden opnieuw.",
				},
			})
			return
		}
		rateMu.Unlock()
		c.Next()
	}
}

// RequestLogger logs incoming requests.
func RequestLogger() gin.HandlerFunc {
	return gin.Logger()
}

// JWTClaims represents the claims embedded in the JWT access token.
type JWTClaims struct {
	UserID   uint   `json:"user_id"`
	SchoolID uint   `json:"school_id"`
	Role     string `json:"role"`
	Email    string `json:"email"`
	jwt.RegisteredClaims
}

// RequireAuth validates the JWT Bearer token.
func RequireAuth(jwtSecret string) gin.HandlerFunc {
	return func(c *gin.Context) {
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" || !strings.HasPrefix(authHeader, "Bearer ") {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{
				"success": false,
				"error": gin.H{
					"code":    "MISSING_TOKEN",
					"message": "Authenticatie vereist. Log opnieuw in.",
				},
			})
			return
		}

		tokenString := strings.TrimPrefix(authHeader, "Bearer ")
		claims := &JWTClaims{}

		token, err := jwt.ParseWithClaims(tokenString, claims, func(t *jwt.Token) (interface{}, error) {
			return []byte(jwtSecret), nil
		})

		if err != nil || !token.Valid {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{
				"success": false,
				"error": gin.H{
					"code":    "INVALID_TOKEN",
					"message": "Je sessie is ongeldig of verlopen. Log opnieuw in.",
				},
			})
			return
		}

		// Store claims in context for downstream handlers
		c.Set("user_id", claims.UserID)
		c.Set("school_id", claims.SchoolID)
		c.Set("role", claims.Role)
		c.Set("email", claims.Email)
		c.Next()
	}
}

// RequireRole checks if the authenticated user has one of the required roles.
func RequireRole(roles ...string) gin.HandlerFunc {
	return func(c *gin.Context) {
		userRole, exists := c.Get("role")
		if !exists {
			c.AbortWithStatusJSON(http.StatusForbidden, gin.H{
				"success": false,
				"error": gin.H{
					"code":    "INSUFFICIENT_PERMISSIONS",
					"message": "Je hebt geen toestemming voor deze actie.",
				},
			})
			return
		}

		roleStr, ok := userRole.(string)
		if !ok {
			c.AbortWithStatus(http.StatusInternalServerError)
			return
		}

		for _, r := range roles {
			if roleStr == r {
				c.Next()
				return
			}
		}

		c.AbortWithStatusJSON(http.StatusForbidden, gin.H{
			"success": false,
			"error": gin.H{
				"code":    "INSUFFICIENT_PERMISSIONS",
				"message": "Je hebt geen toestemming voor deze actie.",
			},
		})
	}
}