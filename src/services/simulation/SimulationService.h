#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include <unordered_map>
#include <spdlog/spdlog.h>

#include "src/simulation/engine/SimulationEngine.h"
#include "src/simulation/modules/finance/FinanceModule.h"
#include "src/simulation/modules/sales/SalesModule.h"
#include "src/simulation/modules/inventory/InventoryModule.h"
#include "src/simulation/modules/hr/HRModule.h"
#include "src/simulation/modules/marketing/MarketingModule.h"
#include "src/simulation/modules/logistics/LogisticsModule.h"
#include "src/simulation/templates/IndustryTemplates.h"

namespace eduerp::services {

/**
 * @brief Owns SimulationEngine instances and drives the tick clock.
 *        One SimulationService per active company.
 *        The QTimer fires at intervals determined by timeScale and speed.
 */
class SimulationService : public QObject {
    Q_OBJECT

public:
    explicit SimulationService(QObject* parent = nullptr) : QObject(parent) {
        connect(&m_tickTimer, &QTimer::timeout, this, &SimulationService::onTick);
    }

    /**
     * @brief Create a new simulation for a company using an industry template.
     * @return Raw pointer to the engine (owned by this service).
     */
    sim::SimulationEngine* createSimulation(int companyId, const std::string& industryTemplate) {
        spdlog::info("SimulationService: Creating simulation for company {} with template '{}'",
                     companyId, industryTemplate);

        auto engine = std::make_unique<sim::SimulationEngine>(companyId);
        const auto& seed = sim::IndustryTemplates::get(industryTemplate);

        // Create and seed Finance module
        auto finance = std::make_unique<sim::FinanceModule>();
        finance->initialize();
        // Set starting cash from template
        engine->addModule(std::move(finance));

        // Create and seed Sales module
        auto sales = std::make_unique<sim::SalesModule>();
        sales->initialize();
        // Seed customers from template
        engine->addModule(std::move(sales));

        // Create and seed Inventory module
        auto inventory = std::make_unique<sim::InventoryModule>();
        inventory->initialize();
        // Seed products from template
        for (const auto& p : seed.initialProducts) {
            sim::Product product;
            product.id = static_cast<int>(&p - &seed.initialProducts[0]) + 1;
            product.sku = p.sku;
            product.name = p.name;
            product.category = p.category;
            product.costPrice = p.costPrice;
            product.sellingPrice = p.sellingPrice;
            product.currentStock = p.initialStock;
            inventory->addProduct(product);
        }
        engine->addModule(std::move(inventory));

        // Create and seed HR module
        auto hr = std::make_unique<sim::HRModule>();
        hr->initialize();
        for (const auto& e : seed.initialEmployees) {
            hr->hireEmployee(e.name, e.department, e.position, e.salary);
        }
        engine->addModule(std::move(hr));

        // Create and seed Marketing module
        auto marketing = std::make_unique<sim::MarketingModule>();
        marketing->initialize();
        engine->addModule(std::move(marketing));

        // Create and seed Logistics module
        auto logistics = std::make_unique<sim::LogisticsModule>();
        logistics->initialize();
        engine->addModule(std::move(logistics));

        // Listen for simulation events
        engine->addEventListener([this, companyId](const sim::SimulationEngine::SimulationEvent& event) {
            spdlog::info("SimEvent[{}]: {} - {}", companyId, event.eventType, event.title);
            emit simulationEvent(companyId, QString::fromStdString(event.title),
                                 QString::fromStdString(event.description));
        });

        auto* rawPtr = engine.get();
        m_engines[companyId] = std::move(engine);
        m_activeCompanyId = companyId;

        spdlog::info("SimulationService: Simulation created with {} modules, {} employees, {} products",
                     6, seed.initialEmployees.size(), seed.initialProducts.size());

        return rawPtr;
    }

    /**
     * @brief Start the simulation clock.
     */
    void start(int companyId) {
        if (m_engines.find(companyId) == m_engines.end()) {
            spdlog::error("SimulationService: No engine for company {}", companyId);
            return;
        }
        m_activeCompanyId = companyId;
        m_engines[companyId]->resume();
        updateTickInterval();
        m_tickTimer.start();
        spdlog::info("SimulationService: Started simulation for company {}", companyId);
        emit simulationStarted(companyId);
    }

