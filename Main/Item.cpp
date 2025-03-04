#include "stdafx.h"
#include "Item.h"
#include "ItemValue.h"
#include "ItemOption.h"
#include "CustomItem.h"
#include "CustomBow.h"
#include "CustomWing.h"

CItem gItem;

CItem::CItem()
{

}

CItem::~CItem()
{

}

void CItem::Init()
{
	SetCompleteHook(0xE9, 0x0047B910, &this->ItemConvert);

	SetCompleteHook(0xE9, 0x004C86A9, &this->InsertOptionText);

	SetCompleteHook(0xE8, 0x004C5BC4, &this->OptionAddExcellentDamageRate);

	SetCompleteHook(0xE8, 0x004C5A4E, &this->OptionAddPhysiDamageByLevel);
	SetCompleteHook(0xE8, 0x004C5AEB, &this->OptionAddPhysiDamageByLevel);

	SetCompleteHook(0xE8, 0x004C5B90, &this->OptionMulPhysiDamage);

	SetCompleteHook(0xE8, 0x004C5B5C, &this->OptionAddMagicDamageByLevel);

	SetCompleteHook(0xE8, 0x004C5C2E, &this->OptionAddHuntHP);

	SetCompleteHook(0xE8, 0x004C5C62, &this->OptionAddHuntMP);

	SetCompleteHook(0xE9, 0x004C45C0, &this->myCalcMaxDurability);

	SetCompleteHook(0xE9, 0x0047C690, &this->ItemValue);

	SetCompleteHook(0xE9, 0x004C3EF0, &this->ConvertRepairGold);
	
	SetByte(0x004C5EC4, 0xEB); // Skip reducing damage in staffs
}

