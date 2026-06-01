#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <spdlog/spdlog.h>

#include "src/services/simulation/SimulationService.h"

namespace eduerp::ctrl {

/**
 * @brief QML-facing controller that bridges SimulationService ↔ QML views.
 *        All KPI properties are live-bound via Q_PROPERTY and update on every tick.
 */
class SimulationController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // Status
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY statusChanged)
    Q_PROPERTY(bool hasActiveSimulation READ hasActiveSimulation NOTIFY companyChanged)
    Q_PROPERTY(int speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY kpiChanged)

    // KPIs — live updated
    Q_PROPERTY(double revenue READ revenue NOTIFY kpiChanged)
    Q_PROPERTY(double expenses READ expenses NOTIFY kpiChanged)
    Q_PROPERTY(double netProfit READ netProfit NOTIFY kpiChanged)
    Q_PROPERTY(double cashOnHand READ cashOnHand NOTIFY kpiChanged)
    Q_PROPERTY(double customerSatisfaction READ customerSatisfaction NOTIFY kpiChanged)
    Q_PROPERTY(double employeeSatisfaction READ employeeSatisfaction NOTIFY kpiChanged)
    Q_PROPERTY(double brandAwareness READ brandAwareness NOTIFY kpiChanged)
    Q_PROPERTY(double inventoryValue READ inventoryValue NOTIFY kpiChanged)
    Q_PROPERTY(int employeeCount READ employeeCount NOTIFY kpiChanged)
    Q_PROPERTY(int activeShipments READ activeShipments NOTIFY kpiChanged)

