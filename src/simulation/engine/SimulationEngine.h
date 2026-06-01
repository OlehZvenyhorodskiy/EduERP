#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <spdlog/spdlog.h>

#include "src/simulation/modules/IModule.h"

namespace eduerp::sim {

/**
 * @brief Central simulation engine that orchestrates all ERP modules.
 *
 * The engine manages time progression, event dispatching, and module ticking.
 * It supports realtime, accelerated, and turn-based time scales.
 * Date management uses std::chrono with proper day advancement.
 */
class SimulationEngine {
public:
    enum class TimeScale { Realtime, Accelerated, TurnBased };

    struct SimulationEvent {
        int id;
        std::string eventType;     // market_change, regulation, competitor, etc.
        std::string title;
        std::string description;
        std::string triggeredAt;
        std::unordered_map<std::string, double> impact;
    };

    using EventCallback = std::function<void(const SimulationEvent&)>;

private:
    std::vector<std::unique_ptr<IModule>> m_modules;
    std::vector<SimulationEvent> m_pendingEvents;
    std::vector<EventCallback> m_eventListeners;

    // Date state — chrono-based
    std::chrono::year_month_day m_currentYMD;
    std::string m_currentDateStr;
    std::string m_startDateStr;
    int m_dayCount = 0;

    TimeScale m_timeScale = TimeScale::Realtime;
    int m_speed = 1;
    bool m_paused = false;
    int m_companyId = 0;
    int m_nextEventId = 1;

public:
    explicit SimulationEngine(int companyId)
        : m_companyId(companyId)
    {
        // Default start date: today
        auto now = std::chrono::system_clock::now();
        auto tp = std::chrono::floor<std::chrono::days>(now);
        m_currentYMD = std::chrono::year_month_day{std::chrono::sys_days{tp}};
        m_currentDateStr = formatDate(m_currentYMD);
        m_startDateStr = m_currentDateStr;
    }

    /**
     * @brief Register a simulation module (Finance, Sales, etc.).
     */
    void addModule(std::unique_ptr<IModule> module) {
        spdlog::info("SimEngine[{}]: Registered module '{}'", m_companyId, module->name());
        m_modules.push_back(std::move(module));
    }

    /**
     * @brief Advance simulation by one day. Called by the simulation clock.
     */
    void tick() {
        if (m_paused) return;

        advanceDate();
        m_dayCount++;

        spdlog::debug("SimEngine[{}]: Day {} — {}", m_companyId, m_dayCount, m_currentDateStr);

        // Tick all enabled modules with current simulated date
        for (auto& module : m_modules) {
            if (module->isEnabled()) {
                module->tick(m_currentDateStr);
            }
        }

        // Generate random market events (roughly 1 in 15 days)
        if (m_dayCount % 15 == 0) {
            generateRandomEvent();
        }

        // Monthly triggers — first of the month
        if (static_cast<unsigned>(m_currentYMD.day()) == 1) {
            onMonthStart();
        }

        // Process any pending events
        for (const auto& event : m_pendingEvents) {
            for (const auto& listener : m_eventListeners) {
                listener(event);
            }
        }
        m_pendingEvents.clear();
    }

    void pause() { m_paused = true; }
    void resume() { m_paused = false; }
    bool isPaused() const { return m_paused; }
    int dayCount() const { return m_dayCount; }

    void setTimeScale(TimeScale scale) { m_timeScale = scale; }
    void setSpeed(int speed) { m_speed = std::clamp(speed, 1, 10); }
    int speed() const { return m_speed; }

    void pushEvent(SimulationEvent event) {
        m_pendingEvents.push_back(std::move(event));
    }

    void addEventListener(EventCallback cb) {
        m_eventListeners.push_back(std::move(cb));
    }

    const std::string& currentDate() const { return m_currentDateStr; }

    IModule* getModule(const std::string& type) {
        for (auto& m : m_modules) {
            if (m->type() == type) return m.get();
        }
        return nullptr;
    }

    const std::vector<std::unique_ptr<IModule>>& modules() const { return m_modules; }

private:
    void advanceDate() {
        // Advance by 1 day using chrono
        auto sysDays = std::chrono::sys_days{m_currentYMD};
        sysDays += std::chrono::days{1};
        m_currentYMD = std::chrono::year_month_day{sysDays};
        m_currentDateStr = formatDate(m_currentYMD);
    }

    static std::string formatDate(const std::chrono::year_month_day& ymd) {
        // Format as "YYYY-MM-DD" ISO 8601
        std::ostringstream oss;
        oss << static_cast<int>(ymd.year()) << '-'
            << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.month()) << '-'
            << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.day());
        return oss.str();
    }

    void onMonthStart() {
        spdlog::info("SimEngine[{}]: New month started — {}", m_companyId, m_currentDateStr);
        // Monthly payroll, rent, insurance deductions happen here
        pushEvent({m_nextEventId++, "monthly_report", "Maandrapport",
                   "Het is de eerste van de maand. Salarissen en vaste kosten worden afgeschreven.",
                   m_currentDateStr, {}});
    }

    void generateRandomEvent() {
        static const std::vector<std::pair<std::string, std::string>> events = {
            {"Marktverandering", "De marktvraag naar je producten is gestegen met 10%."},
            {"Concurrent actie", "Een concurrent heeft zijn prijzen verlaagd met 5%."},
            {"Nieuwe regulering", "Nieuwe BTW-regels zijn van kracht geworden."},
            {"Seizoenseffect", "Seizoensgebonden vraag beïnvloedt je verkoopcijfers."},
            {"Leveringsprobleem", "Je belangrijkste leverancier heeft vertragingen gemeld."},
            {"Positief nieuws", "Een artikel over je bedrijf heeft de naamsbekendheid vergroot."},
            {"Werknemerstevredenheid", "Je team vraagt om betere arbeidsomstandigheden."},
            {"Technologische doorbraak", "Nieuwe technologie kan je productiekosten verlagen."},
        };

        int idx = m_dayCount % static_cast<int>(events.size());
        const auto& [title, desc] = events[idx];

        pushEvent({m_nextEventId++, "market_event", title, desc, m_currentDateStr, {}});
        spdlog::info("SimEngine[{}]: Random event — {}", m_companyId, title);
    }
};

} // namespace eduerp::sim
