#pragma once

#include "src/simulation/modules/IModule.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <cmath>

namespace eduerp::sim {

struct Shipment {
    int id = 0;
    int orderId = 0;
    std::string origin;
    std::string destination;
    std::string carrier;
    std::string status = "pending"; // pending, in_transit, delivered, failed
    double weight = 0;
    double cost = 0;
    int estimatedDays = 0;
    int daysInTransit = 0;
};

struct Warehouse {
    int id = 0;
    std::string name;
    std::string location;
    int capacityUnits = 10000;
    int usedUnits = 0;
    double maintenanceCost = 500.00;
    bool isActive = true;
};

struct Route {
    int id = 0;
    std::string origin;
    std::string destination;
    double distanceKm = 0;
    double costPerKm = 0.15;
    int estimatedDays = 1;
};

class LogisticsModule : public IModule {
private:
    bool m_enabled = true;
    std::vector<Shipment> m_shipments;
    std::vector<Warehouse> m_warehouses;
    std::vector<Route> m_routes;
    int m_nextShipmentId = 1;
    double m_totalShippingCost = 0;

public:
    std::string name() const override { return "Logistiek"; }
    std::string type() const override { return "logistics"; }

    void initialize() override {
        spdlog::info("LogisticsModule: Initializing logistics system");

        // Default warehouse
        m_warehouses.push_back({1, "Hoofdmagazijn", "Antwerpen", 10000, 0, 500.00, true});

        // Default routes (Belgian cities)
        m_routes = {
            {1, "Antwerpen", "Brussel", 45.0, 0.15, 1},
            {2, "Antwerpen", "Gent", 60.0, 0.15, 1},
            {3, "Brussel", "Luik", 100.0, 0.15, 1},
            {4, "Antwerpen", "Brugge", 95.0, 0.15, 1},
            {5, "Brussel", "Charleroi", 60.0, 0.15, 1},
        };
    }

    void tick(const std::string& simulatedDate) override {
        spdlog::debug("LogisticsModule: Processing day {}", simulatedDate);

        // Advance shipments in transit
        for (auto& shipment : m_shipments) {
            if (shipment.status == "in_transit") {
                shipment.daysInTransit++;
                if (shipment.daysInTransit >= shipment.estimatedDays) {
                    shipment.status = "delivered";
                    spdlog::info("LogisticsModule: Shipment {} delivered", shipment.id);
                }
            }
        }
    }

    int createShipment(int orderId, const std::string& origin, const std::string& dest,
                       double weight)
    {
        Shipment shipment;
        shipment.id = m_nextShipmentId++;
        shipment.orderId = orderId;
        shipment.origin = origin;
        shipment.destination = dest;
        shipment.weight = weight;
        shipment.status = "in_transit";

        // Find route for cost/time calculation
        for (const auto& route : m_routes) {
            if (route.origin == origin && route.destination == dest) {
                shipment.estimatedDays = route.estimatedDays;
                shipment.cost = route.distanceKm * route.costPerKm + weight * 0.05;
                break;
            }
        }

        if (shipment.estimatedDays == 0) {
            shipment.estimatedDays = 3; // Fallback
            shipment.cost = weight * 0.10 + 5.0;
        }

        m_totalShippingCost += shipment.cost;
        m_shipments.push_back(shipment);
        return shipment.id;
    }

    double totalShippingCost() const { return m_totalShippingCost; }

    int activeShipmentCount() const {
        int count = 0;
        for (const auto& s : m_shipments) {
            if (s.status == "in_transit") count++;
        }
        return count;
    }

    std::string serializeState() const override {
        nlohmann::json j;
        j["shipment_count"] = m_shipments.size();
        j["active_shipments"] = activeShipmentCount();
        j["total_shipping_cost"] = m_totalShippingCost;
        j["warehouse_count"] = m_warehouses.size();
        return j.dump();
    }

    void deserializeState(const std::string& json) override {}
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }
};

} // namespace eduerp::sim