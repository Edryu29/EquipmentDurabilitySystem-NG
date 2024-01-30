#include "Settings.h"
#include "Utility.h"

INIFile ini;

constexpr auto settings_path = L"Data/SKSE/Plugins/EquipmentDurability/EquipmentDurability.ini";
constexpr auto break_path = L"Data/SKSE/Plugins/EquipmentDurability/NoBreakForms.ini";
constexpr auto enchant_path = L"Data/SKSE/Plugins/EquipmentDurability/";

INIFile::INIFile() : EquipmentHealthThreshold(1.0f) {
	degradationMap["disabledegradation"] = 0;
	degradationMap["onlyplayer"] = 1;
	degradationMap["nobreakmagicdisallowenchanting"] = 1;
	degradationMap["increasedurability"] = 1;
	degradationMap["equipmenthealththreshold"] = 0;

	degradationRateMap["weaponsword"] = 3.0;
	degradationRateMap["weapondagger"] = 3.0;
	degradationRateMap["weaponwaraxe"] = 3.0;
	degradationRateMap["weaponmace"] = 3.0;
	degradationRateMap["weapongreatsword"] = 3.0;
	degradationRateMap["weaponhammer"] = 3.0;
	degradationRateMap["weaponbattleaxe"] = 3.0;
	degradationRateMap["weaponbow"] = 3.0;
	degradationRateMap["weaponcrossbow"] = 3.0;
	degradationRateMap["lightarmor"] = 3.0;
	degradationRateMap["heavyarmor"] = 3.0;
	degradationRateMap["clothing"] = 3.0;
	degradationRateMap["defaultarmor"] = 3.0;
	degradationRateMap["powerattackmultiplier"] = 2.0;
	degradationRateMap["followermultiplier"] = 1.0;
	degradationRateMap["npcmultiplier"] = 1.0;

	breakChanceMap["weaponsword"] = 0.0;
	breakChanceMap["weapondagger"] = 0.0;
	breakChanceMap["weaponwaraxe"] = 0.0;
	breakChanceMap["weaponmace"] = 0.0;
	breakChanceMap["weapongreatsword"] = 0.0;
	breakChanceMap["weaponhammer"] = 0.0;
	breakChanceMap["weaponbattleaxe"] = 0.0;
	breakChanceMap["weaponbow"] = 0.0;
	breakChanceMap["weaponcrossbow"] = 0.0;
	breakChanceMap["lightarmor"] = 0.0;
	breakChanceMap["heavyarmor"] = 0.0;
	breakChanceMap["clothing"] = 0.0;
	breakChanceMap["defaultarmor"] = 0.0;
	breakChanceMap["powerattackmultiplier"] = 2.0;
	breakChanceMap["followermultiplier"] = 1.0;
	breakChanceMap["npcmultiplier"] = 1.0;

	temperMap["disabledynamictemper"] = 0;
	temperMap["temperchance"] = 40;
	temperMap["bosstemperchance"] = 80;

	widgetMap["disablewidget"] = 0;
	widgetMap["positionx"] = 3;
	widgetMap["positiony"] = 73;
	widgetMap["scale"] = 100;
	widgetMap["unbreakablecolor"] = 0xFFFFFF;
	widgetMap["breakablecolor"] = 0xFFD700;
	widgetMap["hidepoisonname"] = 0;
	widgetMap["hidewithweaponssheathed"] = 0;
	widgetMap["togglekeycode"] = 0;
	widgetMap["toggleduration"] = 0.0;
	widgetMap["flags"] = 8;
}

void INIFile::Load() {
	SetSettings();
	ShowSettings();

	if (degradationMap["equipmenthealththreshold"] == 0)
		EquipmentHealthThreshold = 1.0f;
	else
		EquipmentHealthThreshold = degradationMap["equipmenthealththreshold"] * 0.001f + 1.0f;
}

int INIFile::GetDegradationSettings(std::string str) {
	std::string tmp = str;
	ToLower(tmp);
	return 	(degradationMap.count(tmp) >= 1) ? degradationMap.at(tmp) : 0;
}

double INIFile::GetDegradationRateSettings(std::string str) {
	std::string tmp = str;
	ToLower(tmp);
	return 	(degradationRateMap.count(tmp) >= 1) ? degradationRateMap.at(tmp) : 0.0;
}

