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

    static void ShowNotification(std::string msg, bool messageBox = false) {
        if (messageBox) {
            RE::DebugMessageBox(msg.c_str());
        } else {
            RE::DebugNotification(msg.c_str());
        }
    }

    // Player checks
    static bool IsPlayerInDialogue() {
        return Utility::GetSingleton()->GetUI()->IsMenuOpen(RE::DialogueMenu::MENU_NAME);
    }

    static bool IsPlayerInMenu() {
        auto ui = Utility::GetSingleton()->GetUI();

        if (ui && !ui->GameIsPaused() && !ui->IsApplicationMenuOpen() && !ui->IsItemMenuOpen() && !ui->IsMenuOpen(RE::InterfaceStrings::GetSingleton()->dialogueMenu))
            return true;
        else
            return false;
    }
};
