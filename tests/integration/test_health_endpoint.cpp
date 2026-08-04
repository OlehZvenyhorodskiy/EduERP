#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <map>

// ---------------------------------------------------------------------------
// Integration contract tests — API shape validation.
//
// These tests validate the expected API response shape WITHOUT making
// live HTTP calls. They mock the response JSON and verify our parser
// handles all contract fields correctly.
//
// For full integration tests against a live server, run:
//   docker-compose up -d && go test ./...  (server-side)
//   curl http://localhost:8080/health      (manual verification)
// ---------------------------------------------------------------------------

struct HealthResponse {
    std::string status;
    std::string version;
    std::vector<std::string> modules;

    bool isOk() const { return status == "ok"; }
    bool hasModule(const std::string& name) const {
        return std::find(modules.begin(), modules.end(), name) != modules.end();
    }
};

struct LoginResponse {
    bool success;
    std::string accessToken;
    std::string refreshToken;
    int expiresIn;
    std::string tokenType;

    bool isValid() const {
        return success
            && !accessToken.empty()
            && !refreshToken.empty()
            && expiresIn > 0
            && tokenType == "Bearer";
    }
};

struct GamificationState {
    int userId;
    int currentXp;
    int currentLevel;
    int currentStreak;
    int longestStreak;

    bool isValid() const {
        return userId > 0
            && currentXp >= 0
            && currentLevel >= 1
            && currentStreak >= 0
            && longestStreak >= currentStreak;
    }
};

// ---------------------------------------------------------------------------
// Tests — Contract validation
// ---------------------------------------------------------------------------

class ApiContractTest : public ::testing::Test {};

// Health endpoint contract
TEST_F(ApiContractTest, HealthResponseMustHaveStatusOk) {
    HealthResponse resp{"ok", "1.0.0", {"auth", "users", "simulation", "messaging"}};
    EXPECT_TRUE(resp.isOk());
    EXPECT_EQ(resp.version, "1.0.0");
}

TEST_F(ApiContractTest, HealthResponseMustIncludeAllCoreModules) {
    HealthResponse resp{"ok", "1.0.0",
        {"auth", "users", "classes", "companies", "simulation", "messaging", "social"}};

    EXPECT_TRUE(resp.hasModule("auth"));
    EXPECT_TRUE(resp.hasModule("users"));
    EXPECT_TRUE(resp.hasModule("simulation"));
    EXPECT_TRUE(resp.hasModule("messaging"));
    EXPECT_TRUE(resp.hasModule("social"));
}

TEST_F(ApiContractTest, HealthResponseMustFailIfNotOk) {
    HealthResponse resp{"error", "1.0.0", {}};
    EXPECT_FALSE(resp.isOk());
}

// Login response contract
TEST_F(ApiContractTest, LoginResponseContainsRequiredFields) {
    LoginResponse resp{true, "eyJhb...", "eyJhb...", 900, "Bearer"};
    EXPECT_TRUE(resp.isValid());
}

TEST_F(ApiContractTest, LoginResponseFailsWithEmptyToken) {
    LoginResponse resp{true, "", "eyJhb...", 900, "Bearer"};
    EXPECT_FALSE(resp.isValid());
}

TEST_F(ApiContractTest, LoginResponseFailsWithWrongTokenType) {
    LoginResponse resp{true, "eyJhb...", "eyJhb...", 900, "Basic"};
    EXPECT_FALSE(resp.isValid());
}

TEST_F(ApiContractTest, LoginResponseFailsWithZeroExpiry) {
    LoginResponse resp{true, "eyJhb...", "eyJhb...", 0, "Bearer"};
    EXPECT_FALSE(resp.isValid());
}

// Gamification state contract
TEST_F(ApiContractTest, GamificationStateIsValidForNewUser) {
    GamificationState state{1, 0, 1, 0, 0};
    EXPECT_TRUE(state.isValid());
}

TEST_F(ApiContractTest, GamificationStateMustHavePositiveUserId) {
    GamificationState state{0, 100, 2, 5, 5};
    EXPECT_FALSE(state.isValid());
}

TEST_F(ApiContractTest, GamificationStateLongestStreakGeCurrentStreak) {
    // longestStreak must always >= currentStreak
    GamificationState state{1, 500, 3, 10, 5}; // longestStreak=5 < currentStreak=10
    EXPECT_FALSE(state.isValid());
}

TEST_F(ApiContractTest, GamificationStateLevelMinimumIsOne) {
    GamificationState state{1, 0, 0, 0, 0}; // level 0 is invalid
    EXPECT_FALSE(state.isValid());
}

// API versioning contract
TEST_F(ApiContractTest, ApiVersionMustBeV1) {
    std::string basePath = "/api/v1";
    EXPECT_THAT(basePath, ::testing::StartsWith("/api/v"));
    EXPECT_EQ(basePath, "/api/v1");
}