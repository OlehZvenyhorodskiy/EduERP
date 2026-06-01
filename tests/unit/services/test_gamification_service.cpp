#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <string>
#include <chrono>

// ---------------------------------------------------------------------------
// GamificationService logic under test (pure-logic extraction).
//
// The real GamificationService.h depends on Qt and SQLite.
// Here we extract and validate the pure algorithms (XP → level, streak math)
// without any infrastructure dependency. These are the rules that must not regress.
// ---------------------------------------------------------------------------

// ── XP to Level calculation ──
// Mirrors GamificationService::levelForXp()
// Formula: level = floor(1 + sqrt(xp / 100))
static int levelForXp(int xp) {
    if (xp <= 0) return 1;
    return static_cast<int>(1 + std::sqrt(static_cast<double>(xp) / 100.0));
}

// ── XP required to reach a level ──
// level N starts at (N-1)^2 * 100 XP
static int xpForLevel(int level) {
    if (level <= 1) return 0;
    return (level - 1) * (level - 1) * 100;
}

// ── Streak calculation ──
// Given last login date string "YYYY-MM-DD" and today's date, determine new streak.
// Returns: 0 = broken, streak+1 = continued, 1 = first login
static int computeNewStreak(int currentStreak, bool loggedYesterday) {
    if (currentStreak == 0) return 1;
    if (loggedYesterday) return currentStreak + 1;
    return 1; // streak resets — not 0, because today counts as day 1
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

class GamificationServiceTest : public ::testing::Test {};

// XP → Level tests
TEST_F(GamificationServiceTest, Level1AtZeroXp) {
    EXPECT_EQ(levelForXp(0), 1);
}

TEST_F(GamificationServiceTest, Level1BelowFirstThreshold) {
    EXPECT_EQ(levelForXp(99), 1);
}

TEST_F(GamificationServiceTest, Level2At100Xp) {
    EXPECT_EQ(levelForXp(100), 2);
}

TEST_F(GamificationServiceTest, Level3At400Xp) {
    EXPECT_EQ(levelForXp(400), 3);
}

TEST_F(GamificationServiceTest, Level10At8100Xp) {
    EXPECT_EQ(levelForXp(8100), 10);
}

TEST_F(GamificationServiceTest, NegativeXpReturnsLevel1) {
    EXPECT_EQ(levelForXp(-50), 1);
}

// Level → XP threshold tests
TEST_F(GamificationServiceTest, Level1StartsAt0Xp) {
    EXPECT_EQ(xpForLevel(1), 0);
}

TEST_F(GamificationServiceTest, Level2StartsAt100Xp) {
    EXPECT_EQ(xpForLevel(2), 100);
}

TEST_F(GamificationServiceTest, Level5StartsAt1600Xp) {
    EXPECT_EQ(xpForLevel(5), 1600);
}

// Roundtrip: levelForXp(xpForLevel(N)) == N
TEST_F(GamificationServiceTest, LevelRoundtripLevels2to15) {
    for (int lvl = 2; lvl <= 15; ++lvl) {
        int xp = xpForLevel(lvl);
        EXPECT_EQ(levelForXp(xp), lvl)
            << "Level " << lvl << " roundtrip failed at XP=" << xp;
    }
}

// Streak tests
TEST_F(GamificationServiceTest, FirstLoginStartsStreak1) {
    EXPECT_EQ(computeNewStreak(0, false), 1);
}

TEST_F(GamificationServiceTest, ContinuedStreakIncrements) {
    EXPECT_EQ(computeNewStreak(5, true), 6);
}

TEST_F(GamificationServiceTest, BrokenStreakResetsTo1) {
    // Even if streak was high, missing a day resets to 1 (today counts)
    EXPECT_EQ(computeNewStreak(30, false), 1);
}

TEST_F(GamificationServiceTest, StreakOf1ContinuedBecomesTwo) {
    EXPECT_EQ(computeNewStreak(1, true), 2);
}

// XP earning edge cases
TEST_F(GamificationServiceTest, LargeXpDoesNotOverflow) {
    // Level formula must handle large XP without crash
    int bigXp = 1'000'000;
    int level = levelForXp(bigXp);
    EXPECT_GT(level, 1);
    EXPECT_LT(level, 1000); // sanity bound
}
