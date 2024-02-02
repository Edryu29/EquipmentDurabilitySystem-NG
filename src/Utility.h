#pragma once
#include "Settings.h"

class Utility {
public:
    RE::BGSKeyword* keywordWarhammer;
    RE::BGSKeyword* keywordClothing;
    RE::BGSKeyword* keywordJewelry;
    RE::BGSKeyword* keywordMagicDisallow;

    RE::TESFaction* factionFollower1;
    RE::TESFaction* factionFollower2;

    RE::BGSLocationRefType* locationBoss;
    RE::BGSLocationRefType* locationBossContainer;

    std::vector<RE::BSFixedString> menuList;

    uintptr_t PlayerSingletonAddress;
    uintptr_t UISingletonAddress;
    uintptr_t MenuControlsSingletonAddress;

    static Utility* GetSingleton() {
        static Utility playerStatus;
        return &playerStatus; 
    }

    static RE::PlayerCharacter* GetPlayer() {
        REL::Relocation<RE::NiPointer<RE::PlayerCharacter>*> singleton{Utility::GetSingleton()->PlayerSingletonAddress};
        return singleton->get();
    }

    static RE::UI* GetUI() {
        REL::Relocation<RE::NiPointer<RE::UI>*> singleton{Utility::GetSingleton()->UISingletonAddress};
        return singleton->get();
    }

    static RE::MenuControls* GetMenuControls() {
        REL::Relocation<RE::NiPointer<RE::MenuControls>*> singleton{
            Utility::GetSingleton()->MenuControlsSingletonAddress};
        return singleton->get();
    }

    static void ShowNotification(std::string msg, bool messageBox = false, const char* a_soundToPlay = 0) {
        if (messageBox) {
            RE::DebugMessageBox(msg.c_str());
        } else {
            RE::DebugNotification(msg.c_str(), a_soundToPlay);
        }
    }

    static bool IsSystemMenu(RE::BSFixedString menuName) {
        auto menu = Utility::GetSingleton()->menuList;
        return (std::find(menu.begin(), menu.end(), menuName) != menu.end());
    }

    static bool IsSystemMenuOpen(RE::BSFixedString menuName = "") {
        auto ui = Utility::GetSingleton()->GetUI();
        auto menu = Utility::GetSingleton()->menuList;

        for (auto & element : menu) {
            if (menuName.c_str() != "" && std::strcmp(element.c_str(), menuName.c_str()))
                continue;

            if (ui->IsMenuOpen(element))
                return true;
        }

        return false;
    }

    // Player checks
    static bool IsPlayerInDialogue() {
        return Utility::GetSingleton()->GetUI()->IsMenuOpen(RE::DialogueMenu::MENU_NAME);
    }

    static bool PlayerNotInMenu() {
        auto ui = Utility::GetSingleton()->GetUI();

        if (ui && !ui->GameIsPaused() && !ui->IsApplicationMenuOpen() && !ui->IsItemMenuOpen() && !ui->IsMenuOpen(RE::InterfaceStrings::GetSingleton()->dialogueMenu))
            return true;
        else
            return false;
    }
};
