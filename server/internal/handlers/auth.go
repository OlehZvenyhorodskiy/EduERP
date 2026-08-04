package handlers

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strings"
	"time"

	"github.com/eduerp/server/internal/config"
	"github.com/eduerp/server/internal/models"
	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
	"gorm.io/gorm"
)

// AuthHandler manages authentication endpoints.
type AuthHandler struct {
	db  *gorm.DB
	cfg *config.Config
}

func NewAuthHandler(db *gorm.DB, cfg *config.Config) *AuthHandler {
	return &AuthHandler{db: db, cfg: cfg}
}

// LoginRequest matches POST /auth/login body from the spec.
type LoginRequest struct {
	Provider     string `json:"provider" binding:"required,oneof=google microsoft"`
	Code         string `json:"code" binding:"required"`
	CodeVerifier string `json:"code_verifier" binding:"required"`
	RedirectURI  string `json:"redirect_uri" binding:"required"`
}

// Login handles OAuth code exchange and JWT issuance.
func (h *AuthHandler) Login(c *gin.Context) {
	var req LoginRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"success": false,
			"error": gin.H{
				"code":    "VALIDATION_ERROR",
				"message": "Ongeldige aanvraag. Controleer de invoer.",
			},
		})
		return
	}

	// Exchange the authorization code with the OAuth provider
	// This would call Google/Microsoft token endpoints in production
	email, displayName, oauthSubject, err := exchangeOAuthCode(req.Provider, req.Code, req.CodeVerifier, req.RedirectURI, h.cfg)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"error": gin.H{
				"code":    "OAUTH_FAILED",
				"message": "Authenticatie mislukt. Probeer opnieuw.",
			},
		})
		return
	}

	// Find or validate school by email domain
	domain := extractDomain(email)
	var school models.School
	if err := h.db.Where("? = ANY(oauth_domains) AND is_active = true", domain).First(&school).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"error": gin.H{
				"code":    "INVALID_DOMAIN",
				"message": "Dit e-mailadres is niet toegestaan. Gebruik je schoolaccount.",
				"details": gin.H{
					"provided_domain": domain,
				},
			},
		})
		return
	}

	// Find or create user
	var user models.User
	result := h.db.Where("oauth_provider = ? AND oauth_subject = ?", req.Provider, oauthSubject).First(&user)
	if result.Error != nil {
		// Create new user
		user = models.User{
			SchoolID:      school.ID,
			Email:         email,
			OAuthProvider: req.Provider,
			OAuthSubject:  oauthSubject,
			Role:          "student", // Default role
			DisplayName:   displayName,
			IsActive:      true,
		}
		if err := h.db.Create(&user).Error; err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{
				"success": false,
				"error":   gin.H{"code": "INTERNAL_ERROR", "message": "Kan gebruiker niet aanmaken."},
			})
			return
		}
	}

	if !user.IsActive {
		c.JSON(http.StatusForbidden, gin.H{
			"success": false,
			"error": gin.H{
				"code":    "ACCOUNT_DISABLED",
				"message": "Je account is uitgeschakeld. Neem contact op met je leraar.",
			},
		})
		return
	}

	// Update last login
	now := time.Now()
	h.db.Model(&user).Update("last_login_at", now)

	// Generate JWT access token (15 min expiry)
	accessToken, err := generateJWT(user, school, h.cfg.JWTSecret, 15*time.Minute)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "TOKEN_ERROR", "message": "Kan token niet genereren."},
		})
		return
	}

	// Generate refresh token (30 days)
	refreshToken, err := generateJWT(user, school, h.cfg.JWTSecret, 30*24*time.Hour)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "TOKEN_ERROR", "message": "Kan refresh token niet genereren."},
		})
		return
	}

	// Store refresh token hash in DB
	hash := sha256Hash(refreshToken)
	session := models.UserSession{
		UserID:           user.ID,
		RefreshTokenHash: hash,
		DeviceInfo:       c.GetHeader("User-Agent"),
		IPAddress:        c.ClientIP(),
		ExpiresAt:        time.Now().Add(30 * 24 * time.Hour),
		LastUsedAt:       time.Now(),
	}
	h.db.Create(&session)

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"access_token":  accessToken,
			"refresh_token": refreshToken,
			"expires_in":    900,
			"token_type":    "Bearer",
			"user": gin.H{
				"id":           user.ID,
				"email":        user.Email,
				"display_name": user.DisplayName,
				"role":         user.Role,
				"school": gin.H{
					"id":               school.ID,
					"name":             school.Name,
					"default_language": school.DefaultLanguage,
				},
			},
		},
	})
}

