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
	if (pForm->formType == RE::FormType::Weapon && ini.GetWidgetSettings("HidePoisonName") == 0 && pExtraData->HasType(RE::ExtraDataType::kPoison))
	{
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

void FoundEquipData::SetItemHealthPercent(float value)
{
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

bool FoundEquipData::CanBreak()
{
	if (!pForm || !pExtraData)
		return false;

	if (ini.GetDegradationSettings("NoBreakMagicDisallowEnchanting") == 1)
	{
		static RE::BGSKeyword *keywordMagicDisallow = RE::TESDataHandler::GetSingleton()->LookupForm(RE::FormID(0x0C27BD), "Skyrim.esm")->As<RE::BGSKeyword>();
		if (pForm->formType == RE::FormType::Weapon)
		{
			if (pForm->As<RE::TESObjectWEAP>()->HasKeyword(keywordMagicDisallow))
				return false;
		}
		else if (pForm->formType == RE::FormType::Armor)
		{
			if (pForm->As<RE::TESObjectARMO>()->HasKeyword(keywordMagicDisallow))
				return false;
		}
	}

	if (pForm->formID == 0x0001F4)
		return false;

	if (ini.HasNoBreakForms(pForm->formID))
		return false;

	RE::InventoryEntryData newData(pForm->As<RE::TESBoundObject>(), 1);
	if (newData.IsQuestObject())
		return false;

	return true;
}

bool FoundEquipData::CanTemper()
{
	if (!pForm)
		return false;

	auto utility = Utility::GetSingleton();
	if (pForm->formType == RE::FormType::Weapon)
	{
		RE::TESObjectWEAP* weap = pForm->As<RE::TESObjectWEAP>();
		if (weap->GetWeaponType() != RE::WEAPON_TYPE::kStaff && (weap->weaponData.flags2 & RE::TESObjectWEAP::Data::Flag2::kBoundWeapon) == RE::TESObjectWEAP::Data::Flag2::kNone)
			return true;
	}
	else if (pForm->formType == RE::FormType::Armor)
	{
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

FoundEquipData FoundEquipData::FindEquippedWeapon(RE::InventoryChanges *exChanges, bool abLeftHand, RE::TESForm* form)
{
	FoundEquipData equipData;

	if (exChanges->entryList)
	{
		for (const auto& pEntry : *exChanges->entryList)
		{
			if (!pEntry || pEntry->GetObject() != form || !pEntry->extraLists)
				continue;

			for (const auto& pExtraDataList : *pEntry->extraLists)
			{
				if (pExtraDataList)
				{
					if ((!abLeftHand && pExtraDataList->HasType(RE::ExtraDataType::kWorn)) || (abLeftHand && pExtraDataList->HasType(RE::ExtraDataType::kWornLeft)))
					{
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

FoundEquipData FoundEquipData::FindEquippedArmor(RE::InventoryChanges *exChanges, RE::BGSBipedObjectForm::BipedObjectSlot slotMask)
{
	FoundEquipData equipData;

	if (exChanges->entryList)
	{
		for (const auto& pEntry : *exChanges->entryList)
		{
			if (!pEntry || pEntry->GetObject()->GetFormType() != RE::FormType::Armor || !pEntry->GetObject()->As<RE::TESObjectARMO>()->HasPartOf(slotMask) || !pEntry->extraLists)
				continue;

			for (const auto& pExtraDataList : *pEntry->extraLists)
			{
				if (pExtraDataList)
				{
					if (pExtraDataList->HasType(RE::ExtraDataType::kWorn) || pExtraDataList->HasType(RE::ExtraDataType::kWornLeft))
					{
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