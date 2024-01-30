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

        loadMenus();
        logger::info("All menu names are loaded.");
    }

    void loadMenus() {
        auto utility = Utility::GetSingleton();
        utility->menuList.push_back(RE::MainMenu::MENU_NAME);
        utility->menuList.push_back(RE::DialogueMenu::MENU_NAME);
        utility->menuList.push_back(RE::CraftingMenu::MENU_NAME);
        utility->menuList.push_back(RE::MagicMenu::MENU_NAME);
        utility->menuList.push_back(RE::LockpickingMenu::MENU_NAME);
        utility->menuList.push_back(RE::InventoryMenu::MENU_NAME);
        utility->menuList.push_back(RE::LoadingMenu::MENU_NAME);
        utility->menuList.push_back(RE::JournalMenu::MENU_NAME);
        utility->menuList.push_back(RE::BarterMenu::MENU_NAME);
        utility->menuList.push_back(RE::ContainerMenu::MENU_NAME);
        utility->menuList.push_back(RE::BookMenu::MENU_NAME);
        utility->menuList.push_back(RE::MapMenu::MENU_NAME);
        utility->menuList.push_back(RE::GiftMenu::MENU_NAME);
        utility->menuList.push_back(RE::FavoritesMenu::MENU_NAME);
        utility->menuList.push_back(RE::CreationClubMenu::MENU_NAME);
        utility->menuList.push_back(RE::ModManagerMenu::MENU_NAME);
        utility->menuList.push_back(RE::RaceSexMenu::MENU_NAME);
        utility->menuList.push_back(RE::StatsMenu::MENU_NAME);
        utility->menuList.push_back(RE::TrainingMenu::MENU_NAME);
        utility->menuList.push_back(RE::TutorialMenu::MENU_NAME);
        utility->menuList.push_back(RE::TweenMenu::MENU_NAME);
        utility->menuList.push_back(RE::CursorMenu::MENU_NAME);
    }

    void LoadMiscForms([[maybe_unused]] RE::TESDataHandler* dataHandler) {
        auto utility = Utility::GetSingleton();

        // Keywords
        utility->keywordWarhammer = dataHandler->LookupForm(RE::FormID(0x06D930), skyrimPluginName)->As<RE::BGSKeyword>();
        utility->keywordClothing = dataHandler->LookupForm(RE::FormID(0x08F95B), skyrimPluginName)->As<RE::BGSKeyword>();
        utility->keywordJewelry = dataHandler->LookupForm(RE::FormID(0x08F95A), skyrimPluginName)->As<RE::BGSKeyword>();
        utility->keywordMagicDisallow = dataHandler->LookupForm(RE::FormID(0x0C27BD), skyrimPluginName)->As<RE::BGSKeyword>();

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