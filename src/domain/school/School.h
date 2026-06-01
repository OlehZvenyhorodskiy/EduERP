#pragma once

#include <string>
#include <vector>

namespace eduerp::domain {

struct School {
    int id = 0;
    std::string name;
    std::string subdomain;
    std::vector<std::string> oauthDomains;
    std::string defaultLanguage = "nl-BE";
    std::vector<std::string> allowedLanguages = {"nl-BE", "en-GB", "fr-BE"};
    bool streakEnabled = true;
    bool friendSystemEnabled = true;
    bool crossClassMessaging = false;
    bool energySavingDefault = false;
    std::string animationDefault = "full";
    bool isActive = true;
};

struct Class {
    int id = 0;
    int schoolId = 0;
    std::string name;
    std::string description;
    std::string academicYear;
    int teacherId = 0;
    int maxTeamSize = 4;
    std::vector<std::string> allowedModules = {"finance", "sales", "inventory", "hr", "marketing", "logistics"};
    std::string simulationTimeScale = "realtime";
    bool isActive = true;
};

struct Team {
    int id = 0;
    int classId = 0;
    std::string name;
    std::string companyName;
    int currentSimulationId = 0;
    bool isActive = true;
};

struct TeamMembership {
    int id = 0;
    int teamId = 0;
    int studentId = 0;
    std::string role; // ceo, cfo, sales_manager, etc.
    std::string joinedAt;
};

} // namespace eduerp::domain
