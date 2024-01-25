#pragma once

#include "SimpleIni.h"
#include <unordered_map>
#include <unordered_set>

class INIFile
{
public:
	INIFile();
	void Load();

	int GetDegradationSettings(std::string str);
	double GetDegradationRateSettings(std::string str);
	double GetDegradationRateSettings(RE::TESForm* form);
	double GetBreakChanceSettings(std::string str);
	double GetBreakChanceSettings(RE::TESForm* form);
	int GetTemperSettings(std::string str);
	int GetWidgetSettings(std::string str);

	bool HasNoBreakForms(int formid);

	float EquipmentHealthThreshold;

private:
	void ToLower(std::string &str);
	bool stricmp(const char* str_1, const char* str_2);

	void SetSettings();
	void SetINIData1(std::list<CSimpleIniA::Entry> *list, const char* section);
	void SetINIData1(std::list<CSimpleIniA::Entry> *list, const char* section, std::unordered_map<std::string, double> *map);
	void SetINIData2(std::list<CSimpleIniA::Entry> *list, std::unordered_set<int> *set);
	void ShowSettings();

	std::unordered_map<std::string, int> degradationMap;
	std::unordered_map<std::string, double> degradationRateMap;
	std::unordered_map<std::string, double> breakChanceMap;
	std::unordered_map<std::string, int> temperMap;
	std::unordered_map<std::string, int> widgetMap;

	std::unordered_set<int> noBreakForms;
};

extern INIFile ini;