// RefreshToken exchanges a refresh token for a new access token.
func (h *AuthHandler) RefreshToken(c *gin.Context) {
	authHeader := c.GetHeader("Authorization")
	if authHeader == "" {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"error":   gin.H{"code": "MISSING_TOKEN", "message": "Refresh token vereist."},
		})
		return
	}

	tokenStr := authHeader[len("Bearer "):]
	hash := sha256Hash(tokenStr)

	var session models.UserSession
	if err := h.db.Where("refresh_token_hash = ? AND revoked_at IS NULL AND expires_at > ?", hash, time.Now()).First(&session).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"error": gin.H{
				"code":    "TOKEN_EXPIRED",
				"message": "Je sessie is verlopen. Log opnieuw in.",
			},
		})
		return
	}

	var user models.User
	h.db.First(&user, session.UserID)
	var school models.School
	h.db.First(&school, user.SchoolID)

	accessToken, err := generateJWT(user, school, h.cfg.JWTSecret, 15*time.Minute)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{
			"success": false,
			"error":   gin.H{"code": "TOKEN_ERROR", "message": "Kan token niet genereren."},
		})
		return
	}

	h.db.Model(&session).Update("last_used_at", time.Now())

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"access_token": accessToken,
			"expires_in":   900,
			"token_type":   "Bearer",
		},
	})
}

// Logout revokes the current session or all sessions.
func (h *AuthHandler) Logout(c *gin.Context) {
	userID, _ := c.Get("user_id")

	var body struct {
		RevokeAll bool `json:"revoke_all"`
	}
	c.ShouldBindJSON(&body)

	now := time.Now()
	if body.RevokeAll {
		h.db.Model(&models.UserSession{}).Where("user_id = ? AND revoked_at IS NULL", userID).Update("revoked_at", now)
	} else {
		// Revoke current session only — ideally would match on the token hash
		h.db.Model(&models.UserSession{}).Where("user_id = ? AND revoked_at IS NULL", userID).
			Order("last_used_at DESC").Limit(1).Update("revoked_at", now)
	}

	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    gin.H{"message": "Succesvol uitgelogd"},
	})
}

// --- Helper functions ---

func generateJWT(user models.User, school models.School, secret string, duration time.Duration) (string, error) {
	claims := jwt.MapClaims{
		"user_id":   user.ID,
		"school_id": school.ID,
		"role":      user.Role,
		"email":     user.Email,
		"exp":       time.Now().Add(duration).Unix(),
		"iat":       time.Now().Unix(),
	}
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	return token.SignedString([]byte(secret))
}

func sha256Hash(input string) string {
	h := sha256.Sum256([]byte(input))
	return hex.EncodeToString(h[:])
}

func extractDomain(email string) string {
	for i := len(email) - 1; i >= 0; i-- {
		if email[i] == '@' {
			return email[i+1:]
		}
	}
	return ""
}

// exchangeOAuthCode performs the real OAuth 2.0 + PKCE authorization code exchange.
// It calls the provider's token endpoint, then the userinfo endpoint to retrieve
// the user's email, display name, and unique subject identifier.
func exchangeOAuthCode(provider, code, codeVerifier, redirectURI string, cfg *config.Config) (email, displayName, oauthSubject string, err error) {
	switch provider {
	case "google":
		return exchangeGoogleCode(code, codeVerifier, redirectURI, cfg)
	case "microsoft":
		return exchangeMicrosoftCode(code, codeVerifier, redirectURI, cfg)
	default:
		return "", "", "", fmt.Errorf("unsupported oauth provider: %s", provider)
	}
}