public:
    explicit SimulationController(QObject* parent = nullptr) : QObject(parent) {}

    void setSimulationService(services::SimulationService* service) {
        m_service = service;
        connect(m_service, &services::SimulationService::kpiUpdated,
                this, &SimulationController::onKPIUpdated);
        connect(m_service, &services::SimulationService::simulationStarted,
                this, [this](int) { m_isRunning = true; emit statusChanged(); });
        connect(m_service, &services::SimulationService::simulationPaused,
                this, [this](int) { m_isRunning = false; emit statusChanged(); });
        connect(m_service, &services::SimulationService::simulationEvent,
                this, &SimulationController::onSimulationEvent);
    }

    // ── Property getters ──
    bool isRunning() const { return m_isRunning; }
    bool hasActiveSimulation() const { return m_activeCompanyId > 0; }
    int speed() const { return m_speed; }
    QString currentDate() const { return m_snapshot.currentDate; }
    double revenue() const { return m_snapshot.revenue; }
    double expenses() const { return m_snapshot.expenses; }
    double netProfit() const { return m_snapshot.netProfit; }
    double cashOnHand() const { return m_snapshot.cashOnHand; }
    double customerSatisfaction() const { return m_snapshot.customerSatisfaction; }
    double employeeSatisfaction() const { return m_snapshot.employeeSatisfaction; }
    double brandAwareness() const { return m_snapshot.brandAwareness; }
    double inventoryValue() const { return m_snapshot.inventoryValue; }
    int employeeCount() const { return m_snapshot.employeeCount; }
    int activeShipments() const { return m_snapshot.activeShipments; }

    void setSpeed(int speed) {
        m_speed = qBound(1, speed, 10);
        if (m_service && m_activeCompanyId > 0) {
            m_service->setSpeed(m_activeCompanyId, m_speed);
        }
        emit speedChanged();
    }

    // ── Q_INVOKABLE actions for QML ──

    Q_INVOKABLE void createAndStartSimulation(int companyId, const QString& industryTemplate) {
        if (!m_service) return;
        m_service->createSimulation(companyId, industryTemplate.toStdString());
        m_activeCompanyId = companyId;
        emit companyChanged();

        // Take initial KPI snapshot
        m_snapshot = m_service->getKPIs(companyId);
        emit kpiChanged();
    }

    Q_INVOKABLE void startSimulation() {
        if (m_service && m_activeCompanyId > 0) {
            m_service->start(m_activeCompanyId);
        }
    }

    Q_INVOKABLE void pauseSimulation() {
        if (m_service && m_activeCompanyId > 0) {
            m_service->pause(m_activeCompanyId);
        }
    }

    Q_INVOKABLE void advanceOneDay() {
        if (m_service && m_activeCompanyId > 0) {
            m_service->advanceOneDay(m_activeCompanyId);
        }
    }

    // ── Finance actions ──
    Q_INVOKABLE void recordTransaction(const QString& description, double amount,
                                       const QString& debitAccount, const QString& creditAccount)
    {
        if (!m_service) return;
        auto* engine = m_service->getEngine(m_activeCompanyId);
        if (!engine) return;

        auto* finance = dynamic_cast<sim::FinanceModule*>(engine->getModule("finance"));
        if (finance) {
            // Map account names to IDs (simplified)
            int debitId = 1; // Default to bank account
            int creditId = 5; // Default to revenue
            finance->recordTransaction(currentDate().toStdString(), debitId, creditId, amount,
                                       description.toStdString());
            refreshKPIs();
        }
    }

    // ── HR actions ──
    Q_INVOKABLE void hireEmployee(const QString& name, const QString& department,
                                  const QString& position, double salary)
    {
        if (!m_service) return;
        auto* engine = m_service->getEngine(m_activeCompanyId);
        if (!engine) return;

        auto* hr = dynamic_cast<sim::HRModule*>(engine->getModule("hr"));
        if (hr) {
            hr->hireEmployee(name.toStdString(), department.toStdString(),
                             position.toStdString(), salary);
            refreshKPIs();
        }
    }

    Q_INVOKABLE void fireEmployee(int employeeId) {
        if (!m_service) return;
        auto* engine = m_service->getEngine(m_activeCompanyId);
        if (!engine) return;

        auto* hr = dynamic_cast<sim::HRModule*>(engine->getModule("hr"));
        if (hr) {
            hr->fireEmployee(employeeId);
            refreshKPIs();
        }
    }

    // ── Inventory actions ──
    Q_INVOKABLE void addProduct(const QString& sku, const QString& name,
                                double costPrice, double sellingPrice, int initialStock)
    {
        if (!m_service) return;
        auto* engine = m_service->getEngine(m_activeCompanyId);
        if (!engine) return;

        auto* inventory = dynamic_cast<sim::InventoryModule*>(engine->getModule("inventory"));
        if (inventory) {
            sim::Product product;
            product.id = static_cast<int>(inventory->products().size()) + 1;
            product.sku = sku.toStdString();
            product.name = name.toStdString();
            product.costPrice = costPrice;
            product.sellingPrice = sellingPrice;
            product.currentStock = initialStock;
            inventory->addProduct(product);
            refreshKPIs();
        }
    }

    // ── Sales actions ──
    Q_INVOKABLE void createSalesOrder(int customerId, double amount) {
        if (!m_service) return;
        auto* engine = m_service->getEngine(m_activeCompanyId);
        if (!engine) return;

        auto* sales = dynamic_cast<sim::SalesModule*>(engine->getModule("sales"));
        auto* finance = dynamic_cast<sim::FinanceModule*>(engine->getModule("finance"));
        if (sales && finance) {
            auto order = sales->createOrder(customerId, amount);
            // Record revenue in finance
            finance->recordTransaction(currentDate().toStdString(), 1, 5, order.totalAmount,
                                       "Verkooporder " + order.orderNumber, "invoice", order.id);
            refreshKPIs();
        }
    }

    // ── Marketing actions ──
    Q_INVOKABLE void launchCampaign(const QString& name, const QString& type,
                                    double budget, int durationDays)
    {
        if (!m_service) return;
        auto* engine = m_service->getEngine(m_activeCompanyId);
        if (!engine) return;

        auto* marketing = dynamic_cast<sim::MarketingModule*>(engine->getModule("marketing"));
        auto* finance = dynamic_cast<sim::FinanceModule*>(engine->getModule("finance"));
        if (marketing && finance) {
            marketing->launchCampaign(name.toStdString(), type.toStdString(), budget, durationDays);
            // Debit marketing expense
            finance->recordTransaction(currentDate().toStdString(), 9, 1, budget,
                                       "Marketingcampagne: " + name.toStdString());
            refreshKPIs();
        }
    }

    // ── Logistics actions ──
    Q_INVOKABLE void createShipment(int orderId, const QString& origin, const QString& destination,
                                    double weight)
    {
        if (!m_service) return;
        auto* engine = m_service->getEngine(m_activeCompanyId);
        if (!engine) return;

        auto* logistics = dynamic_cast<sim::LogisticsModule*>(engine->getModule("logistics"));
        if (logistics) {
            logistics->createShipment(orderId, origin.toStdString(), destination.toStdString(), weight);
            refreshKPIs();
        }
    }

signals:
    void statusChanged();
    void companyChanged();
    void speedChanged();
    void kpiChanged();
    void eventReceived(const QString& title, const QString& description);

private slots:
    void onKPIUpdated(int companyId) {
        if (companyId == m_activeCompanyId) {
            refreshKPIs();
        }
    }

    void onSimulationEvent(int companyId, const QString& title, const QString& description) {
        if (companyId == m_activeCompanyId) {
            emit eventReceived(title, description);
        }
    }

private:
    void refreshKPIs() {
        if (m_service && m_activeCompanyId > 0) {
            m_snapshot = m_service->getKPIs(m_activeCompanyId);
            emit kpiChanged();
        }
    }

    services::SimulationService* m_service = nullptr;
    services::SimulationService::KPISnapshot m_snapshot;
    int m_activeCompanyId = 0;
    bool m_isRunning = false;
    int m_speed = 1;
};

} // namespace eduerp::ctrl
