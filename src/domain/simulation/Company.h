#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace eduerp::domain {

struct CompanySettings {
    std::string timeScale = "realtime";
    int simulationSpeed = 1;
    std::string currencyCode = "EUR";
    bool isAiEnabled = false;
};

struct Company {
    int id = 0;
    int schoolId = 0;
    int teamId = 0;
    int creatorId = 0;

    std::string name;
    std::string logoUrl;
    std::string industryTemplate; // retail, tech, logistics, etc.
    double initialBudget = 100000.00;

    CompanySettings settings;

    std::string status = "active"; // active, paused, completed, archived
    std::string currentSimulatedDate;
    std::string simulationStartDate;
    std::string createdAt;
};

enum class IndustryTemplate {
    RetailClothing,
    TechHardware,
    LogisticsDelivery,
    AccountingServices,
    FoodBeverage,
    EcommerceMarketplace,
    TechGiant,
    Semiconductor,
    BelgianSME
};

inline std::string industryTemplateToString(IndustryTemplate tmpl) {
    switch (tmpl) {
        case IndustryTemplate::RetailClothing: return "retail_clothing";
        case IndustryTemplate::TechHardware: return "tech_hardware";
        case IndustryTemplate::LogisticsDelivery: return "logistics_delivery";
        case IndustryTemplate::AccountingServices: return "accounting_services";
        case IndustryTemplate::FoodBeverage: return "food_beverage";
        case IndustryTemplate::EcommerceMarketplace: return "ecommerce_marketplace";
        case IndustryTemplate::TechGiant: return "tech_giant";
        case IndustryTemplate::Semiconductor: return "semiconductor";
        case IndustryTemplate::BelgianSME: return "belgian_sme";
    }
    return "unknown";
}

} // namespace eduerp::domain