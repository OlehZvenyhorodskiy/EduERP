#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace eduerp::sim {

/**
 * @brief Interface for all ERP simulation modules.
 *        Each module (Finance, Sales, HR, etc.) implements this interface.
 *        The SimulationEngine ticks each module on every simulation step.
 */
class IModule {
public:
    virtual ~IModule() = default;

    virtual std::string name() const = 0;
    virtual std::string type() const = 0;

    /**
     * @brief Called once when the module is first activated.
     */
    virtual void initialize() = 0;

    /**
     * @brief Advance the simulation by one tick (1 simulated day).
     * @param simulatedDate The current simulated date as ISO string.
     */
    virtual void tick(const std::string& simulatedDate) = 0;

    /**
     * @brief Serialize the module's current state to JSON for persistence.
     */
    virtual std::string serializeState() const = 0;

    /**
     * @brief Restore state from a JSON string.
     */
    virtual void deserializeState(const std::string& json) = 0;

    /**
     * @brief Whether this module is currently enabled.
     */
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

} // namespace eduerp::sim
