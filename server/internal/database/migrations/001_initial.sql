-- EduERP Initial Migration
-- All tables for the core system

-- Enable RLS support
CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- Schools (Multi-tenant isolation)
CREATE TABLE IF NOT EXISTS schools (
    id                      SERIAL PRIMARY KEY,
    name                    VARCHAR(255) NOT NULL,
    subdomain               VARCHAR(63) UNIQUE,
    oauth_domains           TEXT[] NOT NULL,
    default_language        VARCHAR(5) DEFAULT 'nl-BE',
    allowed_languages       VARCHAR(5)[] DEFAULT ARRAY['nl-BE', 'en-GB', 'fr-BE'],
    streak_enabled          BOOLEAN DEFAULT TRUE,
    friend_system_enabled   BOOLEAN DEFAULT TRUE,
    cross_class_messaging   BOOLEAN DEFAULT FALSE,
    energy_saving_default   BOOLEAN DEFAULT FALSE,
    animation_default       VARCHAR(20) DEFAULT 'full',
    is_active               BOOLEAN DEFAULT TRUE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    deleted_at              TIMESTAMP WITH TIME ZONE
);

CREATE INDEX IF NOT EXISTS idx_schools_oauth_domains ON schools USING GIN(oauth_domains);
CREATE INDEX IF NOT EXISTS idx_schools_active ON schools(is_active) WHERE is_active = TRUE;

-- Users
CREATE TABLE IF NOT EXISTS users (
    id                      SERIAL PRIMARY KEY,
    school_id               INTEGER NOT NULL REFERENCES schools(id) ON DELETE CASCADE,
    email                   VARCHAR(255) NOT NULL,
    oauth_provider          VARCHAR(20) NOT NULL,
    oauth_subject           VARCHAR(255) NOT NULL,
    role                    VARCHAR(20) NOT NULL,
    display_name            VARCHAR(100),
    username                VARCHAR(50),
    avatar_url              VARCHAR(500),
    banner_url              VARCHAR(500),
    bio                     VARCHAR(500),
    preferred_language      VARCHAR(5) DEFAULT 'nl-BE',
    theme_preference        VARCHAR(50) DEFAULT 'system',
    font_size               VARCHAR(10) DEFAULT 'medium',
    animation_preference    VARCHAR(20) DEFAULT 'full',
    energy_saving_mode      BOOLEAN DEFAULT FALSE,
    profile_visibility      VARCHAR(20) DEFAULT 'friends',
    friend_requests_allowed VARCHAR(20) DEFAULT 'class',
    is_active               BOOLEAN DEFAULT TRUE,
    last_login_at           TIMESTAMP WITH TIME ZONE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    deleted_at              TIMESTAMP WITH TIME ZONE,
    CONSTRAINT unique_email_per_school UNIQUE(school_id, email),
    CONSTRAINT unique_username_per_school UNIQUE(school_id, username),
    CONSTRAINT unique_oauth UNIQUE(oauth_provider, oauth_subject)
);

CREATE INDEX IF NOT EXISTS idx_users_school ON users(school_id);
CREATE INDEX IF NOT EXISTS idx_users_role ON users(role);
CREATE INDEX IF NOT EXISTS idx_users_school_role ON users(school_id, role);

-- User Sessions
CREATE TABLE IF NOT EXISTS user_sessions (
    id                      SERIAL PRIMARY KEY,
    user_id                 INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    refresh_token_hash      VARCHAR(64) NOT NULL,
    device_info             VARCHAR(255),
    ip_address              INET,
    expires_at              TIMESTAMP WITH TIME ZONE NOT NULL,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    last_used_at            TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    revoked_at              TIMESTAMP WITH TIME ZONE
);

CREATE INDEX IF NOT EXISTS idx_sessions_user ON user_sessions(user_id);
CREATE INDEX IF NOT EXISTS idx_sessions_token ON user_sessions(refresh_token_hash);

-- Classes
CREATE TABLE IF NOT EXISTS classes (
    id                      SERIAL PRIMARY KEY,
    school_id               INTEGER NOT NULL REFERENCES schools(id) ON DELETE CASCADE,
    name                    VARCHAR(100) NOT NULL,
    description             TEXT,
    academic_year           VARCHAR(9) NOT NULL,
    teacher_id              INTEGER NOT NULL REFERENCES users(id),
    max_team_size           INTEGER DEFAULT 4,
    allowed_modules         VARCHAR(50)[] DEFAULT ARRAY['finance', 'sales', 'inventory', 'hr', 'marketing', 'logistics'],
    simulation_time_scale   VARCHAR(20) DEFAULT 'realtime',
    is_active               BOOLEAN DEFAULT TRUE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_classes_school ON classes(school_id);
CREATE INDEX IF NOT EXISTS idx_classes_teacher ON classes(teacher_id);

-- Class Memberships
CREATE TABLE IF NOT EXISTS class_memberships (
    id                      SERIAL PRIMARY KEY,
    class_id                INTEGER NOT NULL REFERENCES classes(id) ON DELETE CASCADE,
    student_id              INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    joined_at               TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    left_at                 TIMESTAMP WITH TIME ZONE,
    CONSTRAINT unique_student_class UNIQUE(class_id, student_id, left_at)
);

-- Teams
CREATE TABLE IF NOT EXISTS teams (
    id                      SERIAL PRIMARY KEY,
    class_id                INTEGER NOT NULL REFERENCES classes(id) ON DELETE CASCADE,
    name                    VARCHAR(100) NOT NULL,
    company_name            VARCHAR(100),
    current_simulation_id   INTEGER,
    is_active               BOOLEAN DEFAULT TRUE,
    created_by              INTEGER NOT NULL REFERENCES users(id),
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Team Memberships
CREATE TABLE IF NOT EXISTS team_memberships (
    id                      SERIAL PRIMARY KEY,
    team_id                 INTEGER NOT NULL REFERENCES teams(id) ON DELETE CASCADE,
    student_id              INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    role                    VARCHAR(30) NOT NULL,
    joined_at               TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    left_at                 TIMESTAMP WITH TIME ZONE,
    CONSTRAINT unique_student_team UNIQUE(team_id, student_id, left_at)
);

-- Simulation Companies
CREATE TABLE IF NOT EXISTS simulation_companies (
    id                      SERIAL PRIMARY KEY,
    school_id               INTEGER NOT NULL REFERENCES schools(id) ON DELETE CASCADE,
    team_id                 INTEGER REFERENCES teams(id),
    creator_id              INTEGER NOT NULL REFERENCES users(id),
    name                    VARCHAR(100) NOT NULL,
    logo_url                VARCHAR(500),
    industry_template       VARCHAR(50) NOT NULL,
    initial_budget          DECIMAL(15,2) DEFAULT 100000.00,
    currency_code           VARCHAR(3) DEFAULT 'EUR',
    time_scale              VARCHAR(20) DEFAULT 'realtime',
    simulation_speed        INTEGER DEFAULT 1,
    current_simulated_date  DATE,
    simulation_start_date   DATE,
    status                  VARCHAR(20) DEFAULT 'active',
    is_ai_enabled           BOOLEAN DEFAULT FALSE,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_sim_companies_school ON simulation_companies(school_id);
CREATE INDEX IF NOT EXISTS idx_sim_companies_team ON simulation_companies(team_id);

-- Seed data for development
INSERT INTO schools (name, subdomain, oauth_domains, default_language)
VALUES ('Test School', 'testschool', ARRAY['school.be', 'testschool.be'], 'nl-BE')
ON CONFLICT (subdomain) DO NOTHING;