double INIFile::GetDegradationRateSettings(RE::TESForm* form) {
	auto utility = Utility::GetSingleton();

	if (form->formType == RE::FormType::Weapon) {
		RE::TESObjectWEAP *weap = form->As<RE::TESObjectWEAP>();
		switch (weap->GetWeaponType()) {
		case RE::WEAPON_TYPE::kOneHandSword:
			return GetDegradationRateSettings("WeaponSword");
		case RE::WEAPON_TYPE::kOneHandDagger:
			return GetDegradationRateSettings("WeaponDagger");
		case RE::WEAPON_TYPE::kOneHandAxe:
			return GetDegradationRateSettings("WeaponWarAxe");
		case RE::WEAPON_TYPE::kOneHandMace:
			return GetDegradationRateSettings("WeaponMace");
		case RE::WEAPON_TYPE::kTwoHandSword:
			return GetDegradationRateSettings("WeaponGreatSword");
		case RE::WEAPON_TYPE::kTwoHandAxe:
			if (weap->HasKeyword(utility->keywordWarhammer))
				return GetDegradationRateSettings("WeaponHammer");
			else
				return GetDegradationRateSettings("WeaponBattleAxe");
		case RE::WEAPON_TYPE::kBow:
			return GetDegradationRateSettings("WeaponBow");
		case RE::WEAPON_TYPE::kCrossbow:
			return GetDegradationRateSettings("WeaponCrossbow");
		default:
			return GetDegradationRateSettings("WeaponSword");
		}
	} else if (form->formType == RE::FormType::Armor) {
		RE::TESObjectARMO *armor = form->As<RE::TESObjectARMO>();
		if (armor->IsLightArmor()) {
			return GetDegradationRateSettings("LightArmor");
		} else if (armor->IsHeavyArmor()) {
			return GetDegradationRateSettings("HeavyArmor");
		} else {
			if (armor->HasKeyword(utility->keywordClothing)) {
				return GetDegradationRateSettings("Clothing");
			}
		}
	}

	return GetDegradationRateSettings("DefaultArmor");
}

double INIFile::GetBreakChanceSettings(std::string str) {
	std::string tmp = str;
	ToLower(tmp);
	return (breakChanceMap.count(tmp) >= 1) ? breakChanceMap.at(tmp) : 0.0;
}

double INIFile::GetBreakChanceSettings(RE::TESForm* form) {
	auto utility = Utility::GetSingleton();

	if (form->formType == RE::FormType::Weapon) {
		RE::TESObjectWEAP *weap = form->As<RE::TESObjectWEAP>();
		switch (weap->GetWeaponType()) {
		case RE::WEAPON_TYPE::kOneHandSword:
			return GetBreakChanceSettings("WeaponSword");
		case RE::WEAPON_TYPE::kOneHandDagger:
			return GetBreakChanceSettings("WeaponDagger");
		case RE::WEAPON_TYPE::kOneHandAxe:
			return GetBreakChanceSettings("WeaponWarAxe");
		case RE::WEAPON_TYPE::kOneHandMace:
			return GetBreakChanceSettings("WeaponMace");
		case RE::WEAPON_TYPE::kTwoHandSword:
			return GetBreakChanceSettings("WeaponGreatSword");
		case RE::WEAPON_TYPE::kTwoHandAxe:
			if (weap->HasKeyword(utility->keywordWarhammer))
				return GetBreakChanceSettings("WeaponHammer");
			else
				return GetBreakChanceSettings("WeaponBattleAxe");
		case RE::WEAPON_TYPE::kBow:
			return GetBreakChanceSettings("WeaponBow");
		case RE::WEAPON_TYPE::kCrossbow:
			return GetBreakChanceSettings("WeaponCrossbow");
		default:
			return GetBreakChanceSettings("WeaponSword");
		}
	} else if (form->formType == RE::FormType::Armor) {
		RE::TESObjectARMO *armor = form->As<RE::TESObjectARMO>();
		if (armor->IsLightArmor()) {
			return GetBreakChanceSettings("LightArmor");
		} else if (armor->IsHeavyArmor()) {
			return GetBreakChanceSettings("HeavyArmor");
		} else {
			if (armor->HasKeyword(utility->keywordClothing)) {
				return GetBreakChanceSettings("Clothing");
			}
		}
	}

	return GetBreakChanceSettings("DefaultArmor");
}

int INIFile::GetTemperSettings(std::string str)
{
	std::string tmp = str;
	ToLower(tmp);
	return 	(temperMap.count(tmp) >= 1) ? temperMap.at(tmp) : 0;
}

int INIFile::GetWidgetSettings(std::string str)
{
	std::string tmp = str;
	ToLower(tmp);
	return 	(widgetMap.count(tmp) >= 1) ? widgetMap.at(tmp) : 0;
}

bool INIFile::HasNoBreakForms(int formid) {
	if (noBreakForms.empty())
		return false;

	auto it = noBreakForms.find(formid);
	if (it == noBreakForms.end())
		return false;

	return true;
}

void INIFile::ToLower(std::string &str)
{
	for (auto &c : str)
		c = tolower(c);
}

