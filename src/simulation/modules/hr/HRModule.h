#pragma once

#include "src/simulation/modules/IModule.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace eduerp::sim {

struct Employee {
    int id = 0;
    std::string name;
    std::string department;  // sales, production, admin, management, marketing
    std::string position;
    double salary = 2500.00;
    int satisfactionScore = 70; // 0-100
    int performanceScore = 70;  // 0-100
    int skillLevel = 50;        // 0-100
    bool isActive = true;
    std::string hiredAt;
};

class HRModule : public IModule {
private:
    bool m_enabled = true;
    std::vector<Employee> m_employees;
    double m_totalPayroll = 0;
    int m_nextEmployeeId = 1;

public:
    std::string name() const override { return "Human Resources"; }
    std::string type() const override { return "hr"; }

    void initialize() override {
        spdlog::info("HRModule: Initializing HR system");
    }

    void tick(const std::string& simulatedDate) override {
        spdlog::debug("HRModule: Processing day {}", simulatedDate);
        // Update satisfaction based on workload, pay, and conditions
        // Check for resignation triggers
        // Process training effects on skill levels
    }

    int hireEmployee(const std::string& empName, const std::string& dept,
                     const std::string& pos, double salary)
    {
        Employee emp;
        emp.id = m_nextEmployeeId++;
        emp.name = empName;
        emp.department = dept;
        emp.position = pos;
        emp.salary = salary;
        emp.satisfactionScore = 70;
        emp.performanceScore = 50;
        emp.skillLevel = 50;
        emp.isActive = true;
        m_employees.push_back(emp);
        return emp.id;
    }

    bool fireEmployee(int empId) {
        for (auto& e : m_employees) {
            if (e.id == empId && e.isActive) {
                e.isActive = false;
                return true;
            }
        }
        return false;
    }

    double calculateMonthlyPayroll() const {
        double total = 0;
        for (const auto& e : m_employees) {
            if (e.isActive) total += e.salary;
        }
        return total;
    }

    double averageSatisfaction() const {
        if (m_employees.empty()) return 0;
        double sum = 0;
        int count = 0;
        for (const auto& e : m_employees) {
            if (e.isActive) {
                sum += e.satisfactionScore;
                count++;
            }
        }
        return count > 0 ? sum / count : 0;
    }

    const std::vector<Employee>& employees() const { return m_employees; }

    std::string serializeState() const override {
        nlohmann::json j;
        j["employee_count"] = m_employees.size();
        j["monthly_payroll"] = calculateMonthlyPayroll();
        j["avg_satisfaction"] = averageSatisfaction();
        return j.dump();
    }

    void deserializeState(const std::string& json) override {}
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }
};

} // namespace eduerp::sim