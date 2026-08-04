#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace eduerp::sim {

/**
 * @brief Seed data for each industry template.
 *        Provides realistic starting conditions for Belgian business simulation.
 */
struct TemplateSeedData {
    std::string templateId;
    std::string displayName;
    std::string description;
    double initialBudget;

    // Finance seed
    struct FinanceSeed {
        double cashOnHand;
        double monthlyRent;
        double monthlyUtilities;
        double monthlyInsurance;
        double taxRate; // Belgian corporate tax
    } finance;

    // Employee seed
    struct EmployeeSeed {
        std::string name;
        std::string department;
        std::string position;
        double salary;
    };
    std::vector<EmployeeSeed> initialEmployees;

    // Product seed
    struct ProductSeed {
        std::string sku;
        std::string name;
        std::string category;
        double costPrice;
        double sellingPrice;
        int initialStock;
    };
    std::vector<ProductSeed> initialProducts;

    // Customer seed
    struct CustomerSeed {
        std::string name;
        std::string type;
        std::string segment;
        int loyaltyScore;
    };
    std::vector<CustomerSeed> initialCustomers;

    // Marketing seed
    double initialBrandAwareness;

    // Logistics seed
    struct WarehouseSeed {
        std::string name;
        std::string location;
        int capacityUnits;
        double maintenanceCost;
    };
    std::vector<WarehouseSeed> initialWarehouses;
};

/**
 * @brief Registry of all available industry templates.
 */
class IndustryTemplates {
public:
    static const TemplateSeedData& get(const std::string& templateId) {
        static auto templates = buildTemplates();
        auto it = templates.find(templateId);
        if (it != templates.end()) {
            return it->second;
        }
        spdlog::warn("IndustryTemplates: Unknown template '{}', falling back to belgian_sme", templateId);
        return templates.at("belgian_sme");
    }

