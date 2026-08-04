#pragma once

#include "src/simulation/modules/IModule.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace eduerp::sim {

/**
 * @brief Double-entry bookkeeping account.
 */
struct Account {
    int id = 0;
    std::string code;        // e.g., "1000", "4000"
    std::string name;
    std::string accountType; // asset, liability, equity, revenue, expense
    int parentId = 0;
    bool isActive = true;
};

/**
 * @brief A single ledger entry representing a double-entry transaction.
 */
struct LedgerEntry {
    int id = 0;
    std::string date;
    int debitAccountId = 0;
    int creditAccountId = 0;
    double amount = 0.0;
    std::string description;
    std::string referenceType; // invoice, payment, payroll, adjustment
    int referenceId = 0;
};

/**
 * @brief Finance module implementing double-entry bookkeeping, reporting, and cash flow.
 */
class FinanceModule : public IModule {
private:
    bool m_enabled = true;
    std::vector<Account> m_accounts;
    std::vector<LedgerEntry> m_ledger;

    double m_cashOnHand = 100000.00;
    double m_totalRevenue = 0;
    double m_totalExpenses = 0;

public:
    std::string name() const override { return "Finance"; }
    std::string type() const override { return "finance"; }

    void initialize() override {
        spdlog::info("FinanceModule: Initializing chart of accounts");
        // Standard Belgian chart of accounts (simplified)
        m_accounts = {
            {1, "1000", "Kas / Bankrekening", "asset", 0, true},
            {2, "1100", "Debiteuren", "asset", 0, true},
            {3, "2000", "Crediteuren", "liability", 0, true},
            {4, "3000", "Eigen vermogen", "equity", 0, true},
            {5, "4000", "Verkoopopbrengsten", "revenue", 0, true},
            {6, "5000", "Inkoopkosten", "expense", 0, true},
            {7, "5100", "Loonkosten", "expense", 0, true},
            {8, "5200", "Huurkosten", "expense", 0, true},
            {9, "5300", "Marketingkosten", "expense", 0, true},
            {10, "6000", "BTW Ontvangen", "liability", 0, true},
            {11, "6100", "BTW Betaald", "asset", 0, true},
        };
    }

    void tick(const std::string& simulatedDate) override {
        // Daily financial processing:
        // 1. Process pending invoices
        // 2. Calculate interest
        // 3. Check for scheduled payments (rent, salaries if end of month)
        spdlog::debug("FinanceModule: Processing day {}", simulatedDate);
    }

    /**
     * @brief Record a double-entry transaction.
     */
    void recordTransaction(const std::string& date, int debitId, int creditId, double amount,
                           const std::string& desc, const std::string& refType = "", int refId = 0)
    {
        LedgerEntry entry;
        entry.id = static_cast<int>(m_ledger.size()) + 1;
        entry.date = date;
        entry.debitAccountId = debitId;
        entry.creditAccountId = creditId;
        entry.amount = amount;
        entry.description = desc;
        entry.referenceType = refType;
        entry.referenceId = refId;

        m_ledger.push_back(entry);

        // Update running totals based on account types
        auto* debitAcc = findAccount(debitId);
        auto* creditAcc = findAccount(creditId);

        if (debitAcc && debitAcc->accountType == "expense") m_totalExpenses += amount;
        if (creditAcc && creditAcc->accountType == "revenue") m_totalRevenue += amount;

        // Update cash if bank account is involved
        if (debitAcc && debitAcc->code == "1000") m_cashOnHand += amount;
        if (creditAcc && creditAcc->code == "1000") m_cashOnHand -= amount;
    }

    double cashOnHand() const { return m_cashOnHand; }
    double totalRevenue() const { return m_totalRevenue; }
    double totalExpenses() const { return m_totalExpenses; }
    double netProfit() const { return m_totalRevenue - m_totalExpenses; }

    std::string serializeState() const override {
        nlohmann::json j;
        j["cash_on_hand"] = m_cashOnHand;
        j["total_revenue"] = m_totalRevenue;
        j["total_expenses"] = m_totalExpenses;
        j["ledger_count"] = m_ledger.size();
        return j.dump();
    }

    void deserializeState(const std::string& json) override {
        auto j = nlohmann::json::parse(json);
        m_cashOnHand = j.value("cash_on_hand", 100000.0);
        m_totalRevenue = j.value("total_revenue", 0.0);
        m_totalExpenses = j.value("total_expenses", 0.0);
    }

    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }

private:
    Account* findAccount(int id) {
        for (auto& acc : m_accounts) {
            if (acc.id == id) return &acc;
        }
        return nullptr;
    }
};

} // namespace eduerp::sim