#undef GetObject

#include "Events.h"
#include "FoundEquipData.h"
#include "Settings.h"
#include "Utility.h"

#include <unordered_set>
#include <set>
#include <array>
#include <random>

class ExtraLocationRefType : public RE::BSExtraData {
public:
	RE::BGSLocationRefType * refType;
};

std::mt19937 mt(std::random_device{}());
std::mutex lock_mt;

std::unordered_set<RE::Actor*> list;
std::mutex lock_list;

bool skip = false;
int UpdateCount = 0;

static bool Probability(int value) {
	if (value == 0)
		return false;

	std::uniform_int_distribution<> score{0, 99};
	std::lock_guard<std::mutex> guard(lock_mt);
	int guess = score(mt);

	if (guess < value)
		return true;

	return false;
}

static bool Probability(double value) {
	if (value <= 0.0)
		return false;

	std::uniform_real_distribution<double> score{0.0, 100.0};
	std::lock_guard<std::mutex> guard(lock_mt);
	double guess = score(mt) ;

	if (guess < value)
		return true;

	return false;
}

static void ShuffleSlots(std::array<RE::BGSBipedObjectForm::BipedObjectSlot, 4> *slots) {
	std::lock_guard<std::mutex> guard(lock_mt);
	std::shuffle(slots->begin(), slots->end(), mt);
}

static double GetRandom(double a, double b) {
	std::uniform_real_distribution<> score(a, b);
	std::lock_guard<std::mutex> guard(lock_mt);
	return score(mt) * 0.0001;
}

static bool AddActor(RE::Actor* actor) {
	std::lock_guard<std::mutex> guard(lock_list);

	auto result = list.insert(actor);
	if (result.second) {
		skip = true;
		return true;
	}

	return false;
}

static void ClearActor() {
	std::lock_guard<std::mutex> guard(lock_list);

	if (skip)
		skip = false;
	else if (!list.empty())
		list.clear();
}

static void RemoveEquipment(FoundEquipData eqD, RE::Actor* actor) {
	auto utility = Utility::GetSingleton();
	if (actor == utility->GetPlayer()) {
		eqD.GenerateName();
		auto msg = std::format("Your {} has broken", eqD.pForm->GetName());
		utility->ShowNotification(msg,false);
	}
	actor->RemoveItem(eqD.pForm->As<RE::TESBoundObject>(), 1, RE::ITEM_REMOVE_REASON::kRemove, eqD.pExtraData, nullptr, 0, 0);
	actor->Update(0);
}

static void TemperDecay(FoundEquipData eqD, RE::Actor* actor, bool powerAttack) {
	auto utility = Utility::GetSingleton();

	if (!eqD.pForm)
		return;

	if (!AddActor(actor))
		return;

	float itemHealthPercent = eqD.GetItemHealthPercent();

	// Check if item will break
	if (itemHealthPercent <= ini.EquipmentHealthThreshold)
	{
		double chance = ini.GetBreakChanceSettings(eqD.pForm);

		if (chance != 0.0 && eqD.CanBreak())
		{
			if (ini.GetDegradationSettings("IncreaseDurability") == 1 && itemHealthPercent > 1.0f)
				chance *= 1.0 - ((itemHealthPercent - 1.0f) / (ini.EquipmentHealthThreshold - 1.0f));

			if (powerAttack)
				chance *= ini.GetBreakChanceSettings("PowerAttackMultiplier");

			if (actor != utility->GetPlayer())
			{
				if (actor->IsPlayerTeammate())
					chance *= ini.GetBreakChanceSettings("FollowerMultiplier");
				else
					chance *= ini.GetBreakChanceSettings("NPCMultiplier");
			}

			if (Probability(chance))
			{
				RemoveEquipment(eqD, actor);
				return;
			}
		}
	}

	if (itemHealthPercent <= 1.0f)
		return;

	double rate = ini.GetDegradationRateSettings(eqD.pForm);
	if (rate == 0.0)
		return;

	// Health Degredation
	if (powerAttack)
		rate *= ini.GetDegradationRateSettings("PowerAttackMultiplier");

	if (actor != utility->GetPlayer())
	{
		if (actor->IsPlayerTeammate() != 0)
			rate *= ini.GetDegradationRateSettings("FollowerMultiplier");
		else
			rate *= ini.GetDegradationRateSettings("NPCMultiplier");
	}

	itemHealthPercent -= GetRandom(rate, std::pow(rate + 1.0, 2.0));

	if (itemHealthPercent < 1.0f)
		itemHealthPercent = 1.0f;

	eqD.SetItemHealthPercent(itemHealthPercent);
}

// On Hit event. Decay player or enemy equipment
class HitEventHandler : public RE::BSTEventSink<RE::TESHitEvent> {
public:
    std::mutex container_mutex;
        