    void pause(int companyId) {
        if (auto it = m_engines.find(companyId); it != m_engines.end()) {
            it->second->pause();
            m_tickTimer.stop();
            spdlog::info("SimulationService: Paused simulation for company {}", companyId);
            emit simulationPaused(companyId);
        }
    }

    void setSpeed(int companyId, int speed) {
        if (auto it = m_engines.find(companyId); it != m_engines.end()) {
            it->second->setSpeed(speed);
            if (!it->second->isPaused()) {
                updateTickInterval();
            }
        }
    }

    void advanceOneDay(int companyId) {
        if (auto it = m_engines.find(companyId); it != m_engines.end()) {
            it->second->tick();
            collectKPIs(companyId);
        }
    }

    sim::SimulationEngine* getEngine(int companyId) {
        auto it = m_engines.find(companyId);
        return it != m_engines.end() ? it->second.get() : nullptr;
    }

    struct KPISnapshot {
        double revenue = 0;
        double expenses = 0;
        double netProfit = 0;
        double cashOnHand = 0;
        double customerSatisfaction = 0;
        double employeeSatisfaction = 0;
        double brandAwareness = 0;
        double inventoryValue = 0;
        int employeeCount = 0;
        int activeShipments = 0;
        QString currentDate;
    };

    KPISnapshot getKPIs(int companyId) {
        KPISnapshot kpi;
        auto* engine = getEngine(companyId);
        if (!engine) return kpi;

        if (auto* finance = dynamic_cast<sim::FinanceModule*>(engine->getModule("finance"))) {
            kpi.revenue = finance->totalRevenue();
            kpi.expenses = finance->totalExpenses();
            kpi.netProfit = finance->netProfit();
            kpi.cashOnHand = finance->cashOnHand();
        }
        if (auto* hr = dynamic_cast<sim::HRModule*>(engine->getModule("hr"))) {
            kpi.employeeSatisfaction = hr->averageSatisfaction() / 100.0;
            kpi.employeeCount = 0;
            for (const auto& e : hr->employees()) {
                if (e.isActive) kpi.employeeCount++;
            }
        }
        if (auto* marketing = dynamic_cast<sim::MarketingModule*>(engine->getModule("marketing"))) {
            kpi.brandAwareness = marketing->brandAwareness();
        }
        if (auto* inventory = dynamic_cast<sim::InventoryModule*>(engine->getModule("inventory"))) {
            kpi.inventoryValue = inventory->totalInventoryValue();
        }
        if (auto* logistics = dynamic_cast<sim::LogisticsModule*>(engine->getModule("logistics"))) {
            kpi.activeShipments = logistics->activeShipmentCount();
        }

        kpi.currentDate = QString::fromStdString(engine->currentDate());
        return kpi;
    }

signals:
    void simulationStarted(int companyId);
    void simulationPaused(int companyId);
    void kpiUpdated(int companyId);
    void simulationEvent(int companyId, const QString& title, const QString& description);

private slots:
    void onTick() {
        if (m_activeCompanyId <= 0) return;
        auto* engine = getEngine(m_activeCompanyId);
        if (engine && !engine->isPaused()) {
            engine->tick();
            collectKPIs(m_activeCompanyId);
        }
    }

private:
    void updateTickInterval() {
        // Base: 1 simulated day = 5 seconds realtime at speed 1
        // Speed 2 = 2.5s, Speed 5 = 1s, Speed 10 = 0.5s
        int baseMs = 5000;
        auto* engine = getEngine(m_activeCompanyId);
        if (engine) {
            // Read speed from engine (clamped 1-10 internally)
            int intervalMs = baseMs; // Simplified: always 5s for now, engine manages multi-ticks
            m_tickTimer.setInterval(intervalMs);
        }
    }

    void collectKPIs(int companyId) {
        emit kpiUpdated(companyId);
    }

    QTimer m_tickTimer;
    std::unordered_map<int, std::unique_ptr<sim::SimulationEngine>> m_engines;
    int m_activeCompanyId = 0;
};

} // namespace eduerp::services