bool INIFile::stricmp(const char* str_1, const char* str_2) {
	// Create locals and lower case them
	std::string a(str_1);
	ToLower(a);
	std::string b(str_2);
	ToLower(b);

	// Compare the values and return true/false
	if(a.compare(b) == 0)
		return true;
	else
		return false;
}

void INIFile::SetSettings() {
	// System Settings
	CSimpleIniA iniSettings;
	iniSettings.SetUnicode();
	iniSettings.LoadFile(settings_path);

	std::list<CSimpleIniA::Entry> i_sections;
	iniSettings.GetAllSections(i_sections);

	for (auto i_section : i_sections)  {
		std::list<CSimpleIniA::Entry> entryList;
		iniSettings.GetAllKeys(i_section.pItem, entryList);

		if (stricmp(i_section.pItem, "DegradationRate")) {
			SetINIData1(&entryList, i_section.pItem, &degradationRateMap);
		} else if (stricmp(i_section.pItem, "BreakChance")) {
			SetINIData1(&entryList, i_section.pItem, &breakChanceMap);
		} else {
			SetINIData1(&entryList, i_section.pItem);
		}
	}

	// Break Forms
	CSimpleIniA iniBreak;
	iniBreak.SetUnicode();
	iniBreak.LoadFile(break_path);

	std::list<CSimpleIniA::Entry> sectionList;
	iniBreak.GetAllSections(sectionList);

	SetINIData2(&sectionList, &noBreakForms);

	// Enchantment Forms
	std::vector<std::string> configs{};
	std::string sfilename = "_ENCH_";

	for (const auto iterator = std::filesystem::directory_iterator(enchant_path); const auto& entry : iterator) {
		if (entry.exists()) {
			if (const auto& path = entry.path(); !path.empty() && path.extension() == ".ini") {
				if (const auto& fileName = entry.path().string(); sfilename.empty() || fileName.rfind(sfilename) != std::string::npos) {
					configs.push_back(fileName);
				}
			}
		}
	}
	std::ranges::sort(configs);

	if (configs.empty()) {
		logger::debug("No .ini files with _SWAP suffix were found within the Data folder, aborting...");
		return;
	}

	logger::info("{} matching inis found", configs.size());
	for (auto& path : configs) {
		logger::info("Enchant File : {}", path);

		CSimpleIniA iniEnch;
		iniEnch.SetUnicode();
		iniEnch.SetMultiKey();
		iniEnch.SetAllowKeyOnly();

		if (const auto rc = iniEnch.LoadFile(path.c_str()); rc < 0) {
			logger::error("\tcouldn't read INI");
			continue;
		}

		std::list<CSimpleIniA::Entry> sections;
		iniEnch.GetAllSections(sections);
		sections.sort(CSimpleIniA::Entry::LoadOrder());

		constexpr auto push_filter = [](const std::string& a_condition, std::vector<FormIDStr>& a_processedFilters) {
			if (const auto processedID = SwapData::GetFormID(a_condition); processedID != 0) {
				a_processedFilters.emplace_back(processedID);
			} else {
				logger::error("\t\tFilter  [{}] INFO - unable to find form, treating filter as string", a_condition);
				a_processedFilters.emplace_back(a_condition);
			}
		};
	}

}

void INIFile::SetINIData1(std::list<CSimpleIniA::Entry> *list, const char* section) {
	CSimpleIniA iniSettings;
	iniSettings.SetUnicode();
	iniSettings.LoadFile(settings_path);

	for (CSimpleIniA::Entry str : *list) {
		std::string key(str.pItem);
		ToLower(key);

		if (!key.empty()) {
			auto cValue = iniSettings.GetValue(section, str.pItem);
			std::string value = iniSettings.GetValue(section, str.pItem);
			ToLower(value);

			int iValue = 0;
			if (stricmp(cValue,"true"))
				iValue = 1;

			if (stricmp(str.pItem,"disabledegradation") || stricmp(str.pItem,"onlyplayer") || stricmp(str.pItem,"nobreakmagicdisallowenchanting") || stricmp(str.pItem,"increasedurability")) {
				degradationMap.at(key) = iValue;
			} else if (stricmp(str.pItem,"disabledynamictemper")) {
				temperMap.at(key) = iValue;
			} else if (stricmp(str.pItem,"disablewidget") || stricmp(str.pItem,"hidepoisonname") || stricmp(str.pItem,"hidewithweaponssheathed")) {
				widgetMap.at(key) = iValue;
			} else {
				iValue = 0;
				try {
					iValue = std::stoi(value, nullptr, 0);
				} catch (const std::invalid_argument& e) {
					logger::debug("{} : invalid argument", key);
					logger::debug("   {}", e.what());
				} catch (const std::out_of_range& e) {
					logger::debug("{} : out of range", key);
					logger::debug("   {}", e.what());
				}

				if (stricmp(str.pItem,"equipmenthealththreshold")) {
					degradationMap.at(key) = iValue;
				} else if (stricmp(str.pItem,"temperchance") || stricmp(str.pItem,"bosstemperchance")) {
					temperMap.at(key) = iValue;
				} else if (stricmp(str.pItem,"scale")|| stricmp(str.pItem,"togglekeycode") || stricmp(str.pItem,"flags") || stricmp(str.pItem,"toggleDuration")) {
					widgetMap.at(key) = iValue;
				} else if (stricmp(str.pItem,"positionx") || stricmp(str.pItem,"positiony")) {
					if (iValue <= 100)
						widgetMap.at(key) = iValue;
				} else if (stricmp(str.pItem,"unbreakablecolor") || stricmp(str.pItem,"breakablecolor")) {
					if (iValue <= 0xFFFFFF)
						widgetMap.at(key) = iValue;
				}
			}
		}
	}
}

