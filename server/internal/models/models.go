package models

import (
	"time"

	"github.com/lib/pq"
	"gorm.io/gorm"
)

// School represents a tenant in the multi-tenant system.
type School struct {
	gorm.Model
	Name                string         `gorm:"size:255;not null" json:"name"`
	Subdomain           string         `gorm:"size:63;uniqueIndex" json:"subdomain"`
	OAuthDomains        pq.StringArray `gorm:"type:text[]" json:"oauth_domains"`
	DefaultLanguage     string         `gorm:"size:5;default:'nl-BE'" json:"default_language"`
	AllowedLanguages    pq.StringArray `gorm:"type:varchar(5)[];default:'{nl-BE,en-GB,fr-BE}'" json:"allowed_languages"`
	StreakEnabled        bool           `gorm:"default:true" json:"streak_enabled"`
	FriendSystemEnabled bool           `gorm:"default:true" json:"friend_system_enabled"`
	CrossClassMessaging bool           `gorm:"default:false" json:"cross_class_messaging"`
	EnergySavingDefault bool           `gorm:"default:false" json:"energy_saving_default"`
	AnimationDefault    string         `gorm:"size:20;default:'full'" json:"animation_default"`
	IsActive            bool           `gorm:"default:true" json:"is_active"`
	DeletedAt           gorm.DeletedAt `json:"deleted_at,omitempty"`
}

// User represents all user accounts (students, teachers, admins).
type User struct {
	gorm.Model
	SchoolID         uint      `gorm:"not null;index" json:"school_id"`
	School           School    `json:"school,omitempty"`
	Email            string    `gorm:"size:255;not null" json:"email"`
	OAuthProvider    string    `gorm:"size:20;not null" json:"oauth_provider"`
	OAuthSubject     string    `gorm:"size:255;not null" json:"oauth_subject"`
	Role             string    `gorm:"size:20;not null" json:"role"`
	DisplayName      string    `gorm:"size:100" json:"display_name"`
	Username         string    `gorm:"size:50" json:"username"`
	AvatarURL        string    `gorm:"size:500" json:"avatar_url"`
	BannerURL        string    `gorm:"size:500" json:"banner_url"`
	Bio              string    `gorm:"size:500" json:"bio"`
	PreferredLang    string    `gorm:"size:5;default:'nl-BE'" json:"preferred_language"`
	ThemePreference  string    `gorm:"size:50;default:'system'" json:"theme_preference"`
	FontSize         string    `gorm:"size:10;default:'medium'" json:"font_size"`
	AnimationPref    string    `gorm:"size:20;default:'full'" json:"animation_preference"`
	EnergySaving     bool      `gorm:"default:false" json:"energy_saving_mode"`
	ProfileVisibility string   `gorm:"size:20;default:'friends'" json:"profile_visibility"`
	FriendRequests    string   `gorm:"size:20;default:'class'" json:"friend_requests_allowed"`
	IsActive         bool      `gorm:"default:true" json:"is_active"`
	LastLoginAt      *time.Time `json:"last_login_at,omitempty"`
	DeletedAt        gorm.DeletedAt `json:"deleted_at,omitempty"`
}

// UserSession tracks JWT refresh tokens for security.
type UserSession struct {
	gorm.Model
	UserID           uint      `gorm:"not null;index" json:"user_id"`
	RefreshTokenHash string    `gorm:"size:64;not null;index" json:"-"`
	DeviceInfo       string    `gorm:"size:255" json:"device_info"`
	IPAddress        string    `gorm:"size:45" json:"ip_address"`
	ExpiresAt        time.Time `gorm:"not null" json:"expires_at"`
	LastUsedAt       time.Time `json:"last_used_at"`
	RevokedAt        *time.Time `json:"revoked_at,omitempty"`
}

// Class represents a school class/group.
type Class struct {
	gorm.Model
	SchoolID            uint           `gorm:"not null;index" json:"school_id"`
	Name                string         `gorm:"size:100;not null" json:"name"`
	Description         string         `json:"description"`
	AcademicYear        string         `gorm:"size:9;not null" json:"academic_year"`
	TeacherID           uint           `gorm:"not null;index" json:"teacher_id"`
	Teacher             User           `gorm:"foreignKey:TeacherID" json:"teacher,omitempty"`
	MaxTeamSize         int            `gorm:"default:4" json:"max_team_size"`
	AllowedModules      pq.StringArray `gorm:"type:varchar(50)[]" json:"allowed_modules"`
	SimulationTimeScale string         `gorm:"size:20;default:'realtime'" json:"simulation_time_scale"`
	IsActive            bool           `gorm:"default:true" json:"is_active"`
}

// Team represents a student team within a class.
type Team struct {
	gorm.Model
	ClassID             uint   `gorm:"not null;index" json:"class_id"`
	Name                string `gorm:"size:100;not null" json:"name"`
	CompanyName         string `gorm:"size:100" json:"company_name"`
	CurrentSimulationID *uint  `json:"current_simulation_id"`
	IsActive            bool   `gorm:"default:true" json:"is_active"`
	CreatedBy           uint   `gorm:"not null" json:"created_by"`
}

// TeamMembership links students to teams with simulation roles.
type TeamMembership struct {
	gorm.Model
	TeamID    uint       `gorm:"not null;index" json:"team_id"`
	StudentID uint       `gorm:"not null;index" json:"student_id"`
	Student   User       `gorm:"foreignKey:StudentID" json:"student,omitempty"`
	Role      string     `gorm:"size:30;not null" json:"role"`
	LeftAt    *time.Time `json:"left_at,omitempty"`
}

// SimulationCompany represents a simulated company.
type SimulationCompany struct {
	gorm.Model
	SchoolID            uint    `gorm:"not null;index" json:"school_id"`
	TeamID              *uint   `gorm:"index" json:"team_id"`
	CreatorID           uint    `gorm:"not null;index" json:"creator_id"`
	Name                string  `gorm:"size:100;not null" json:"name"`
	LogoURL             string  `gorm:"size:500" json:"logo_url"`
	IndustryTemplate    string  `gorm:"size:50;not null" json:"industry_template"`
	InitialBudget       float64 `gorm:"type:decimal(15,2);default:100000.00" json:"initial_budget"`
	CurrencyCode        string  `gorm:"size:3;default:'EUR'" json:"currency_code"`
	TimeScale           string  `gorm:"size:20;default:'realtime'" json:"time_scale"`
	SimulationSpeed     int     `gorm:"default:1" json:"simulation_speed"`
	CurrentSimDate      string  `gorm:"type:date" json:"current_simulated_date"`
	SimulationStartDate string  `gorm:"type:date" json:"simulation_start_date"`
	Status              string  `gorm:"size:20;default:'active'" json:"status"`
	IsAIEnabled         bool    `gorm:"default:false" json:"is_ai_enabled"`
}