    static std::vector<std::string> availableTemplates() {
        return {
            "retail_clothing", "tech_hardware", "logistics_delivery",
            "accounting_services", "food_beverage", "ecommerce_marketplace",
            "tech_giant", "semiconductor", "belgian_sme"
        };
    }

private:
    static std::unordered_map<std::string, TemplateSeedData> buildTemplates() {
        std::unordered_map<std::string, TemplateSeedData> m;

        m["belgian_sme"] = {
            "belgian_sme", "Belgisch KMO", "Klein/middelgroot bedrijf", 100000.0,
            {100000.0, 1200.0, 350.0, 200.0, 0.25},
            {
                {"Marie Peeters", "admin", "Office Manager", 2800.0},
                {"Jan Willems", "sales", "Verkoper", 2500.0},
            },
            {
                {"GEN-001", "Standaard Dienst A", "services", 50.0, 120.0, 0},
                {"GEN-002", "Standaard Dienst B", "services", 30.0, 80.0, 0},
            },
            {
                {"Lokaal Bedrijf NV", "business", "standard", 50},
                {"Gemeente Diensten", "business", "standard", 60},
            },
            0.05,
            {{"Kantoor", "Brussel", 500, 300.0}}
        };

        m["retail_clothing"] = {
            "retail_clothing", "Kledingwinkel", "Detailhandel in mode", 120000.0,
            {120000.0, 2500.0, 450.0, 300.0, 0.25},
            {
                {"Sophie De Vos", "sales", "Winkelmanager", 3000.0},
                {"Emma Janssens", "sales", "Verkoopster", 2200.0},
                {"Lucas Maes", "sales", "Verkoopster", 2200.0},
            },
            {
                {"CLT-001", "T-Shirt Basic", "tops", 8.50, 24.99, 200},
                {"CLT-002", "Jeans Slim Fit", "bottoms", 18.00, 59.99, 100},
                {"CLT-003", "Hoodie Premium", "outerwear", 22.00, 69.99, 80},
                {"CLT-004", "Sneakers Urban", "footwear", 35.00, 89.99, 60},
                {"CLT-005", "Sjaal Wol", "accessories", 6.00, 19.99, 150},
            },
            {
                {"Modewinkel Gent NV", "business", "premium", 70},
                {"Student Shoppers", "individual", "budget", 40},
                {"Trendy Boutique", "business", "standard", 55},
                {"Online Klanten", "individual", "standard", 50},
            },
            0.08,
            {{"Winkel + Magazijn", "Antwerpen", 5000, 500.0}}
        };

        m["tech_hardware"] = {
            "tech_hardware", "Tech Hardware", "Computerapparatuur", 200000.0,
            {200000.0, 3000.0, 800.0, 500.0, 0.25},
            {
                {"Thomas Claes", "management", "Directeur", 4500.0},
                {"Noor El Amrani", "sales", "Account Manager", 3200.0},
                {"Pieter Wouters", "production", "Technicus", 2800.0},
                {"Lien Vandenberghe", "admin", "Boekhouder", 2600.0},
            },
            {
                {"HW-001", "Laptop Pro 15\"", "computers", 450.0, 899.0, 50},
                {"HW-002", "Desktop Workstation", "computers", 600.0, 1299.0, 30},
                {"HW-003", "Monitor 27\" 4K", "peripherals", 180.0, 449.0, 80},
                {"HW-004", "Toetsenbord Mechanisch", "peripherals", 35.0, 89.0, 200},
                {"HW-005", "Muis Ergonomisch", "peripherals", 15.0, 49.0, 300},
                {"HW-006", "USB-C Hub", "accessories", 12.0, 39.0, 500},
            },
            {
                {"Belgian Electronics NV", "business", "premium", 75},
                {"School Inkoop Dienst", "business", "standard", 60},
                {"Freelance Ontwikkelaars", "individual", "premium", 45},
                {"KMO Tech Oplossingen", "business", "standard", 55},
            },
            0.10,
            {{"Magazijn Mechelen", "Mechelen", 10000, 800.0}}
        };

        m["food_beverage"] = {
            "food_beverage", "Horeca", "Eten en drinken", 80000.0,
            {80000.0, 3500.0, 600.0, 400.0, 0.25},
            {
                {"Chef Karim", "production", "Hoofdkok", 3500.0},
                {"Lisa De Smedt", "sales", "Bediening", 2100.0},
                {"Tom Hendrickx", "sales", "Bediening", 2100.0},
            },
            {
                {"FB-001", "Stoofvlees", "main", 4.50, 18.50, 0},
                {"FB-002", "Vol-au-vent", "main", 3.80, 16.50, 0},
                {"FB-003", "Frieten", "side", 0.80, 4.50, 0},
                {"FB-004", "Belgisch Bier", "drinks", 1.20, 4.50, 500},
                {"FB-005", "Wafel", "desserts", 1.00, 6.50, 0},
            },
            {
                {"Buurtbewoners", "individual", "standard", 60},
                {"Kantoorwerkers", "individual", "standard", 50},
                {"Toeristen", "individual", "budget", 30},
            },
            0.15,
            {{"Keuken + Opslag", "Brugge", 2000, 400.0}}
        };

        m["ecommerce_marketplace"] = {
            "ecommerce_marketplace", "E-commerce", "Online marktplaats", 150000.0,
            {150000.0, 500.0, 200.0, 350.0, 0.25},
            {
                {"Dries Vermeersch", "management", "CEO", 4000.0},
                {"Sarah Cools", "marketing", "Digital Marketeer", 3000.0},
                {"Kevin Michiels", "production", "Webontwikkelaar", 3200.0},
                {"Amber Luyten", "admin", "Klantenservice", 2300.0},
            },
            {
                {"EC-001", "Smartphone Hoesje", "electronics", 2.00, 14.99, 1000},
                {"EC-002", "Bluetooth Speaker", "electronics", 18.00, 49.99, 200},
                {"EC-003", "LED Strip 5m", "home", 5.00, 19.99, 500},
                {"EC-004", "Yogamat", "sport", 8.00, 29.99, 300},
            },
            {
                {"Online Shoppers BE", "individual", "standard", 40},
                {"Dropship Partners", "business", "standard", 50},
                {"Social Media Kopers", "individual", "budget", 35},
            },
            0.03,
            {{"Fulfillment Center", "Luik", 20000, 1200.0}}
        };

        m["logistics_delivery"] = {
            "logistics_delivery", "Logistiek", "Bezorg- en transportdiensten", 250000.0,
            {250000.0, 4000.0, 1200.0, 800.0, 0.25},
            {
                {"Mark Stevens", "management", "Operations Manager", 4200.0},
                {"Chauffeur 1", "production", "Bezorger", 2400.0},
                {"Chauffeur 2", "production", "Bezorger", 2400.0},
                {"Chauffeur 3", "production", "Bezorger", 2400.0},
                {"Dispatch Operator", "admin", "Planner", 2600.0},
            },
            {},
            {
                {"Webshops BE", "business", "standard", 50},
                {"Restaurants", "business", "standard", 55},
                {"Industriële klanten", "business", "premium", 70},
            },
            0.06,
            {
                {"Hub Antwerpen", "Antwerpen", 15000, 1000.0},
                {"Hub Brussel", "Brussel", 10000, 900.0},
            }
        };

        m["accounting_services"] = {
            "accounting_services", "Boekhouding", "Accountancykantoor", 60000.0,
            {60000.0, 1500.0, 300.0, 250.0, 0.25},
            {
                {"Partner Accountant", "management", "Partner", 5000.0},
                {"Junior Accountant", "admin", "Boekhouder", 2500.0},
            },
            {
                {"ACC-001", "Jaarrekening KMO", "services", 200.0, 850.0, 0},
                {"ACC-002", "BTW Aangifte", "services", 30.0, 150.0, 0},
                {"ACC-003", "Loonberekening", "services", 15.0, 75.0, 0},
            },
            {
                {"KMO Klant 1", "business", "standard", 70},
                {"KMO Klant 2", "business", "standard", 65},
                {"Zelfstandige", "individual", "budget", 40},
            },
            0.12,
            {{"Kantoor", "Leuven", 200, 150.0}}
        };

        // Simplified entries for tech_giant and semiconductor
        m["tech_giant"] = {
            "tech_giant", "Tech Gigant", "Groot technologiebedrijf", 500000.0,
            {500000.0, 8000.0, 2000.0, 1500.0, 0.25},
            {
                {"CEO", "management", "CEO", 8000.0},
                {"CTO", "management", "CTO", 7000.0},
                {"Lead Dev 1", "production", "Senior Developer", 4500.0},
                {"Lead Dev 2", "production", "Senior Developer", 4500.0},
                {"Designer", "production", "UI/UX Designer", 3500.0},
                {"HR Manager", "admin", "HR Manager", 3200.0},
                {"Sales Lead", "sales", "Sales Director", 4000.0},
                {"Marketeer", "marketing", "Marketing Manager", 3500.0},
            },
            {
                {"SW-001", "Enterprise Licentie", "software", 100.0, 999.0, 0},
                {"SW-002", "SaaS Abonnement /maand", "software", 10.0, 49.0, 0},
            },
            {
                {"Enterprise Client", "business", "premium", 80},
                {"MKB Client", "business", "standard", 55},
            },
            0.15,
            {{"Data Center", "Zaventem", 50000, 3000.0}}
        };

        m["semiconductor"] = {
            "semiconductor", "Halfgeleiders", "Chipproductie", 1000000.0,
            {1000000.0, 15000.0, 5000.0, 3000.0, 0.25},
            {
                {"Dr. Ingenieur", "management", "Plant Director", 9000.0},
                {"Process Engineer 1", "production", "Process Engineer", 5000.0},
                {"Process Engineer 2", "production", "Process Engineer", 5000.0},
                {"Quality Manager", "production", "Quality Control", 4000.0},
                {"Operator 1", "production", "Machine Operator", 2800.0},
                {"Operator 2", "production", "Machine Operator", 2800.0},
            },
            {
                {"SC-001", "Wafer 300mm", "components", 500.0, 2500.0, 20},
                {"SC-002", "Chip Package", "finished", 50.0, 350.0, 100},
            },
            {
                {"Auto Industrie", "business", "premium", 85},
                {"Consumer Electronics", "business", "premium", 75},
            },
            0.20,
            {{"Fab Facility", "Leuven", 100000, 8000.0}}
        };

        return m;
    }
};

} // namespace eduerp::sim