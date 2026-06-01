#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace eduerp::domain {

enum class UserRole {
    SuperAdmin,
    SchoolAdmin,
    Teacher,
    Student
};

inline std::string userRoleToString(UserRole role) {
    switch (role) {
        case UserRole::SuperAdmin: return "super_admin";
        case UserRole::SchoolAdmin: return "school_admin";
        case UserRole::Teacher: return "teacher";
        case UserRole::Student: return "student";
    }
    return "unknown";
}

struct UserSettings {
    std::string language = "nl-BE";
    std::string theme = "system";
    std::string fontSize = "medium";    // small, medium, large
    std::string animationPref = "full"; // full, reduced, none
    bool energySavingMode = false;
};

struct UserPrivacy {
    std::string profileVisibility = "friends"; // everyone, friends, class, teacher
    std::string friendRequestsAllowed = "class"; // everyone, class, disabled
};

struct User {
    int id = 0;
    int schoolId = 0;
    std::string email;
    std::string oauthProvider;  // google, microsoft
    std::string oauthSubject;
    UserRole role = UserRole::Student;

    std::string displayName;
    std::string username;
    std::string avatarUrl;
    std::string bannerUrl;
    std::string bio;

    UserSettings settings;
    UserPrivacy privacy;

    bool isActive = true;
    std::string lastLoginAt;
    std::string createdAt;
};

} // namespace eduerp::domain
