#pragma once

#include "src/simulation/modules/IModule.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace eduerp::sim {

struct Product {
    int id = 0;
    std::string sku;
    std::string name;
    std::string description;
    std::string category;
    double costPrice = 0;
    double sellingPrice = 0;
    int currentStock = 0;
    int minStockLevel = 10;
    int maxStockLevel = 1000;
    int reorderPoint = 25;
    std::string unitOfMeasure = "stuks";
    bool isActive = true;
};

struct Supplier {
    int id = 0;
    std::string name;
    std::string contactEmail;
    int leadTimeDays = 7;
    double reliabilityScore = 0.95;
    bool isActive = true;
};

struct StockMovement {
    int id = 0;
    int productId = 0;
    std::string movementType; // in, out, adjustment, transfer
    int quantity = 0;
    int stockAfter = 0;
    double unitCost = 0;
    std::string referenceType;
    int referenceId = 0;
};

class InventoryModule : public IModule {
private:
    bool m_enabled = true;
    std::vector<Product> m_products;
    std::vector<Supplier> m_suppliers;
    std::vector<StockMovement> m_movements;

public:
    std::string name() const override { return "Inventory"; }
    std::string type() const override { return "inventory"; }

    void initialize() override {
        spdlog::info("InventoryModule: Initializing inventory system");
    }

    void tick(const std::string& simulatedDate) override {
        spdlog::debug("InventoryModule: Processing day {}", simulatedDate);
        // Check for low stock alerts
        // Process incoming purchase orders
        // Update stock valuations
        checkLowStock();
    }

    void addProduct(const Product& product) {
        m_products.push_back(product);
    }

    void addSupplier(const Supplier& supplier) {
        m_suppliers.push_back(supplier);
    }

    bool adjustStock(int productId, int quantity, const std::string& type,
                     const std::string& refType = "", int refId = 0)
    {
        for (auto& product : m_products) {
            if (product.id == productId) {
                product.currentStock += quantity;
                if (product.currentStock < 0) {
                    product.currentStock -= quantity; // Rollback
                    return false;
                }
                StockMovement movement;
                movement.id = static_cast<int>(m_movements.size()) + 1;
                movement.productId = productId;
                movement.movementType = type;
                movement.quantity = quantity;
                movement.stockAfter = product.currentStock;
                movement.unitCost = product.costPrice;
                movement.referenceType = refType;
                movement.referenceId = refId;
                m_movements.push_back(movement);
                return true;
            }
        }
        return false;
    }

    double totalInventoryValue() const {
        double total = 0;
        for (const auto& p : m_products) {
            total += p.currentStock * p.costPrice;
        }
        return total;
    }

    const std::vector<Product>& products() const { return m_products; }

    std::string serializeState() const override {
        nlohmann::json j;
        j["product_count"] = m_products.size();
        j["total_value"] = totalInventoryValue();
        j["movement_count"] = m_movements.size();
        return j.dump();
    }

    void deserializeState(const std::string& json) override {}

    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }

private:
    void checkLowStock() {
        for (const auto& p : m_products) {
            if (p.isActive && p.currentStock <= p.reorderPoint) {
                spdlog::warn("InventoryModule: Low stock alert for '{}' (stock: {}, reorder: {})",
                             p.name, p.currentStock, p.reorderPoint);
            }
        }
    }
};

} // namespace eduerp::sim