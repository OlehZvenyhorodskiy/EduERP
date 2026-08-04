#pragma once

#include "src/simulation/modules/IModule.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace eduerp::sim {

struct MarketingCampaign {
    int id = 0;
    std::string name;
    std::string type;       // social_media, print, event, email, sponsoring
    double budget = 0;
    double spentAmount = 0;
    int durationDays = 30;
    int remainingDays = 30;
    double reachMultiplier = 1.0;   // How effective this campaign is
    bool isActive = true;
};

class MarketingModule : public IModule {
private:
    bool m_enabled = true;
    std::vector<MarketingCampaign> m_campaigns;
    double m_brandAwareness = 0.05;   // 0.0 to 1.0
    double m_customerAcquisitionRate = 0.0;
    int m_nextCampaignId = 1;

public:
    std::string name() const override { return "Marketing"; }
    std::string type() const override { return "marketing"; }

    void initialize() override {
        spdlog::info("MarketingModule: Initializing marketing system");
    }

    void tick(const std::string& simulatedDate) override {
        spdlog::debug("MarketingModule: Processing day {}", simulatedDate);
        // Decrease remaining days on active campaigns
        // Calculate brand awareness change based on active campaigns
        // Adjust customer acquisition rate
        for (auto& campaign : m_campaigns) {
            if (campaign.isActive && campaign.remainingDays > 0) {
                campaign.remainingDays--;
                campaign.spentAmount += campaign.budget / campaign.durationDays;
                m_brandAwareness += 0.001 * campaign.reachMultiplier;
                if (campaign.remainingDays == 0) {
                    campaign.isActive = false;
                }
            }
        }
        m_brandAwareness = std::clamp(m_brandAwareness, 0.0, 1.0);
    }

    int launchCampaign(const std::string& campaignName, const std::string& campaignType,
                       double budget, int durationDays)
    {
        MarketingCampaign campaign;
        campaign.id = m_nextCampaignId++;
        campaign.name = campaignName;
        campaign.type = campaignType;
        campaign.budget = budget;
        campaign.durationDays = durationDays;
        campaign.remainingDays = durationDays;
        campaign.reachMultiplier = calculateReachMultiplier(campaignType, budget);
        campaign.isActive = true;
        m_campaigns.push_back(campaign);
        return campaign.id;
    }

    double brandAwareness() const { return m_brandAwareness; }

    std::string serializeState() const override {
        nlohmann::json j;
        j["brand_awareness"] = m_brandAwareness;
        j["active_campaigns"] = 0;
        for (const auto& c : m_campaigns) {
            if (c.isActive) j["active_campaigns"] = j["active_campaigns"].get<int>() + 1;
        }
        return j.dump();
    }

    void deserializeState(const std::string& json) override {}
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }

private:
    double calculateReachMultiplier(const std::string& type, double budget) {
        double base = 1.0;
        if (type == "social_media") base = 1.5;
        else if (type == "event") base = 2.0;
        else if (type == "print") base = 0.8;
        else if (type == "email") base = 1.2;
        // Higher budget = diminishing returns
        return base * (1.0 + std::log10(std::max(budget, 100.0)) / 5.0);
    }
};

} // namespace eduerp::sim