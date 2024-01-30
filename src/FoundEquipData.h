#pragma once

struct FoundEquipData
{
	RE::TESForm* pForm;
	RE::ExtraDataList* pExtraData;
	std::string name;
	std::vector<RE::EnchantmentItem*>* pEnchantList;

	FoundEquipData() : pForm(nullptr), pExtraData(nullptr)
	{
	}

	explicit FoundEquipData(RE::TESForm* a_pForm) : pForm(a_pForm), pExtraData(nullptr)
	{
	}

	void GenerateName();

	float GetItemHealthPercent();
	void SetItemHealthPercent(float value);

	void SetEnchantmentList();
	void SetItemEnchantment(int index, float value);

	bool CanBreak();
	bool CanTemper();
	bool CanEnchant();

	static FoundEquipData FindEquippedWeapon(RE::InventoryChanges *exChanges, bool abLeftHand, RE::TESForm* form);
	static FoundEquipData FindEquippedArmor(RE::InventoryChanges *exChanges, RE::BGSBipedObjectForm::BipedObjectSlot slotMask);
};