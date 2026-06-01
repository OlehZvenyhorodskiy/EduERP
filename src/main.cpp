#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QFont>
#include <QFontDatabase>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <spdlog/spdlog.h>

#include "infrastructure/logging/Logger.h"
#include "app/Application.h"

int main(int argc, char* argv[])
{

    QGuiApplication app(argc, argv);
    app.setApplicationName("EduERP");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("EduERP");
    app.setOrganizationDomain("eduerp.be");

    // Initialize logging first — everything else logs through this
    eduerp::infra::Logger::init();
    spdlog::info("═══════════════════════════════════════════");
    spdlog::info("  EduERP v{} — Starting...", app.applicationVersion().toStdString());
    spdlog::info("═══════════════════════════════════════════");

    // Dump all QRC resources so we can verify main.qml is embedded
    spdlog::info("Listing QRC resources under :/EduERP/");
    QDirIterator it(":/EduERP", QDirIterator::Subdirectories);
    int resourceCount = 0;
    while (it.hasNext()) {
        QString path = it.next();
        if (path.contains("main.qml")) {
            spdlog::info("  [FOUND] {}", path.toStdString());
        }
        resourceCount++;
    }
    spdlog::info("Total QRC resources under :/EduERP: {}", resourceCount);

    // Also check alternative prefix paths
    QDirIterator it2(":/qt/qml/EduERP", QDirIterator::Subdirectories);
    int altCount = 0;
    while (it2.hasNext()) {
        QString path = it2.next();
        if (path.contains("main.qml")) {
            spdlog::info("  [FOUND ALT] {}", path.toStdString());
        }
        altCount++;
    }
    spdlog::info("Total QRC resources under :/qt/qml/EduERP: {}", altCount);

    // Check if the specific file exists
    bool mainExists = QFile::exists(":/EduERP/src/ui/main.qml");
    bool mainAltExists = QFile::exists(":/qt/qml/EduERP/src/ui/main.qml");
    spdlog::info("main.qml exists at qrc:/EduERP/src/ui/main.qml: {}", mainExists);
    spdlog::info("main.qml exists at qrc:/qt/qml/EduERP/src/ui/main.qml: {}", mainAltExists);

    // Set default font to Inter (falls back to system sans-serif)
    QFont defaultFont("Inter", 13);
    defaultFont.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(defaultFont);

    // Set Qt Quick style to Basic for custom styling
    QQuickStyle::setStyle("Basic");

    // Create QML engine
    QQmlApplicationEngine engine;

    // Hook into QML warnings so we can see parse errors
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError>& warnings) {
        for (const auto& w : warnings) {
            spdlog::error("QML Warning: {}", w.toString().toStdString());
        }
    });

    // Create the Application coordinator — the composition root.
    // This initializes all services, controllers, and wires them together.
    eduerp::app::Application application(&engine);

    // Load the main QML file
    QUrl url;
    if (mainExists) {
        url = QUrl("qrc:/EduERP/src/ui/main.qml");
    } else if (mainAltExists) {
        url = QUrl("qrc:/qt/qml/EduERP/src/ui/main.qml");
    } else {
        spdlog::error("FATAL: main.qml not found at ANY expected resource path!");
        spdlog::info("Dumping all root-level QRC paths:");
        QDirIterator rootIt(":/", QDir::Dirs | QDir::NoDotAndDotDot);
        while (rootIt.hasNext()) {
            spdlog::info("  root: {}", rootIt.next().toStdString());
        }
        eduerp::infra::Logger::shutdown();
        return -1;
    }

    spdlog::info("Loading QML from: {}", url.toString().toStdString());

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            spdlog::error("FATAL: QML object creation failed — check resource paths");
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        spdlog::error("FATAL: No root QML objects loaded from {}", url.toString().toStdString());
        eduerp::infra::Logger::shutdown();
        return -1;
    }

    spdlog::info("EduERP: Application loaded successfully");
    spdlog::info("  Modules: Finance, Sales, Inventory, HR, Marketing, Logistics");
    spdlog::info("  Backend: http://localhost:8080/api/v1");
    spdlog::info("  WebSocket: ws://localhost:8080/ws");

    int result = app.exec();

    spdlog::info("EduERP: Shutting down (exit code {})", result);
    eduerp::infra::Logger::shutdown();
    return result;
}
