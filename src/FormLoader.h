#pragma once

#include "Utility.h"
#undef AddForm

class FormLoader {
public:
    const std::string_view skyrimPluginName = "Skyrim.esm";
    const std::string_view updatePluginName = "Update.esm";
    const std::string_view dgPluginName = "Dawnguard.esm";
    const std::string_view hfPluginName = "HearthFires.esm";

    static FormLoader* GetSingleton() {
        static FormLoader formLoader;
        return &formLoader;
    }

    void LoadAllForms() {
        logger::info("Loading all forms.");

        const auto dataHandler = RE::TESDataHandler::GetSingleton();

        LoadMiscForms(dataHandler);
        logger::info("All forms are loaded.");
    }

    void LoadMiscForms([[maybe_unused]] RE::TESDataHandler* dataHandler) {
        auto utility = Utility::GetSingleton();

        // Keywords
        utility->keywordWarhammer = dataHandler->LookupForm(RE::FormID(0x06D930), skyrimPluginName)->As<RE::BGSKeyword>();
        utility->keywordClothing = dataHandler->LookupForm(RE::FormID(0x08F95B), skyrimPluginName)->As<RE::BGSKeyword>();
        utility->keywordJewelry = dataHandler->LookupForm(RE::FormID(0x08F95A), skyrimPluginName)->As<RE::BGSKeyword>();

        // Follower Factions
        utility->factionFollower1 = dataHandler->LookupForm(RE::FormID(0x05C84D), skyrimPluginName)->As<RE::TESFaction>();
        utility->factionFollower2 = dataHandler->LookupForm(RE::FormID(0x09A7B2), skyrimPluginName)->As<RE::TESFaction>();

        // Locations
        utility->locationBoss = dataHandler->LookupForm(RE::FormID(0x0130F7), skyrimPluginName)->As<RE::BGSLocationRefType>();
        utility->locationBossContainer = dataHandler->LookupForm(RE::FormID(0x0130F8), skyrimPluginName)->As<RE::BGSLocationRefType>();
    }

    // Cache commonly called addresses to avoid address lib overhead
    void CacheGameAddresses() {
        auto utility = Utility::GetSingleton();

        // Cache player singleton address
        utility->PlayerSingletonAddress = RELOCATION_ID(517014, 403521).address();
        utility->UISingletonAddress = RELOCATION_ID(514178, 400327).address();
        utility->MenuControlsSingletonAddress = RELOCATION_ID(515124, 401263).address();
    }
};