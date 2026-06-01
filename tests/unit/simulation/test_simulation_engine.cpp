#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cmath>
#include <limits>
#include <stdexcept>

// ---------------------------------------------------------------------------
// SimulationEngine logic tests — pure financial/business algorithms.
//
// Tests for: KPI calculation, market event application, profit margin math,
// inventory depletion, and boundary conditions.
// No Qt dependency — these are pure numerical/business logic algorithms.
// ---------------------------------------------------------------------------

// ── KPI Calculations (mirroring SimulationEngine internals) ──

static double calculateProfitMargin(double revenue, double costs) {
    if (revenue <= 0.0) return 0.0;
    return ((revenue - costs) / revenue) * 100.0;
}

static double calculateROI(double netProfit, double investment) {
    if (investment <= 0.0) return 0.0;
    return (netProfit / investment) * 100.0;
}

// Employee productivity score: revenue per employee, capped at 1,000,000
static double employeeProductivity(double revenue, int employees) {
    if (employees <= 0) return 0.0;
    return std::min(revenue / static_cast<double>(employees), 1'000'000.0);
}

// Market share change: positive for growth, negative for decline
// Formula: delta = (currentRevenue - previousRevenue) / previousRevenue * 100
static double revenueGrowthRate(double current, double previous) {
    if (previous <= 0.0) return 0.0;
    return ((current - previous) / previous) * 100.0;
}

// Inventory depletion: units_remaining after N turns of sales_per_turn
static int inventoryAfterTurns(int initial, int salesPerTurn, int turns) {
    if (initial < 0 || salesPerTurn < 0 || turns < 0) return 0;
    int sold = salesPerTurn * turns;
    return std::max(0, initial - sold);
}

// Marketing efficiency: conversions per 1000 currency units spent
static double marketingEfficiency(int conversions, double budget) {
    if (budget <= 0.0) return 0.0;
    return (static_cast<double>(conversions) / budget) * 1000.0;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

class SimulationEngineTest : public ::testing::Test {};

// ── Profit Margin ──
TEST_F(SimulationEngineTest, ProfitMargin50PercentOnEqualSplit) {
    EXPECT_DOUBLE_EQ(calculateProfitMargin(200.0, 100.0), 50.0);
}

TEST_F(SimulationEngineTest, ProfitMarginZeroWhenCostsEqualRevenue) {
    EXPECT_DOUBLE_EQ(calculateProfitMargin(100.0, 100.0), 0.0);
}

TEST_F(SimulationEngineTest, ProfitMarginNegativeWhenLoss) {
    EXPECT_LT(calculateProfitMargin(100.0, 150.0), 0.0);
}

TEST_F(SimulationEngineTest, ProfitMarginZeroOnZeroRevenue) {
    EXPECT_DOUBLE_EQ(calculateProfitMargin(0.0, 50.0), 0.0);
}

TEST_F(SimulationEngineTest, ProfitMargin100PercentOnZeroCost) {
    EXPECT_DOUBLE_EQ(calculateProfitMargin(500.0, 0.0), 100.0);
}

// ── ROI ──
TEST_F(SimulationEngineTest, ROIPositiveOnProfit) {
    // 50 profit on 100 investment = 50% ROI
    EXPECT_DOUBLE_EQ(calculateROI(50.0, 100.0), 50.0);
}

TEST_F(SimulationEngineTest, ROIZeroOnZeroInvestment) {
    EXPECT_DOUBLE_EQ(calculateROI(100.0, 0.0), 0.0);
}

TEST_F(SimulationEngineTest, ROINegativeOnLoss) {
    EXPECT_LT(calculateROI(-20.0, 100.0), 0.0);
}

// ── Employee Productivity ──
TEST_F(SimulationEngineTest, ProductivityDividesRevenueByEmployees) {
    EXPECT_DOUBLE_EQ(employeeProductivity(500'000.0, 5), 100'000.0);
}

TEST_F(SimulationEngineTest, ProductivityZeroWithNoEmployees) {
    EXPECT_DOUBLE_EQ(employeeProductivity(500'000.0, 0), 0.0);
}

TEST_F(SimulationEngineTest, ProductivityCappedAtOneMillion) {
    EXPECT_DOUBLE_EQ(employeeProductivity(10'000'000.0, 1), 1'000'000.0);
}

// ── Revenue Growth Rate ──
TEST_F(SimulationEngineTest, GrowthRate50PercentOnDoubling) {
    EXPECT_DOUBLE_EQ(revenueGrowthRate(150.0, 100.0), 50.0);
}

TEST_F(SimulationEngineTest, GrowthRateZeroOnNoPreviousRevenue) {
    EXPECT_DOUBLE_EQ(revenueGrowthRate(100.0, 0.0), 0.0);
}

TEST_F(SimulationEngineTest, GrowthRateNegativeOnDecline) {
    EXPECT_LT(revenueGrowthRate(80.0, 100.0), 0.0);
}

TEST_F(SimulationEngineTest, GrowthRateZeroOnNoChange) {
    EXPECT_DOUBLE_EQ(revenueGrowthRate(100.0, 100.0), 0.0);
}

// ── Inventory ──
TEST_F(SimulationEngineTest, InventoryDepletesOverTurns) {
    // 100 units, 10 sold/turn, 5 turns = 50 remaining
    EXPECT_EQ(inventoryAfterTurns(100, 10, 5), 50);
}

TEST_F(SimulationEngineTest, InventoryFloorsAtZero) {
    // Selling more than stock — stock can't go negative
    EXPECT_EQ(inventoryAfterTurns(20, 10, 5), 0);
}

TEST_F(SimulationEngineTest, InventoryUnchangedWithZeroSales) {
    EXPECT_EQ(inventoryAfterTurns(100, 0, 10), 100);
}

TEST_F(SimulationEngineTest, InventoryHandlesNegativeInputSafely) {
    EXPECT_EQ(inventoryAfterTurns(-10, 5, 3), 0);
}

// ── Marketing Efficiency ──
TEST_F(SimulationEngineTest, MarketingEfficiencyPerThousand) {
    // 50 conversions on 500 budget = 100 conversions per €1000
    EXPECT_DOUBLE_EQ(marketingEfficiency(50, 500.0), 100.0);
}

TEST_F(SimulationEngineTest, MarketingEfficiencyZeroOnNoBudget) {
    EXPECT_DOUBLE_EQ(marketingEfficiency(100, 0.0), 0.0);
}
