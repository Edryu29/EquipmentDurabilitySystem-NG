#undef GetObject
#include "FoundEquipData.h"
#include "Settings.h"
#include "Utility.h"

void FoundEquipData::GenerateName()
{
	if (!pForm || !pExtraData)
		return;

	RE::InventoryEntryData NewEntry = RE::InventoryEntryData(pForm->As<RE::TESBoundObject>(), pExtraData->GetCount());
	NewEntry.AddExtraList(pExtraData);

	name = std::string(NewEntry.GetDisplayName());
	if (pForm->IsWeapon() && ini.GetWidgetSettings("HidePoisonName") == 0 && pExtraData->HasType(RE::ExtraDataType::kPoison)) {
		RE::ExtraPoison* xPoison = static_cast<RE::ExtraPoison*>(pExtraData->GetByType(RE::ExtraDataType::kPoison));
		if (xPoison && xPoison->poison)
			name += " " + std::string(xPoison->poison->GetFullName()) + " (" + std::to_string(xPoison->count) + ")";
	}
	NewEntry.extraLists->clear();
}

float FoundEquipData::GetItemHealthPercent()
{
	if (!pExtraData)
		return 1.0f;

	RE::ExtraHealth* xHealth = static_cast<RE::ExtraHealth*>(pExtraData->GetByType(RE::ExtraDataType::kHealth));
	if (xHealth)
		return xHealth->health;

	return 1.0f;
}

void FoundEquipData::SetItemHealthPercent(float value) {
	if (!pExtraData)
		return;

	RE::ExtraHealth* xHealth = static_cast<RE::ExtraHealth*>(pExtraData->GetByType(RE::ExtraDataType::kHealth));
	if (xHealth) {
		xHealth->health = value;
	} else if (pExtraData) {
		RE::ExtraHealth* newHealth = static_cast<RE::ExtraHealth*>(RE::ExtraHealth::Create(sizeof(RE::ExtraHealth), RE::VTABLE_ExtraHealth[0].address()));
		newHealth->health = value;
		pExtraData->Add(newHealth);
	}
}

void FoundEquipData::SetEnchantmentList() {
	if (!pForm)
		return;

	//if (pForm->IsWeapon())
	//	//pEnchantList = ;
	//else if (pForm->IsArmor()) {
	//	RE::TESObjectARMO *armor = pForm->As<RE::TESObjectARMO>();
	//	if (armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kBody) || armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kTail))
	//		//pEnchantList = Body;
	//	else if (armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHead) || armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHair) || armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kLongHair))
	//		//pEnchantList = Head;
	//	else if (armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHands) || armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kForearms))
	//		//pEnchantList = Hands;
	//	else if (armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet) || armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kCalves))
	//		//pEnchantList = Feet;
	//	else if (armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kShield))
	//		//pEnchantList = Shield;
	//}

}

void FoundEquipData::SetItemEnchantment(int index, float value) {
	if (!pForm || !pEnchantList->size() <= 0)
		return;

	// Set enchantment and charge
	if (pForm->IsWeapon()) {
		pForm->As<RE::TESObjectWEAP>()->formEnchanting = pEnchantList->at(index);
		pForm->As<RE::TESObjectWEAP>()->amountofEnchantment = value;
	} else if (pForm->IsArmor()) {
		pForm->As<RE::TESObjectARMO>()->formEnchanting = pEnchantList->at(index);
		pForm->As<RE::TESObjectARMO>()->amountofEnchantment = value;
	}

	// Set Name
	name = std::string(pForm->GetName());
	name += " ";
}