// exchangeGoogleCode exchanges an authorization code with Google's OAuth2 token endpoint
// using PKCE (RFC 7636), then fetches the user profile from the userinfo endpoint.
func exchangeGoogleCode(code, codeVerifier, redirectURI string, cfg *config.Config) (email, displayName, oauthSubject string, err error) {
	formData := url.Values{
		"code":          {code},
		"client_id":     {cfg.GoogleClientID},
		"client_secret": {cfg.GoogleClientSecret},
		"redirect_uri":  {redirectURI},
		"grant_type":    {"authorization_code"},
		"code_verifier": {codeVerifier},
	}

	resp, err := http.PostForm("https://oauth2.googleapis.com/token", formData)
	if err != nil {
		return "", "", "", fmt.Errorf("google token request failed: %w", err)
	}
	defer resp.Body.Close()

	body, _ := io.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK {
		return "", "", "", fmt.Errorf("google token endpoint returned %d: %s", resp.StatusCode, string(body))
	}

	var tokenResp struct {
		AccessToken string `json:"access_token"`
		IDToken     string `json:"id_token"`
	}
	if err := json.Unmarshal(body, &tokenResp); err != nil {
		return "", "", "", fmt.Errorf("google token parse error: %w", err)
	}

	userInfoReq, _ := http.NewRequest("GET", "https://www.googleapis.com/oauth2/v3/userinfo", nil)
	userInfoReq.Header.Set("Authorization", "Bearer "+tokenResp.AccessToken)

	client := &http.Client{Timeout: 10 * time.Second}
	userResp, err := client.Do(userInfoReq)
	if err != nil {
		return "", "", "", fmt.Errorf("google userinfo request failed: %w", err)
	}
	defer userResp.Body.Close()

	userBody, _ := io.ReadAll(userResp.Body)
	if userResp.StatusCode != http.StatusOK {
		return "", "", "", fmt.Errorf("google userinfo returned %d: %s", userResp.StatusCode, string(userBody))
	}

	var userInfo struct {
		Sub   string `json:"sub"`
		Email string `json:"email"`
		Name  string `json:"name"`
	}
	if err := json.Unmarshal(userBody, &userInfo); err != nil {
		return "", "", "", fmt.Errorf("google userinfo parse error: %w", err)
	}

	if userInfo.Email == "" || userInfo.Sub == "" {
		return "", "", "", fmt.Errorf("google userinfo missing required fields")
	}

	return userInfo.Email, userInfo.Name, "google:" + userInfo.Sub, nil
}

// exchangeMicrosoftCode exchanges an authorization code with Microsoft's OAuth2 token endpoint
// using PKCE (RFC 7636), then fetches the user profile from the Microsoft Graph /me endpoint.
func exchangeMicrosoftCode(code, codeVerifier, redirectURI string, cfg *config.Config) (email, displayName, oauthSubject string, err error) {
	tokenURL := fmt.Sprintf(
		"https://login.microsoftonline.com/%s/oauth2/v2.0/token",
		cfg.MicrosoftTenantID,
	)

	formData := url.Values{
		"code":          {code},
		"client_id":     {cfg.MicrosoftClientID},
		"client_secret": {cfg.MicrosoftClientSecret},
		"redirect_uri":  {redirectURI},
		"grant_type":    {"authorization_code"},
		"code_verifier": {codeVerifier},
		"scope":         {"openid email profile User.Read"},
	}

	resp, err := http.Post(tokenURL, "application/x-www-form-urlencoded", strings.NewReader(formData.Encode()))
	if err != nil {
		return "", "", "", fmt.Errorf("microsoft token request failed: %w", err)
	}
	defer resp.Body.Close()

	body, _ := io.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK {
		return "", "", "", fmt.Errorf("microsoft token endpoint returned %d: %s", resp.StatusCode, string(body))
	}

	var tokenResp struct {
		AccessToken string `json:"access_token"`
	}
	if err := json.Unmarshal(body, &tokenResp); err != nil {
		return "", "", "", fmt.Errorf("microsoft token parse error: %w", err)
	}

	graphReq, _ := http.NewRequest("GET", "https://graph.microsoft.com/v1.0/me", nil)
	graphReq.Header.Set("Authorization", "Bearer "+tokenResp.AccessToken)

	client := &http.Client{Timeout: 10 * time.Second}
	graphResp, err := client.Do(graphReq)
	if err != nil {
		return "", "", "", fmt.Errorf("microsoft graph request failed: %w", err)
	}
	defer graphResp.Body.Close()

	graphBody, _ := io.ReadAll(graphResp.Body)
	if graphResp.StatusCode != http.StatusOK {
		return "", "", "", fmt.Errorf("microsoft graph returned %d: %s", graphResp.StatusCode, string(graphBody))
	}

	var userInfo struct {
		ID                string `json:"id"`
		DisplayName       string `json:"displayName"`
		Mail              string `json:"mail"`
		UserPrincipalName string `json:"userPrincipalName"`
	}
	if err := json.Unmarshal(graphBody, &userInfo); err != nil {
		return "", "", "", fmt.Errorf("microsoft graph parse error: %w", err)
	}

	// Microsoft sometimes returns email in userPrincipalName when mail is empty
	userEmail := userInfo.Mail
	if userEmail == "" {
		userEmail = userInfo.UserPrincipalName
	}

	if userEmail == "" || userInfo.ID == "" {
		return "", "", "", fmt.Errorf("microsoft graph missing required fields")
	}

	return userEmail, userInfo.DisplayName, "microsoft:" + userInfo.ID, nil
}