void CItem::ItemConvert(ITEM* ip, BYTE Attribute1, BYTE Attribute2)
{
	ip->Level = Attribute1;

	ip->Option1 = Attribute2;

	ITEM_ATTRIBUTE* ItemInfo = (ITEM_ATTRIBUTE*)(ItemAttribute + ip->Type * sizeof(ITEM_ATTRIBUTE));

	ip->TwoHand = ItemInfo->TwoHand;

	ip->WeaponSpeed = ItemInfo->AttackSpeed;

	ip->DamageMin = ItemInfo->DamageMin;

	ip->DamageMax = ItemInfo->DamageMax;

	ip->SuccessfulBlocking = ItemInfo->DefenseRate;

	ip->Defense = ItemInfo->Defense;

	ip->MagicDefense = ItemInfo->MagicDefense;

	ip->WalkSpeed = ItemInfo->WalkSpeed;

	/************************** INSERT ITEM SLOT **************************/

	if (ip->Type >= GET_ITEM(0, 0) && ip->Type < GET_ITEM(4, 0)) // Swords, Axes, Maces, Spears
	{
		ip->Part = EQUIPMENT_WEAPON_RIGHT;
	}
	else if ((ip->Type >= GET_ITEM(4, 0) && ip->Type < GET_ITEM(4, 8)) || ip->Type == GET_ITEM(4, 17) || gCustomBow.IsCustomBow(ip->Type)) // Bows, Bolts
	{
		ip->Part = EQUIPMENT_WEAPON_LEFT;
	}
	else if ((ip->Type >= GET_ITEM(4, 8) && ip->Type < GET_ITEM(4, 19)) && ip->Type != GET_ITEM(4, 17) || gCustomBow.IsCustomCrossbow(ip->Type)) // Crossbows, Arrows
	{
		ip->Part = EQUIPMENT_WEAPON_RIGHT;
	}
	else if (ip->Type >= GET_ITEM(5, 0) && ip->Type < GET_ITEM(6, 0)) // Staffs
	{
		ip->Part = EQUIPMENT_WEAPON_RIGHT;
	}
	else if (ip->Type >= GET_ITEM(6, 0) && ip->Type < GET_ITEM(7, 0)) // Shields
	{
		ip->Part = EQUIPMENT_WEAPON_LEFT;
	}
	else if (ip->Type >= GET_ITEM(7, 0) && ip->Type < GET_ITEM(8, 0)) // Helms
	{
		ip->Part = EQUIPMENT_HELM;
	}
	else if (ip->Type >= GET_ITEM(8, 0) && ip->Type < GET_ITEM(9, 0)) // Armors
	{
		ip->Part = EQUIPMENT_ARMOR;
	}
	else if (ip->Type >= GET_ITEM(9, 0) && ip->Type < GET_ITEM(10, 0)) // Pants
	{
		ip->Part = EQUIPMENT_PANTS;
	}
	else if (ip->Type >= GET_ITEM(10, 0) && ip->Type < GET_ITEM(11, 0)) // Gloves
	{
		ip->Part = EQUIPMENT_GLOVES;
	}
	else if (ip->Type >= GET_ITEM(11, 0) && ip->Type < GET_ITEM(12, 0)) // Boots
	{
		ip->Part = EQUIPMENT_BOOTS;
	}
	else if (ip->Type >= GET_ITEM(12, 0) && ip->Type <= GET_ITEM(12, 6) // Wings
		 || gCustomWing.GetInfoByIndex(ip->Type) != NULL) // Custom Wings
	{
		ip->Part = EQUIPMENT_WING;
	}
	else if (ip->Type >= GET_ITEM(13, 0) && ip->Type <= GET_ITEM(13, 3)) // Angel, Imp, Uniria, Dinorant
	{
		ip->Part = EQUIPMENT_HELPER;
	}
	else if (ip->Type >= GET_ITEM(13, 12) && ip->Type <= GET_ITEM(13, 13)) // Pendants
	{
		ip->Part = EQUIPMENT_AMULET;
	}
	else if (ip->Type >= GET_ITEM(13, 8) && ip->Type <= GET_ITEM(13, 10)) // Rings
	{
		ip->Part = EQUIPMENT_RING_RIGHT;
	}
	else
	{
		ip->Part = -1;
	}

	int m_Level = GET_ITEM_OPT_LEVEL(Attribute1);

	int m_Excellent = GET_ITEM_OPT_EXC(Attribute2);

	int ExceOption = GET_ITEM_OPT_EXC(Attribute2);

	if (ip->Type == GET_ITEM(0, 19) || ip->Type == GET_ITEM(2, 13) || ip->Type == GET_ITEM(4, 18) || ip->Type == GET_ITEM(5, 10)) // Sword of Archangel,Scepter of Archangel,Crossbow of Archangel,Staff of Archangel
	{
		ExceOption = 0;
	}

	if (ip->Part == EQUIPMENT_WING)
	{
		ExceOption = 0;
	}

	if (ip->Type == GET_ITEM(13, 3)) // Dinorant
	{
		ExceOption = 0;
	}

	/************************** STATS REQUIREMENT **************************/

	// If it is a normal item
	int ItemLevel = ItemInfo->Level;

	if (ExceOption) // In case of excellent item
	{
		ItemLevel += 25;
	}

	/************************** REQUIRE STRENGTH **************************/

	if (ItemInfo->RequireStrength != 0)
	{
		ip->RequireStrength = (((ItemInfo->RequireStrength * ((m_Level * 3) + ItemLevel)) * 3) / 100) + 20;
	}
	else
	{
		ip->RequireStrength = 0;
	}

	/************************** REQUIRE AGILITY **************************/

	if (ItemInfo->RequireAgility != 0)
	{
		ip->RequireDexterity = (((ItemInfo->RequireAgility * ((m_Level * 3) + ItemLevel)) * 3) / 100) + 20;
	}
	else
	{
		ip->RequireDexterity = 0;
	}

	/************************** REQUIRE ENERGY **************************/

	if (ItemInfo->RequireEnergy != 0)
	{
		ip->RequireEnergy = (((ItemInfo->RequireEnergy * ((m_Level * 3) + ItemLevel)) * 4) / 100) + 20;
	}
	else
	{
		ip->RequireEnergy = 0;
	}

	/************************** REQUIRE LEVEL **************************/

	if (ip->Type == GET_ITEM(13, 10)) // Transformation Ring
	{
		if (ItemLevel <= 2)
		{
			ip->RequireLevel = 20;
		}
		else
		{
			ip->RequireLevel = 50;
		}
	}
	else if (ItemInfo->RequireLevel != 0)
	{
		if (ip->Type >= GET_ITEM(0, 0) && ip->Type < GET_ITEM(12, 0))
		{
			ip->RequireLevel = ItemInfo->RequireLevel;
		}
		else if ((ip->Type >= GET_ITEM(12, 3) && ip->Type <= GET_ITEM(12, 6)) // Wings
			 || gCustomWing.GetInfoByIndex(ip->Type) != NULL) // Custom Wings
		{
			ip->RequireLevel = ItemInfo->RequireLevel + (m_Level * 5);
		}
		else if ((ip->Type >= GET_ITEM(12, 7) && ip->Type <= GET_ITEM(12, 19) && ip->Type != GET_ITEM(12, 15))) // Orbs,Scrolls
		{
			ip->RequireLevel = ItemInfo->RequireLevel;
		}
		else
		{
			ip->RequireLevel = ItemInfo->RequireLevel + (m_Level * 4);
		}
	}
	else
	{
		ip->RequireLevel = 0;
	}

	if (ExceOption != 0 && ip->RequireLevel > 0)
	{
		if (ip->Type >= GET_ITEM(12, 0))
		{
			ip->RequireLevel += 20;
		}
	}

	/************************** MINS, MAXS AND RATES **************************/

	int ChaosItem = 0;

	if (ip->Type == GET_ITEM(2, 6)) // Chaos Dragon Axe
	{
		ChaosItem = 15;
	}
	else if (ip->Type == GET_ITEM(5, 7)) // Chaos Lightning Staff
	{
		ChaosItem = 25;
	}
	else if (ip->Type == GET_ITEM(4, 6)) // Chaos Nature Bow
	{
		ChaosItem = 30;
	}

	/************************** DAMAGE MAX **************************/

	if (ItemInfo->DamageMax)
	{
		if (ExceOption != 0)
		{
			if (ChaosItem != 0)
			{
				ip->DamageMax += ChaosItem;
			}
			else if (ItemInfo->Level != 0)
			{
				ip->DamageMax += ((ip->DamageMin * 25) / ItemInfo->Level) + 5;
			}
		}

		ip->DamageMax += (m_Level * 3);

		if (m_Level >= 10)
		{
			ip->DamageMax += ((m_Level - 9) * (m_Level - 8)) / 2;
		}
	}

	/************************** DAMAGE MIN **************************/

	if (ItemInfo->DamageMin)
	{
		if (ExceOption != 0)
		{
			if (ChaosItem != 0)
			{
				ip->DamageMin += ChaosItem;
			}
			else if (ItemInfo->Level != 0)
			{
				ip->DamageMin += ((ip->DamageMin * 25) / ItemInfo->Level) + 5;
			}
		}

		ip->DamageMin += (m_Level * 3);

		if (m_Level >= 10)
		{
			ip->DamageMin += ((m_Level - 9) * (m_Level - 8)) / 2;
		}
	}

	/************************** DEFENSE RATE **************************/

	if (ItemInfo->DefenseRate)
	{
		if (ExceOption != 0 && ItemInfo->Level != 0)
		{
			ip->SuccessfulBlocking += ((ip->SuccessfulBlocking * 25) / ItemInfo->Level) + 5;
		}

		ip->SuccessfulBlocking += (m_Level * 3);

		if (m_Level >= 10)
		{
			ip->SuccessfulBlocking += ((m_Level - 9) * (m_Level - 8)) / 2;
		}
	}

	/************************** DEFENSE **************************/

	if (ItemInfo->Defense)
	{
		if (ip->Type >= GET_ITEM(6, 0) && ip->Type < GET_ITEM(7, 0)) // Shields
		{
			ip->Defense += m_Level;
		}
		else
		{
			if (ExceOption != 0 && ItemInfo->Level != 0)
			{
				ip->Defense += (((ip->Defense * 12) / ItemInfo->Level) + (ItemInfo->Level / 5)) + 4;
			}

			if ((ip->Type >= GET_ITEM(12, 3) && ip->Type <= GET_ITEM(12, 6))) // Wings
			{
				ip->Defense += (m_Level * 2);
			}
			else if (gCustomWing.GetInfoByIndex(ip->Type) != NULL) // Custom Wings
			{
				ip->Defense += gCustomWing.GetCustomWingDefense(ip->Type, m_Level);
			}
			else
			{
				ip->Defense += (m_Level * 3);
			}

			if (m_Level >= 10)
			{
				ip->Defense += ((m_Level - 9) * (m_Level - 8)) / 2;
			}
		}
	}

	/************************** MAGIC DEFENSE **************************/

	if (ItemInfo->MagicDefense)
	{
		ip->MagicDefense += (m_Level * 3);

		if (m_Level >= 10)
		{
			ip->MagicDefense += ((m_Level - 9) * (m_Level - 8)) / 2;
		}
	}

	/************************** INSERT OPTIONS **************************/

	ip->SpecialNum = 0;

	memset(ip->Special, 0, sizeof(ip->Special));

	memset(ip->SpecialValue, 0, sizeof(ip->SpecialValue));

	std::deque<std::pair<int, int>> ToInsertLuckOption;

	std::deque<std::pair<int, int>> ToInsertOptions;

	BYTE SkillOption = GET_ITEM_OPT_SKILL(Attribute1);

	BYTE LuckOption = GET_ITEM_OPT_LUCK(Attribute1);

	BYTE AdditionalOption = GET_ITEM_OPT_OPT(Attribute1, Attribute2);

	BYTE pItemOption = 0;

	BYTE pItemValue = 0;

	/************************** INSERT EXCELLENT OPTIONS **************************/

	for (int i = SPECIAL_EXCELLENT1; i <= SPECIAL_EXCELLENT6; i++)
	{
		if (gItemOption.GetItemOption(i, ip->Type, SkillOption, LuckOption, AdditionalOption, m_Excellent, &pItemOption, &pItemValue))
		{
			ToInsertOptions.push_back({ pItemOption, gItemOption.GetItemOptionValue(pItemOption, pItemValue, m_Level) });
		}
	}

	/************************** INSERT LUCK OPTION **************************/

	if (gItemOption.GetItemOption(SPECIAL_LUCK_OPTION, ip->Type, SkillOption, LuckOption, AdditionalOption, m_Excellent, &pItemOption, &pItemValue))
	{
		ToInsertLuckOption.push_back({ pItemOption, pItemValue });
	}

	/************************** INSERT ADDITIONAL OPTION **************************/

	if (gItemOption.GetItemOption(SPECIAL_ADDITIONAL_OPTION, ip->Type, SkillOption, LuckOption, AdditionalOption, m_Excellent, &pItemOption, &pItemValue))
	{
		ToInsertLuckOption.push_back({ pItemOption, pItemValue * AdditionalOption });

		switch (ip->Type)
		{
			case GET_ITEM(12, 0): // Wings of Elf
			{
				ip->RequireStrength += (AdditionalOption * 4);

				break;
			}

			case GET_ITEM(12, 1): // Wings of Angel
			{
				ip->RequireStrength += (AdditionalOption * 4);

				break;
			}

			case GET_ITEM(12, 2): // Wings of Satan
			{
				ip->RequireStrength += (AdditionalOption * 4);

				break;
			}

			case GET_ITEM(12, 3): // Wings of Spirit
			{
				ip->RequireStrength += (AdditionalOption * 4);

				break;
			}

			case GET_ITEM(12, 4): // Wings of Soul
			{
				ip->RequireStrength += (AdditionalOption * 4);

				break;
			}

			case GET_ITEM(12, 5): // Wings of Devil
			{
				ip->RequireStrength += (AdditionalOption * 4);

				break;
			}

			case GET_ITEM(12, 6): // Wings of Darkness
			{
				ip->RequireStrength += (AdditionalOption * 4);

				break;
			}

			default:
			{
				ip->RequireStrength += ((gCustomWing.GetInfoByIndex(ip->Type) != NULL) ? (AdditionalOption * 4) : 0);

				break;
			}
		}
	}

	if (ip->Type >= GET_ITEM(12, 0) && ip->Type <= GET_ITEM(12, 6) // Wings
	    || gCustomWing.GetInfoByIndex(ip->Type) != NULL) // Custom Wings
	{
		ToInsertOptions.insert(ToInsertOptions.end(), ToInsertLuckOption.begin(), ToInsertLuckOption.end());
	}
	else
	{
		ToInsertOptions.insert(ToInsertOptions.begin(), ToInsertLuckOption.begin(), ToInsertLuckOption.end());
	}

	/************************** INSERT SKILL OPTION **************************/

	if (gItemOption.GetItemOption(SPECIAL_SKILL_OPTION, ip->Type, SkillOption, LuckOption, AdditionalOption, m_Excellent, &pItemOption, &pItemValue))
	{
		if (pItemOption != SKILL_NONE || (pItemOption = gCustomItem.GetCustomItemSkill(ip->Type)) != SKILL_NONE)
		{
			ToInsertOptions.insert(ToInsertOptions.begin(), { pItemOption, pItemValue });
		}
	}

	for (const auto& InsertOpt : ToInsertOptions)
	{
		ip->Special[ip->SpecialNum] = InsertOpt.first;

		ip->SpecialValue[ip->SpecialNum] = InsertOpt.second;

		ip->SpecialNum++;
	}
}