    static HitEventHandler* GetSingleton() {
        static HitEventHandler singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override 
	{
		auto utility = Utility::GetSingleton();

		if (a_event->target && a_event->target->formType == RE::FormType::ActorCharacter) {
			RE::Actor* actor = a_event->target->As<RE::Actor>();
			if (actor == utility->GetPlayer() || ini.GetDegradationSettings("OnlyPlayer") == 0) {
				RE::InventoryChanges *exChanges = actor->GetInventoryChanges();
				if (exChanges) {
					RE::TESForm* form = RE::TESForm::LookupByID(a_event->source);

					if (form && ((form->formType == RE::FormType::Weapon && !form->As<RE::TESObjectWEAP>()->IsStaff()) || (form->formType == RE::FormType::Armor && form->As<RE::TESObjectARMO>()->IsShield()))) {
						
						// Decay weapon/shield if it was used or parried with
						if ((a_event->flags & RE::TESHitEvent::Flag::kHitBlocked) != RE::TESHitEvent::Flag::kNone) {
							FoundEquipData eqD_armor = FoundEquipData::FindEquippedArmor(exChanges, RE::BGSBipedObjectForm::BipedObjectSlot::kShield);
							if (eqD_armor.pForm) {
								TemperDecay(eqD_armor, actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
							} else{
								RE::TESForm* weap = actor->GetEquippedObject(false);
								if (weap) {
									if (!weap->As<RE::TESObjectWEAP>()->IsBound())
										TemperDecay(FoundEquipData::FindEquippedWeapon(exChanges, false, weap), actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
								}
							}

						// Decay armor, amrmor slots are shuffled for decay loss
						} else {
							std::array<RE::BGSBipedObjectForm::BipedObjectSlot, 4> slots = { RE::BGSBipedObjectForm::BipedObjectSlot::kHead, RE::BGSBipedObjectForm::BipedObjectSlot::kBody, RE::BGSBipedObjectForm::BipedObjectSlot::kHands, RE::BGSBipedObjectForm::BipedObjectSlot::kFeet };
							ShuffleSlots(&slots);

							for (RE::BGSBipedObjectForm::BipedObjectSlot slot : slots) {
								FoundEquipData eqD_armor = FoundEquipData::FindEquippedArmor(exChanges, slot);
								if (eqD_armor.pForm) {
									TemperDecay(eqD_armor, actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
									break;
								} else if (slot == RE::BGSBipedObjectForm::BipedObjectSlot::kHead) {
									eqD_armor = FoundEquipData::FindEquippedArmor(exChanges, RE::BGSBipedObjectForm::BipedObjectSlot::kHair);
									if (eqD_armor.pForm) {
										TemperDecay(eqD_armor, actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
										break;
									}
								}
							}
						}
					}
				}
			}
		}

		if (a_event->cause && a_event->cause->formType == RE::FormType::ActorCharacter) {
			RE::Actor* actor = a_event->cause->As<RE::Actor>();
			if (actor == utility->GetPlayer() || ini.GetDegradationSettings("OnlyPlayer") == 0) {
				RE::InventoryChanges *exChanges = actor->GetInventoryChanges();
				if (exChanges) {
					RE::TESForm* form = RE::TESForm::LookupByID(a_event->source);
					if (form) {
						if (form->formType == RE::FormType::Weapon) {
							RE::TESObjectWEAP* weap = form->As<RE::TESObjectWEAP>();
							if (!weap->IsStaff() && !weap->IsBound()) {
								if (form == actor->GetEquippedObject(false)) {
									if (form == actor->GetEquippedObject(true)) {
										bool bLeftHandAttack;
										static RE::BSFixedString strLeftHandAttack = "bLeftHandAttack";
										if (actor->GetGraphVariableBool(strLeftHandAttack, bLeftHandAttack) && bLeftHandAttack)
											TemperDecay(FoundEquipData::FindEquippedWeapon(exChanges, true, form), actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
										else
											TemperDecay(FoundEquipData::FindEquippedWeapon(exChanges, false, form), actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
									} else {
										TemperDecay(FoundEquipData::FindEquippedWeapon(exChanges, false, form), actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
									}
								} else if (form == actor->GetEquippedObject(true)) {
									TemperDecay(FoundEquipData::FindEquippedWeapon(exChanges, true, form), actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
								}
							}
						} else if (form->formType == RE::FormType::Armor) {
							if (form->As<RE::TESObjectARMO>()->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kShield)) {
								TemperDecay(FoundEquipData::FindEquippedArmor(exChanges, RE::BGSBipedObjectForm::BipedObjectSlot::kShield), actor, (a_event->flags & RE::TESHitEvent::Flag::kPowerAttack) != RE::TESHitEvent::Flag::kNone);
							}
						}
					}
				}
			}
		}

		return RE::BSEventNotifyControl::kContinue;
    }

    static void Register() {
        RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventHolder->AddEventSink(HitEventHandler::GetSingleton());
    }
};

// Get references within radius of the player and log them
static void GetCloseEquipment(std::set<RE::TESObjectREFR*> *containerlist, std::set<RE::TESObjectREFR*> *equipmentlist) {
	auto utility = Utility::GetSingleton();
	auto TES = RE::TES::GetSingleton();

	if (TES) {
		static double fRadiusSquare = std::pow(7000, 2.0);
		TES->ForEachReferenceInRange(utility->GetPlayer(), fRadiusSquare, [&](RE::TESObjectREFR& b_ref) {
			if (!b_ref.IsDisabled()) {
				RE::TESObjectREFR* newRef = &b_ref;
				if (b_ref.formType == RE::FormType::ActorCharacter || b_ref.GetBaseObject()->formType == RE::FormType::Container) {
					containerlist->insert(newRef);
				} else if (b_ref.GetBaseObject()->formType == RE::FormType::Armor|| b_ref.GetBaseObject()->formType == RE::FormType::Weapon) {
					equipmentlist->insert(newRef);
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
	}
}

// Apply a temper on items that meet the requirements and hit the temper chance
static void DynamicTemper() {
	auto utility = Utility::GetSingleton();
	std::set<RE::TESObjectREFR*> containerlist;
	std::set<RE::TESObjectREFR*> equipmentlist;

	GetCloseEquipment(&containerlist, &equipmentlist);

	if (!equipmentlist.empty()) {
		for (auto& ref : equipmentlist) {
			if (!ref || !ref->GetBaseObject())
				continue;

			FoundEquipData eqD(ref->GetBaseObject());
			if (!eqD.CanTemper() || ref->extraList.HasType((RE::ExtraDataType::kHealth)))
				continue;

			eqD.pExtraData = &ref->extraList;
			int level = 0;
			RE::TESForm* owner = ref->GetOwner();
			if (owner && owner->formID != 0x000007)
				level = utility->GetPlayer()->GetLevel();

			if (level != 0 && eqD.CanBreak() && Probability(ini.GetTemperSettings("TemperChance"))) {
				eqD.SetItemHealthPercent(GetRandom(10001.0, 10001.0 + (level + 10) * 100));
			} else {
				eqD.SetItemHealthPercent(1.0f);
			}
		}
	}

	if (!containerlist.empty()) {
		for (auto& ref : containerlist) {
			if (!ref)
				continue;

			RE::InventoryChanges *exChanges =  ref->GetInventoryChanges();

			if (!exChanges || !exChanges->entryList)
				continue;

			for (const auto& pEntry : *exChanges->entryList)
			{
				if (!pEntry || !pEntry->extraLists)
					continue;

				FoundEquipData eqD(pEntry->GetObject());

				if (!eqD.CanTemper())
					continue;

				for (const auto& pExtraDataList : *pEntry->extraLists)
				{
					if (!pExtraDataList)
						continue;

					if (!pExtraDataList->HasType(RE::ExtraDataType::kHealth)) {
						eqD.pExtraData = pExtraDataList;
					}

					break;
				}

				if (!eqD.pExtraData)
					continue;

				int level = 0;
				if (ref->formType == RE::FormType::ActorCharacter) {
					RE::Actor* actor = ref->As<RE::Actor>();
					if (actor != utility->GetPlayer() && !actor->IsInFaction(utility->factionFollower1) && !actor->IsInFaction(utility->factionFollower2))
						level = actor->GetLevel();
				} else {
					level = utility->GetPlayer()->GetLevel();
				}

				if (level != 0 && eqD.CanBreak()) {

					int chance = ini.GetTemperSettings("TemperChance");
					RE::ExtraLocationRefType * xRefType = static_cast<RE::ExtraLocationRefType*>(ref->extraList.GetByType(RE::ExtraDataType::kLocationRefType));
					if (xRefType && (xRefType->locRefType == utility->locationBoss || xRefType->locRefType == utility->locationBossContainer))
						chance = ini.GetTemperSettings("BossTemperChance");

					if (Probability(chance)) {
						eqD.SetItemHealthPercent(GetRandom(10001.0, 10001.0 + (level + 10) * 100));
					} else {
						eqD.SetItemHealthPercent(1.0f);
					}
				} else {
					eqD.SetItemHealthPercent(1.0f);
				}
			}
		}
	}
}

// On Update event to run Dynamic Temper
static std::int32_t OnUpdate() {
	if (!Utility::GetUI()->GameIsPaused()) {

		ClearActor();

		if (g_deltaTime > 0) {
			lastTime += g_deltaTime;
			if (lastTime >= 1.0f) {
				if (ini.GetTemperSettings("DisableDynamicTemper") == 0)
					DynamicTemper();
				lastTime = 0;
			}
		}
	}

	return _OnUpdate();
}

namespace Events {
	inline static REL::Relocation<std::uintptr_t> On_Update_Hook{ REL::RelocationID(35565, 36564), REL::Relocate(0x748, 0xC26) };

	void Init(void)
	{
		if (ini.GetDegradationSettings("DisableDegradation") == 0) {
			HitEventHandler::Register();
		}

		if (ini.GetDegradationSettings("DisableDegradation") == 0 || ini.GetTemperSettings("DisableDynamicTemper") == 0) {
			auto& trampoline = SKSE::GetTrampoline();
			_OnUpdate = trampoline.write_call<5>(On_Update_Hook.address(), OnUpdate);
			logger::info("Installed update hook");
		}
	}
}