bool FoundEquipData::CanBreak() {
	if (!pForm || !pExtraData)
		return false;

	// Do not break items that you cannot disenchant, user preference
	auto utility = Utility::GetSingleton();
	if (ini.GetDegradationSettings("NoBreakMagicDisallowEnchanting") == 1) {
		if (pForm->IsWeapon() && pForm->As<RE::TESObjectWEAP>()->HasKeyword(utility->keywordMagicDisallow))
			return false;
		else if (pForm->IsArmor() && pForm->As<RE::TESObjectARMO>()->HasKeyword(utility->keywordMagicDisallow))
			return false;
	}

	// Unarmed
	if (pForm->formID == 0x0001F4)
		return false;

	// Compare to the break form list
	if (ini.HasNoBreakForms(pForm->formID))
		return false;

	// Dont break quest items
	RE::InventoryEntryData newData(pForm->As<RE::TESBoundObject>(), 1);
	if (newData.IsQuestObject())
		return false;

	return true;
}

bool FoundEquipData::CanTemper() {
	if (!pForm)
		return false;

	auto utility = Utility::GetSingleton();
	if (pForm->IsWeapon()) {
		// Form cannot be a staff or a bound weapon
		RE::TESObjectWEAP* weap = pForm->As<RE::TESObjectWEAP>();
		if (!weap->IsStaff() && !weap->IsBound())
			return true;
	} else if (pForm->IsArmor()) {
		// Armor has to bip one of the given armor slots
		RE::TESObjectARMO *armo = pForm->As<RE::TESObjectARMO>();
		if (armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHead) || armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHair) 
			|| armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kBody) || armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHands) 
			|| armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet) || armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kShield))
		{
			if (!armo->HasKeyword(utility->keywordClothing))
				return true;
		}
	}

	return false;
}

bool FoundEquipData::CanEnchant() {
	if (!pForm)
		return false;

	auto utility = Utility::GetSingleton();
	if (pForm->IsWeapon()) {
		// Form cannot be a staff or a bound weapon
		RE::TESObjectWEAP* weap = pForm->As<RE::TESObjectWEAP>();
		if (!weap->IsStaff() && !weap->IsBound() && !weap->IsMagicItem())
			return true;
	} else if (pForm->IsArmor()) {
		// Armor has to bip one of the given armor slots
		RE::TESObjectARMO *armo = pForm->As<RE::TESObjectARMO>();
		if (armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHead) || armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHair) 
			|| armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kBody) || armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHands) 
			|| armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kFeet) || armo->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kShield))
		{
			if (!armo->HasKeyword(utility->keywordClothing) && !armo->IsMagicItem())
				return true;
		}
	}

	return false;
}

FoundEquipData FoundEquipData::FindEquippedWeapon(RE::InventoryChanges *exChanges, bool abLeftHand, RE::TESForm* form) {
	FoundEquipData equipData;

	if (exChanges->entryList) {
		for (const auto& pEntry : *exChanges->entryList) {
			if (!pEntry || pEntry->GetObject() != form || !pEntry->extraLists)
				continue;

			for (const auto& pExtraDataList : *pEntry->extraLists) {
				if (pExtraDataList) {
					if ((!abLeftHand && pExtraDataList->HasType(RE::ExtraDataType::kWorn)) || (abLeftHand && pExtraDataList->HasType(RE::ExtraDataType::kWornLeft))) {
						equipData.pForm = pEntry->GetObject();
						equipData.pExtraData = pExtraDataList;

						return equipData;
					}
				}
			}
		}
	}

	return equipData;
}

FoundEquipData FoundEquipData::FindEquippedArmor(RE::InventoryChanges *exChanges, RE::BGSBipedObjectForm::BipedObjectSlot slotMask) {
	FoundEquipData equipData;

	if (exChanges->entryList) {
		for (const auto& pEntry : *exChanges->entryList) {
			if (!pEntry || !pEntry->GetObject()->IsArmor() || !pEntry->GetObject()->As<RE::TESObjectARMO>()->HasPartOf(slotMask) || !pEntry->extraLists)
				continue;

			for (const auto& pExtraDataList : *pEntry->extraLists) {
				if (pExtraDataList) {
					if (pExtraDataList->HasType(RE::ExtraDataType::kWorn) || pExtraDataList->HasType(RE::ExtraDataType::kWornLeft)) {
						equipData.pForm = pEntry->GetObject();
						equipData.pExtraData = pExtraDataList;

						return equipData;
					}
				}
			}
		}
	}

	return equipData;
}