_declspec(naked) void CItem::InsertOptionText()
{
	static DWORD jmpBack = 0x004C8B71;
	static int i;
	static ITEM* ip;
	static int iMana;

	_asm
	{
		Pushad;
		Mov ip, Ebx;
	}

	for (i = 0; i < ip->SpecialNum; i++)
	{
		if (ip->Special[i] == SKILL_DEFENSE)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[80], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_FALLING_SLASH)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[81], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_LUNGE)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[82], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_UPPERCUT)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[83], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_CYCLONE)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[84], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_SLASH)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[85], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_TRIPLE_SHOT)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[86], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_FIRE_BREATH)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[745], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;

			wsprintf(TextList[TextNum], GlobalText[179]);

			TextListColor[TextNum] = TEXT_COLOR_DARKRED;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == SKILL_POWER_SLASH)
		{
			GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL, NULL);

			wsprintf(TextList[TextNum], GlobalText[98], iMana);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_PHYSI_DAMAGE)
		{
			wsprintf(TextList[TextNum], GlobalText[88], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;

			if (ip->Type == 31)
			{
				wsprintf(TextList[TextNum], GlobalText[89], ip->SpecialValue[i]);

				TextListColor[TextNum] = TEXT_COLOR_BLUE;

				TextBold[TextNum] = false;

				TextNum += 1;
			}
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_MAGIC_DAMAGE)
		{
			wsprintf(TextList[TextNum], GlobalText[89], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE)
		{
			wsprintf(TextList[TextNum], GlobalText[90], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_DEFENSE)
		{
			wsprintf(TextList[TextNum], GlobalText[91], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE)
		{
			wsprintf(TextList[TextNum], GlobalText[87]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;

			wsprintf(TextList[TextNum], GlobalText[94], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_HP_RECOVERY_RATE)
		{
			if (ip->Type < GET_ITEM(13, 14) || ip->Type > GET_ITEM(13, 18))
			{
				wsprintf(TextList[TextNum], GlobalText[92], ip->SpecialValue[i]);
			}

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_MUL_HP)
		{
			wsprintf(TextList[TextNum], GlobalText[622], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_MUL_MP)
		{
			wsprintf(TextList[TextNum], GlobalText[623], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_DAMAGE_REDUCTION)
		{
			wsprintf(TextList[TextNum], GlobalText[624], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_DAMAGE_REFLECT)
		{
			wsprintf(TextList[TextNum], GlobalText[625], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE)
		{
			wsprintf(TextList[TextNum], GlobalText[626], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_MONEY_AMOUNT_DROP_RATE)
		{
			wsprintf(TextList[TextNum], GlobalText[627], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE)
		{
			wsprintf(TextList[TextNum], GlobalText[628], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_LEVEL)
		{
			wsprintf(TextList[TextNum], GlobalText[629], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_MUL_PHYSI_DAMAGE)
		{
			wsprintf(TextList[TextNum], GlobalText[630], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_LEVEL)
		{
			wsprintf(TextList[TextNum], GlobalText[631], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_MUL_MAGIC_DAMAGE)
		{
			wsprintf(TextList[TextNum], GlobalText[632], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_SPEED)
		{
			wsprintf(TextList[TextNum], GlobalText[633], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_HUNT_HP)
		{
			wsprintf(TextList[TextNum], GlobalText[634], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_HUNT_MP)
		{
			wsprintf(TextList[TextNum], GlobalText[635], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_WING_HP)
		{
			wsprintf(TextList[TextNum], GlobalText[740], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_WING_MP)
		{
			wsprintf(TextList[TextNum], GlobalText[741], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE)
		{
			wsprintf(TextList[TextNum], GlobalText[742], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_ADD_BP)
		{
			wsprintf(TextList[TextNum], GlobalText[743], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == ITEM_OPTION_MUL_BP)
		{
			wsprintf(TextList[TextNum], GlobalText[744], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else if (ip->Special[i] == 90)
		{
			wsprintf(TextList[TextNum], GlobalText[746], ip->SpecialValue[i]);

			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
		else
		{
			TextListColor[TextNum] = TEXT_COLOR_BLUE;

			TextBold[TextNum] = false;

			TextNum += 1;
		}
	}

	_asm
	{
		Popad;
		Mov Eax, Dword Ptr Ds : [0x07EAA154] ;
		Jmp[jmpBack];
	}
}

int CItem::OptionAddExcellentDamageRate(char* _Dest, const char* _Format)
{
	return wsprintf(_Dest, _Format, 10);
}

int CItem::OptionAddPhysiDamageByLevel(char* _Dest, const char* _Format)
{
	return wsprintf(_Dest, _Format, 20);
}

int CItem::OptionMulPhysiDamage(char* _Dest, const char* _Format)
{
	return wsprintf(_Dest, _Format, 2);
}

int CItem::OptionAddMagicDamageByLevel(char* _Dest, const char* _Format)
{
	return wsprintf(_Dest, _Format, 20);
}

int CItem::OptionAddHuntHP(char* _Dest, const char* _Format)
{
	return wsprintf(_Dest, _Format, 8);
}

int CItem::OptionAddHuntMP(char* _Dest, const char* _Format)
{
	return wsprintf(_Dest, _Format, 8);
}

WORD CItem::myCalcMaxDurability(ITEM* ip, ITEM_ATTRIBUTE* p, int Level)
{
	if (ip->Type == GET_ITEM(14, 21)) // Rena
	{
		return 1;
	}

	if (ip->Type == GET_ITEM(13, 18) || ip->Type == GET_ITEM(14, 19)) // Invisibility Cloak, Devil's Invitation
	{
		return 1;
	}

	int ItemDurability = p->Durability;

	if (ip->Type >= GET_ITEM(5, 0) && ip->Type < GET_ITEM(6, 0)) // Staffs
	{
		ItemDurability = p->MagicDurability;
	}

	int dur = 0;

	if (Level >= 5)
	{
		if (Level == 10)
		{
			dur = ItemDurability + ((Level * 2) - 3);
		}
		else if (Level == 11)
		{
			dur = ItemDurability + ((Level * 2) - 1);
		}
		else
		{
			dur = ItemDurability + ((Level * 2) - 4);
		}
	}
	else
	{
		dur = ItemDurability + Level;
	}

	if (ip->Type != GET_ITEM(0, 19) && ip->Type != GET_ITEM(4, 18) && ip->Type != GET_ITEM(5, 10) && ip->Part != EQUIPMENT_WING) // Sword of Archangel, Crossbow of Archangel, Staff of Archangel & Wings
	{
		if (GET_ITEM_OPT_EXC(ip->Option1) != 0)
		{
			dur += 15;
		}
	}

	return ((dur > 255) ? 255 : dur);
}

DWORD CItem::ItemValue(ITEM* ip, int goldType)
{
	if (ip->Type == -1)
	{
		return 0;
	}

	BYTE m_ItemLevel = GET_ITEM_OPT_LEVEL(ip->Level);
	BYTE m_ItemSkill = GET_ITEM_OPT_SKILL(ip->Level);
	BYTE m_ItemLuck = GET_ITEM_OPT_LUCK(ip->Level);
	BYTE m_ItemAddOption = GET_ITEM_OPT_OPT(ip->Level, ip->Option1);
	BYTE m_ItemExcellent = GET_ITEM_OPT_EXC(ip->Option1);

	DWORD m_BuyMoney = 0;

	DWORD m_SellMoney = 0;

	ITEM_ATTRIBUTE* ItemInfo = (ITEM_ATTRIBUTE*)(ItemAttribute + ip->Type * sizeof(ITEM_ATTRIBUTE));

	if (ItemInfo->Money != 0)
	{
		m_BuyMoney = ItemInfo->Money;

		m_BuyMoney = ((m_BuyMoney >= 100) ? ((m_BuyMoney / 10) * 10) : m_BuyMoney);

		m_BuyMoney = ((m_BuyMoney >= 1000) ? ((m_BuyMoney / 100) * 100) : m_BuyMoney);

		m_SellMoney = ItemInfo->Money / 3;

		m_SellMoney = ((m_SellMoney >= 100) ? ((m_SellMoney / 10) * 10) : m_SellMoney);

		m_SellMoney = ((m_SellMoney >= 1000) ? ((m_SellMoney / 100) * 100) : m_SellMoney);

		return (goldType == 1) ? m_SellMoney : m_BuyMoney;
	}

	int value = 0;

	if (gItemValue.GetItemValue(ip, &value))
	{
		m_BuyMoney = value;

		m_BuyMoney = ((m_BuyMoney >= 100) ? ((m_BuyMoney / 10) * 10) : m_BuyMoney);

		m_BuyMoney = ((m_BuyMoney >= 1000) ? ((m_BuyMoney / 100) * 100) : m_BuyMoney);

		m_SellMoney = value / 3;

		m_SellMoney = ((m_SellMoney >= 100) ? ((m_SellMoney / 10) * 10) : m_SellMoney);

		m_SellMoney = ((m_SellMoney >= 1000) ? ((m_SellMoney / 100) * 100) : m_SellMoney);

		return (goldType == 1) ? m_SellMoney : m_BuyMoney;
	}

	ULONGLONG price = 0;

	if (ItemInfo->Value > 0)
	{
		price = ((ItemInfo->Value * ItemInfo->Value) * 10) / 12;

		if (ip->Type >= GET_ITEM(14, 0) && ip->Type <= GET_ITEM(14, 8))
		{
			if (ip->Type == GET_ITEM(14, 3) || ip->Type == GET_ITEM(14, 6))
			{
				price *= 2;
			}

			price *= ((ULONGLONG)1 << m_ItemLevel);

			price *= (ULONGLONG)ip->Durability;

			price = ((price > MAX_ITEM_PRICE) ? MAX_ITEM_PRICE : price);

			m_BuyMoney = (DWORD)price;

			m_BuyMoney = ((m_BuyMoney >= 10) ? ((m_BuyMoney / 10) * 10) : m_BuyMoney);

			m_SellMoney = (DWORD)(price / 3);

			m_SellMoney = ((m_SellMoney >= 10) ? ((m_SellMoney / 10) * 10) : m_SellMoney);

			return (goldType == 1) ? m_SellMoney : m_BuyMoney;
		}
	}
	else
	{
		int ItemLevel = ItemInfo->Level + (m_ItemLevel * 3);

		for (int n = 0; n < ip->SpecialNum; n++)
		{
			if (ip->Special[n] != 0 && ip->Type < GET_ITEM(12, 0))
			{
				ItemLevel += 25;

				break;
			}
		}

		if (((ip->Type / MAX_ITEM_TYPE) == 12 && ip->Type > GET_ITEM(12, 6) && gCustomWing.GetInfoByIndex(ip->Type) == NULL) || (ip->Type / MAX_ITEM_TYPE) == 13 || (ip->Type / MAX_ITEM_TYPE) == 15)
		{
			price = ((ItemLevel * ItemLevel) * ItemLevel) + 100;

			for (int n = 0; n < ip->SpecialNum; n++)
			{
				if (ip->Special[n] == ITEM_OPTION_ADD_HP_RECOVERY_RATE)
				{
					price += (price * m_ItemAddOption);

					break;
				}
			}
		}
		else
		{
			switch (m_ItemLevel)
			{
				case 5:
				{
					ItemLevel += 4;

					break;
				}

				case 6:
				{
					ItemLevel += 10;

					break;
				}

				case 7:
				{
					ItemLevel += 25;

					break;
				}

				case 8:
				{
					ItemLevel += 45;

					break;
				}

				case 9:
				{
					ItemLevel += 65;

					break;
				}

				case 10:
				{
					ItemLevel += 95;

					break;
				}

				case 11:
				{
					ItemLevel += 135;

					break;
				}
			}

			if ((ip->Type >= GET_ITEM(12, 0) && ip->Type <= GET_ITEM(12, 6)) // Wings
			    || gCustomWing.GetInfoByIndex(ip->Type) != NULL) // Custom Wings
			{
				price = ((((ItemLevel + 40) * ItemLevel) * ItemLevel) * 11) + 40000000;
			}
			else
			{
				price = ((((ItemLevel + 40) * ItemLevel) * ItemLevel) / 8) + 100;
			}

			if (ip->Type >= GET_ITEM(0, 0) && ip->Type < GET_ITEM(6, 0))
			{
				if (ItemInfo->TwoHand == 0)
				{
					price = (price * 80) / 100;
				}
			}

			if (m_ItemSkill != 0)
			{
				price += (price * 25) / 100;
			}

			if (m_ItemLuck != 0)
			{
				price += (price * 25) / 100;
			}

			if (m_ItemAddOption != 0)
			{
				price += ((m_ItemAddOption == 1) ? ((price * 60) / 100) : 0);

				price += ((m_ItemAddOption == 2) ? ((price * 140) / 100) : 0);

				price += ((m_ItemAddOption == 3) ? ((price * 280) / 100) : 0);

				price += ((m_ItemAddOption == 4) ? ((price * 560) / 100) : 0);
			}

			for (int n = 0; n < 6; n++)
			{
				if ((ip->Option1 & (1 << n)) != 0)
				{
					price += ((ip->Type < GET_ITEM(12, 0)) ? ((price * 100) / 100) : ((price * 25) / 100));
				}
			}
		}
	}

	m_BuyMoney = (DWORD)price;

	m_BuyMoney = ((m_BuyMoney >= 100) ? ((m_BuyMoney / 10) * 10) : m_BuyMoney);

	m_BuyMoney = ((m_BuyMoney >= 1000) ? ((m_BuyMoney / 100) * 100) : m_BuyMoney);

	float m_BaseDurability = (float)gItem.myCalcMaxDurability(ip, ItemInfo, m_ItemLevel);

	m_SellMoney = (DWORD)(price / 3);

	m_SellMoney = ((ip->Part >= EQUIPMENT_WEAPON_RIGHT && ip->Part <= EQUIPMENT_RING_LEFT) ? (m_SellMoney - (DWORD)((m_SellMoney * 0.6) * (1 - (ip->Durability / m_BaseDurability)))) : m_SellMoney);

	m_SellMoney = ((m_SellMoney >= 100) ? ((m_SellMoney / 10) * 10) : m_SellMoney);

	m_SellMoney = ((m_SellMoney >= 1000) ? ((m_SellMoney / 100) * 100) : m_SellMoney);

	return (goldType == 1) ? m_SellMoney : m_BuyMoney;
}

DWORD CItem::ConvertRepairGold(int Gold, int Durability, int MaxDurability, short Type, char* Text)
{
	float m_Durability = (float)Durability;

	float m_BaseDurability = (float)MaxDurability;

	int RepairMoney = 0;

	RepairMoney = Gold / 3;

	RepairMoney = ((RepairMoney > MAX_ITEM_PRICE) ? MAX_ITEM_PRICE : RepairMoney);

	RepairMoney = ((RepairMoney >= 100) ? ((RepairMoney / 10) * 10) : RepairMoney);

	RepairMoney = ((RepairMoney >= 1000) ? ((RepairMoney / 100) * 100) : RepairMoney);

	float sq1 = sqrt((float)RepairMoney);

	float sq2 = sqrt(sq1);

	float value = ((((3.0f * sq1) * sq2) * (1.0f - (m_Durability / m_BaseDurability))) + 1.0f);

	if (m_Durability <= 0)
	{
		value *= 1.4f;
	}

	int money = (int)((RepairEnable == 1) ? (value * 2.5f) : value);

	money = ((money >= 100) ? ((money / 10) * 10) : money);

	money = ((money >= 1000) ? ((money / 100) * 100) : money);

	ConvertGold(money, Text);

	return money;
}