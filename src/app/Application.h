#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>
#include <memory>
#include <spdlog/spdlog.h>

#include "src/infrastructure/config/ConfigManager.h"
#include "src/infrastructure/logging/Logger.h"
#include "src/infrastructure/network/HttpClient.h"
#include "src/infrastructure/network/WebSocketClient.h"
#include "src/infrastructure/storage/LocalCache.h"
#include "src/services/auth/AuthService.h"
#include "src/services/simulation/SimulationService.h"
#include "src/infrastructure/i18n/Translator.h"
#include "src/controllers/AuthController.h"
#include "src/controllers/SimulationController.h"
#include "src/controllers/CompanyController.h"
#include "src/controllers/ProfileController.h"
#include "src/controllers/DashboardController.h"
#include "src/controllers/MessagingController.h"
#include "src/controllers/GamificationController.h"
#include "src/services/gamification/GamificationService.h"

namespace eduerp::app {

/**
 * @brief Application coordinator — the composition root.
 *        Creates all services and controllers, wires dependencies,
 *        and registers them with the QML engine.
 *
 *        This is the ONLY place where ownership and dependency injection happens.
 */
class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(QQmlEngine* engine, QObject* parent = nullptr)
        : QObject(parent)
        , m_engine(engine)
    {
        spdlog::info("Application: Initializing composition root...");
        createServices();
        createControllers();
        wireConnections();
        registerWithQml();
        spdlog::info("Application: All systems initialized");
    }

    ~Application() override {
        spdlog::info("Application: Shutting down...");
    }

private:
    void createServices() {
        m_config = std::make_unique<infra::ConfigManager>();
        m_config->load();

        m_httpClient = std::make_unique<infra::HttpClient>(this);
        m_httpClient->setBaseUrl(QString::fromStdString(
            m_config->getString("api_base_url", "http://localhost:8080/api/v1")));

        m_webSocket = std::make_unique<infra::WebSocketClient>(this);

        m_localCache = std::make_unique<infra::LocalCache>();
        m_localCache->migrate();

        m_authService = std::make_unique<services::AuthService>(this);
        m_simulationService = std::make_unique<services::SimulationService>(this);

        // Initialize i18n — load the matching JSON bundle for the configured locale.
        // Defaults to nl-BE; overridden by school default_language after login.
        m_translator = std::make_unique<infra::Translator>(this);
        m_translator->setLocale(QString::fromStdString(
            m_config->getString("default_language", "nl-BE")));

        // Initialize gamification
        m_gamificationService = std::make_unique<services::GamificationService>(this);
        m_gamificationService->registerDailyLogin(); // register streak on app launch
    }

    void createControllers() {
        m_authController = std::make_unique<ctrl::AuthController>(this);
        m_simulationController = std::make_unique<ctrl::SimulationController>(this);
        m_companyController = std::make_unique<ctrl::CompanyController>(this);
        m_profileController = std::make_unique<ctrl::ProfileController>(this);
        m_dashboardController = std::make_unique<ctrl::DashboardController>(this);
        m_messagingController = std::make_unique<ctrl::MessagingController>(this);
        m_gamificationController = std::make_unique<ctrl::GamificationController>(this);

        // Wire SimulationController ↔ SimulationService
        m_simulationController->setSimulationService(m_simulationService.get());

        // Wire GamificationController ↔ GamificationService
        m_gamificationController->setService(m_gamificationService.get());
    }

    void wireConnections() {
        // When auth succeeds, propagate token to all services
        connect(m_authController.get(), &ctrl::AuthController::loginSuccessful, this, [this]() {
            QString token = m_authController->accessToken();
            m_httpClient->setAccessToken(token);
            m_companyController->setAccessToken(token);
            m_profileController->setAccessToken(token);

            // Store tokens for auto-login
            m_authService->storeTokens(token, ""); // TODO: pass refresh token

            // Connect WebSocket
            QString wsUrl = QString::fromStdString(
                m_config->getString("websocket_url", "ws://localhost:8080/ws"));
            m_webSocket->connectToServer(wsUrl, token);

            // Load user profile
            m_profileController->fetchProfile();

            spdlog::info("Application: Post-login initialization complete");
        });

        // When auth state changes to logged out, clean up
        connect(m_authController.get(), &ctrl::AuthController::authStateChanged, this, [this]() {
            if (!m_authController->isLoggedIn()) {
                m_authService->clearTokens();
                m_webSocket->disconnect();
                m_httpClient->setAccessToken("");
                spdlog::info("Application: Post-logout cleanup complete");
            }
        });

        // When HTTP gets 401, trigger token refresh
        connect(m_httpClient.get(), &infra::HttpClient::unauthorizedResponse, this, [this]() {
            spdlog::warn("Application: Received 401, attempting token refresh");
            m_authController->refreshToken();
        });

        // When token refreshed, update everywhere
        connect(m_authController.get(), &ctrl::AuthController::tokenRefreshed, this, [this]() {
            QString token = m_authController->accessToken();
            m_httpClient->setAccessToken(token);
            m_companyController->setAccessToken(token);
            m_profileController->setAccessToken(token);
        });

        // When company created, auto-create simulation
        connect(m_companyController.get(), &ctrl::CompanyController::companyCreated, this, [this]() {
            spdlog::info("Application: Company created, simulation will be initialized on demand");
            m_gamificationService->onCompanyCreated();
        });
    }

    void registerWithQml() {
        auto* ctx = m_engine->rootContext();

        ctx->setContextProperty("authController", m_authController.get());
        ctx->setContextProperty("simulationController", m_simulationController.get());
        ctx->setContextProperty("companyController", m_companyController.get());
        ctx->setContextProperty("profileController", m_profileController.get());
        ctx->setContextProperty("dashboardController", m_dashboardController.get());
        ctx->setContextProperty("messagingController", m_messagingController.get());
        ctx->setContextProperty("translator", m_translator.get());
        ctx->setContextProperty("gamificationController", m_gamificationController.get());

        spdlog::info("Application: All controllers + services registered with QML context");
    }

    QQmlEngine* m_engine;

    // Infrastructure
    std::unique_ptr<infra::ConfigManager> m_config;
    std::unique_ptr<infra::HttpClient> m_httpClient;
    std::unique_ptr<infra::WebSocketClient> m_webSocket;
    std::unique_ptr<infra::LocalCache> m_localCache;

    // Services
    std::unique_ptr<services::AuthService> m_authService;
    std::unique_ptr<services::SimulationService> m_simulationService;
    std::unique_ptr<infra::Translator> m_translator;
    std::unique_ptr<services::GamificationService> m_gamificationService;

    // Controllers
    std::unique_ptr<ctrl::AuthController> m_authController;
    std::unique_ptr<ctrl::SimulationController> m_simulationController;
    std::unique_ptr<ctrl::CompanyController> m_companyController;
    std::unique_ptr<ctrl::ProfileController> m_profileController;
    std::unique_ptr<ctrl::DashboardController> m_dashboardController;
    std::unique_ptr<ctrl::MessagingController> m_messagingController;
    std::unique_ptr<ctrl::GamificationController> m_gamificationController;
};

} // namespace eduerp::app
