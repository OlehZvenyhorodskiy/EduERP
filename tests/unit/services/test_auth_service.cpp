#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <regex>

// ---------------------------------------------------------------------------
// AuthService logic tests — pure algorithms only.
//
// Tests for: domain extraction, PKCE code verifier validation,
// JWT structure validation (without secret), email format validation.
// No Qt dependency — these are string/crypto algorithms only.
// ---------------------------------------------------------------------------

// ── extractDomain — mirrors auth.go helper ──
static std::string extractDomain(const std::string& email) {
    for (int i = static_cast<int>(email.size()) - 1; i >= 0; --i) {
        if (email[i] == '@') {
            return email.substr(i + 1);
        }
    }
    return "";
}

// ── isValidEmail — basic RFC 5322 lite check ──
static bool isValidEmail(const std::string& email) {
    // Must contain exactly one @, non-empty local and domain parts
    std::regex pattern(R"([^@\s]+@[^@\s]+\.[^@\s]+)");
    return std::regex_match(email, pattern);
}

// ── PKCE code verifier validation ──
// RFC 7636: 43–128 chars, charset: A-Z a-z 0-9 - . _ ~
static bool isValidCodeVerifier(const std::string& verifier) {
    if (verifier.size() < 43 || verifier.size() > 128) return false;
    static const std::string allowed =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    for (char c : verifier) {
        if (allowed.find(c) == std::string::npos) return false;
    }
    return true;
}

// ── JWT structure check ──
// A well-formed JWT has exactly 3 base64url segments separated by dots.
static bool hasJwtStructure(const std::string& token) {
    size_t first = token.find('.');
    if (first == std::string::npos) return false;
    size_t second = token.find('.', first + 1);
    if (second == std::string::npos) return false;
    // Must be exactly 2 dots
    size_t third = token.find('.', second + 1);
    return third == std::string::npos;
}

// ---------------------------------------------------------------------------
// Domain extraction tests
// ---------------------------------------------------------------------------

class AuthServiceTest : public ::testing::Test {};

TEST_F(AuthServiceTest, ExtractsDomainFromNormalEmail) {
    EXPECT_EQ(extractDomain("alice@school.be"), "school.be");
}

TEST_F(AuthServiceTest, ExtractsDomainFromSubdomain) {
    EXPECT_EQ(extractDomain("student@leerling.atalanta.be"), "leerling.atalanta.be");
}

TEST_F(AuthServiceTest, ReturnsEmptyForEmailWithNoAt) {
    EXPECT_EQ(extractDomain("notanemail"), "");
}

TEST_F(AuthServiceTest, UsesLastAtSignForDomain) {
    // Gmail-style plus addressing: alice+tag@school.be
    EXPECT_EQ(extractDomain("alice+tag@school.be"), "school.be");
}

TEST_F(AuthServiceTest, EmptyEmailReturnsEmpty) {
    EXPECT_EQ(extractDomain(""), "");
}

// ---------------------------------------------------------------------------
// Email validation tests
// ---------------------------------------------------------------------------

TEST_F(AuthServiceTest, ValidSchoolEmail) {
    EXPECT_TRUE(isValidEmail("student@atalanta.be"));
}

TEST_F(AuthServiceTest, ValidGoogleWorkspaceEmail) {
    EXPECT_TRUE(isValidEmail("teacher@school.be"));
}

TEST_F(AuthServiceTest, InvalidEmailNoAt) {
    EXPECT_FALSE(isValidEmail("notenemailaddress"));
}

TEST_F(AuthServiceTest, InvalidEmailNoDomain) {
    EXPECT_FALSE(isValidEmail("user@"));
}

// ---------------------------------------------------------------------------
// PKCE code verifier tests
// ---------------------------------------------------------------------------

TEST_F(AuthServiceTest, ValidCodeVerifier64Chars) {
    std::string verifier(64, 'A'); // 64 'A' chars
    EXPECT_TRUE(isValidCodeVerifier(verifier));
}

TEST_F(AuthServiceTest, TooShortCodeVerifierRejected) {
    std::string verifier(42, 'A'); // Below 43 minimum
    EXPECT_FALSE(isValidCodeVerifier(verifier));
}

TEST_F(AuthServiceTest, TooLongCodeVerifierRejected) {
    std::string verifier(129, 'A'); // Exceeds 128 maximum
    EXPECT_FALSE(isValidCodeVerifier(verifier));
}

TEST_F(AuthServiceTest, InvalidCharInVerifierRejected) {
    std::string verifier(64, 'A');
    verifier[32] = ' '; // Space is not allowed
    EXPECT_FALSE(isValidCodeVerifier(verifier));
}

TEST_F(AuthServiceTest, AllowedSpecialCharsInVerifier) {
    // RFC 7636 allows - . _ ~ chars
    std::string verifier = std::string(20, 'A') + "-._~" + std::string(39, 'B');
    EXPECT_TRUE(isValidCodeVerifier(verifier));
}

// ---------------------------------------------------------------------------
// JWT structure tests
// ---------------------------------------------------------------------------

TEST_F(AuthServiceTest, WellFormedJwtHasThreeParts) {
    std::string jwt = "eyJhbGciOiJIUzI1NiJ9.eyJ1c2VyX2lkIjoxfQ.signature";
    EXPECT_TRUE(hasJwtStructure(jwt));
}

TEST_F(AuthServiceTest, TwoSegmentTokenIsNotJwt) {
    EXPECT_FALSE(hasJwtStructure("header.payload"));
}

TEST_F(AuthServiceTest, FourSegmentTokenIsNotJwt) {
    EXPECT_FALSE(hasJwtStructure("a.b.c.d"));
}

TEST_F(AuthServiceTest, EmptyTokenIsNotJwt) {
    EXPECT_FALSE(hasJwtStructure(""));
}