void INIFile::SetINIData1(std::list<CSimpleIniA::Entry> *list, const char* section, std::unordered_map<std::string, double> *map) {
	CSimpleIniA iniSettings;
	iniSettings.SetUnicode();
	iniSettings.LoadFile(settings_path);

	for (CSimpleIniA::Entry str : *list) {
		std::string key(str.pItem);
		ToLower(key);

		if (!key.empty()) {
			std::string value(iniSettings.GetValue(section, str.pItem));
			double dValue = 0.0;
			try {
				dValue = std::stod(value);
			} catch (const std::invalid_argument& e) {
				logger::debug("{} : invalid argument", key);
				logger::debug("   {}", e.what());
			} catch (const std::out_of_range& e) {
				logger::debug("{} : out of range", key);
				logger::debug("   {}", e.what());
			}

			if (stricmp(str.pItem,"weaponsword") || stricmp(str.pItem,"weapondagger") || stricmp(str.pItem,"weaponwaraxe") || stricmp(str.pItem,"weaponmace")
				|| stricmp(str.pItem,"weapongreatsword") || stricmp(str.pItem,"weaponhammer") || stricmp(str.pItem,"weaponbattleaxe") || stricmp(str.pItem,"weaponbow")
				|| stricmp(str.pItem,"weaponcrossbow") || stricmp(str.pItem,"lightarmor") || stricmp(str.pItem,"heavyarmor") || stricmp(str.pItem,"clothing")
				|| stricmp(str.pItem,"defaultarmor") || stricmp(str.pItem,"powerattackmultiplier") || stricmp(str.pItem,"followermultiplier") || stricmp(str.pItem,"npcmultiplier"))
			{
				if (dValue >= 0.0)
					map->at(key) = dValue;
			}
		}
	}
}

void INIFile::SetINIData2(std::list<CSimpleIniA::Entry> *list, std::unordered_set<int> *set) {
	CSimpleIniA iniBreak;
	iniBreak.SetUnicode();
	iniBreak.LoadFile(break_path);

	for (CSimpleIniA::Entry strESP : *list) {
		std::string esp(strESP.pItem);
		auto index = RE::TESDataHandler::GetSingleton()->GetModIndex(esp);
		
		if (index.value() != 0xFF) {
			std::list<CSimpleIniA::Entry> entryList;
			iniBreak.GetAllKeys(esp.c_str(), entryList);

			for (CSimpleIniA::Entry str : entryList) {
				std::string value = iniBreak.GetValue(strESP.pItem, str.pItem);
				int iValue = std::atoi(value.c_str());
				if (iValue == 1) {
					uint32_t form = 0;
					try {
						form = std::stoi(str.pItem, nullptr, 0);
					} catch (const std::invalid_argument& e) {
						logger::debug("%s : invalid argument", esp.c_str());
						logger::debug("   %s", e.what());
					} catch (const std::out_of_range& e) {
						logger::debug("%s : out of range", esp.c_str());
						logger::debug("   %s", e.what());
					}

					set->insert((index.value() << 24) + form);
				}
			}
		}
	}
}

void INIFile::ShowSettings() {
	logger::debug("Degradation");
	for (auto& map : degradationMap) {
		logger::debug("   {} = {}", map.first, map.second);
	}

	logger::debug("DegradationRate");
	for (auto& map : degradationRateMap) {
		logger::debug("   {} = {}", map.first, map.second);
	}

	logger::debug("BreakChance");
	for (auto& map : breakChanceMap) {
		logger::debug("   {} = {}", map.first, map.second);
	}

	logger::debug("DynamicTemper");
	for (auto& map : temperMap) {
		logger::debug("   {} = {}", map.first, map.second);
	}

	logger::debug("Widget");
	for (auto& map : widgetMap) {
		logger::debug("   {} = {}", map.first, map.second);
	}
}