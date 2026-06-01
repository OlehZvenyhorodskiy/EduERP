#pragma once

#include "src/simulation/modules/IModule.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace eduerp::sim {

struct Customer {
    int id = 0;
    std::string name;
    std::string type;      // individual, business
    std::string segment;   // premium, standard, budget
    int loyaltyScore = 50;
    int priceSensitivity = 50;
    int qualityExpectation = 50;
    bool isActive = true;
    int totalOrders = 0;
    double lifetimeValue = 0;
};

struct PipelineDeal {
    int id = 0;
    int customerId = 0;
    std::string stage;         // lead, prospect, proposal, negotiation, closed_won, closed_lost
    double expectedValue = 0;
    double probability = 0.20;
    int assignedTo = 0;
};

struct SalesOrder {
    int id = 0;
    int customerId = 0;
    std::string orderNumber;
    std::string orderDate;
    std::string status = "pending"; // pending, confirmed, shipped, delivered, cancelled
    double subtotal = 0;
    double taxAmount = 0;
    double totalAmount = 0;
    std::string paymentStatus = "unpaid";
};

class SalesModule : public IModule {
private:
    bool m_enabled = true;
    std::vector<Customer> m_customers;
    std::vector<PipelineDeal> m_pipeline;
    std::vector<SalesOrder> m_orders;
    int m_nextOrderId = 1;

public:
    std::string name() const override { return "Sales & CRM"; }
    std::string type() const override { return "sales"; }

    void initialize() override {
        spdlog::info("SalesModule: Initializing with seed customers");
        // Seed some initial simulated customers
        m_customers = {
            {1, "Belgian Electronics NV", "business", "premium", 70, 30, 80, true, 0, 0},
            {2, "Kleine Winkel BVBA", "business", "standard", 50, 60, 50, true, 0, 0},
            {3, "Jan Janssens", "individual", "budget", 40, 80, 40, true, 0, 0},
            {4, "Premium Partners NV", "business", "premium", 80, 20, 90, true, 0, 0},
            {5, "Lokale Bakker", "business", "standard", 60, 50, 60, true, 0, 0},
        };
    }

    void tick(const std::string& simulatedDate) override {
        spdlog::debug("SalesModule: Processing day {}", simulatedDate);
        // Process pipeline deals — advance stages based on probability
        // Generate new leads based on marketing effectiveness
        // Process deliveries and payments
    }

    void addDeal(int customerId, double value) {
        PipelineDeal deal;
        deal.id = static_cast<int>(m_pipeline.size()) + 1;
        deal.customerId = customerId;
        deal.stage = "lead";
        deal.expectedValue = value;
        deal.probability = 0.20;
        m_pipeline.push_back(deal);
    }

    SalesOrder createOrder(int customerId, double amount) {
        SalesOrder order;
        order.id = m_nextOrderId++;
        order.customerId = customerId;
        order.orderNumber = "SO-" + std::to_string(order.id);
        order.subtotal = amount;
        order.taxAmount = amount * 0.21; // Belgian VAT 21%
        order.totalAmount = amount + order.taxAmount;
        m_orders.push_back(order);
        return order;
    }

    const std::vector<Customer>& customers() const { return m_customers; }
    const std::vector<PipelineDeal>& pipeline() const { return m_pipeline; }

    std::string serializeState() const override {
        nlohmann::json j;
        j["customer_count"] = m_customers.size();
        j["pipeline_count"] = m_pipeline.size();
        j["order_count"] = m_orders.size();
        return j.dump();
    }

    void deserializeState(const std::string& json) override {
        // Restore from serialized state
    }

    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }
};

} // namespace eduerp::sim
