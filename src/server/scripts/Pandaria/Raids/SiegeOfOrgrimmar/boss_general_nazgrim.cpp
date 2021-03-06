/*
 * Copyright (C) 2016 DeathCore <http://www.noffearrdeathproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "siege_of_orgrimmar.h"

enum eSpells
{
    // Stances
    SPELL_BATTLE_STANCE         = 143589,
    SPELL_BERSERKER_STANCE      = 143594,
    SPELL_DEFENSIVE_STANCE      = 143593,

    // Normal Spells
    SPELL_SUNDERING_BLOW        = 143494,
    SPELL_BONECRACKER           = 143638,
    SPELL_COOLING_OFF           = 143484,
    SPELL_EXECUTE               = 143502,

    // Rage Spells
    SPELL_HEROIC_SHOCKWAVE      = 143716,
    SPELL_AFTERSHOCK            = 143712,
    SPELL_KORKRON_BANNER        = 143591,
    SPELL_WAR_SONG              = 143503,
    SPELL_RAVAGER               = 143872,
    SPELL_RAVAGER_VISUAL        = 143874,
    SPELL_KORKRON_BANNER_AURA   = 143536,

    // Kor'kron Ironblade
    SPELL_IRONSTORM             = 143420,
    SPELL_LAST_STAND            = 143427,

    // Kor'kron Arcweaver
    SPELL_ARCANE_SHOCK          = 143432,
    SPELL_MAGISTRIKE            = 143431,
    SPELL_UNSTABLE_BLINK        = 143471,

    // Kor'kron Assassins
    SPELL_STEALTH               = 118969,
    SPELL_BACKSTAB              = 143481,
    SPELL_ASSASSINS_MARK        = 143480,

    // Kor'kron Warshamans
    SPELL_EARTH_SHIELD          = 143475,
    SPELL_EMPOWERED_CHAIN_HEAL  = 143473,
    SPELL_HEALING_TIDE_TOTEM    = 143474,
    SPELL_HEALING_TIDE          = 143477,

    // Kor'kron Sniper
    SPELL_HUNTERS_MARK          = 143882,
    SPELL_SHOOT                 = 143884,
    SPELL_MULTI_SHOT            = 143887,
};

enum Enum
{
    GLOBAL_COOLDOWN = 1,
};

enum Say
{
    SAY_AGGRO                   =0,
    SAY_AGGRO_HORDE             =1,
    SAY_DEATH                   =2,
    SAY_DEATH_HORDE             =3,
    SAY_SUMMON                  =4,
    SAY_KILL                    =5,
};

enum eEvents
{
    EVENT_BATTLE_STANCE      = 1,
	EVENT_BERSERKER_STANCE   = 2,
	EVENT_DEFFENSIVE_STANCE  = 3,
	EVENT_DAMAGE_TAKEN       = 4,

	// Rage Abilities
	EVENT_HEROIC_SHOCKWAVE   = 5,
	EVENT_KORKRON_BANNER     = 6,
	EVENT_WAR_SONG           = 7,
	EVENT_RAVAGER            = 8,
	EVENT_AFTERSHOCK         = 9,

	EVENT_SUMMON_ADDS_ONE    = 10,
	EVENT_SUNDERING_BLOW     = 11,
	EVENT_BONECRACKER        = 12,

	EVENT_BATTLE_STANCE_RAGE = 13,
	EVENT_HEROIC_SHOCKWAVE_J = 14,

	EVENT_IRONSTORM          = 15,
	EVENT_LAST_STAND         = 16,

	EVENT_ARCANE_SHOCK       = 17,
	EVENT_MAGISTRIKE         = 18,
	EVENT_UNSTABLE_BLINK     = 19,
	
	EVENT_BACKSTAB           = 20,
	EVENT_ASSASSINS_MARK     = 21,

	EVENT_EARTH_SHIELD       = 22,
	EVENT_CHAIN_HEAL         = 23,
	EVENT_HEALING_TIDE_TOTEM = 24,

	EVENT_EXECUTE            = 25,

	EVENT_HUNTERS_MARK       = 26,
	EVENT_SHOOT              = 27,
	EVENT_MULTI_SHOT         = 28,

	EVENT_CREATURE_CHECK     = 29,
	EVENT_SUMMON_ADD_IRO_ASS = 30, // Summon Ironblade and Assassins
	EVENT_SUMMON_ADD_ASS_WAR = 31, // Summon Assassins and Warshaman
	EVENT_SUMMON_ADD_IRO_ARC = 32, // Summon Ironblade and Arcweaver
	EVENT_SUMMON_ADD_ARC_ASS = 33, // Summon Arcweaver and Assassins
	EVENT_SUMMON_ADD_IRO_WAR = 34, // Summon Ironblade and Warshaman
	EVENT_SUMMON_ADD_ARC_WAR = 35, // Summon Arcweaver and Warshaman
};

enum eCreatures
{
    CREATURE_KORKRON_IRONBLADE  = 71516,
    CREATURE_KORKRON_ASSASSINS  = 71518,
    CREATURE_KORKRON_ARCWEAVER  = 71517,
    CREATURE_KORKRON_WARSHAMAN  = 71519,
    CREATURE_KORKRON_SNIPERS    = 71656,
};

Position pos[3] =
{
	{ 1568, -4646, -66, 0 },
	{ 1560, -4634, -67, 0 },
	{ 1560, -4622, -66, 5 },
};

void AddRage(Creature* me,uint32 p_BaseValue, uint64 nazgrimGuid)
{
    if (nazgrimGuid == NULL)
        return;

    if (Creature* generalNazgrim = sObjectAccessor->GetCreature(*me, nazgrimGuid))
    {
        uint32 power = generalNazgrim->GetPower(Powers(POWER_RAGE));

        generalNazgrim->SetPower(Powers(POWER_RAGE), power + p_BaseValue);
    }
}

void RemoveRage(Creature* me, uint32 p_BaseValue, uint64 nazgrimGuid)
{
    if (nazgrimGuid == NULL)
        return;

    if (Creature* generalNazgrim = sObjectAccessor->GetCreature(*me, nazgrimGuid))
    {
        uint32 power = generalNazgrim->GetPower(Powers(POWER_RAGE));

        generalNazgrim->SetPower(Powers(POWER_RAGE), power - p_BaseValue);
    }
}

// 71515 - General Nazgrim
class boss_general_nazgrim : public CreatureScript
{
	public:
		boss_general_nazgrim() : CreatureScript("boss_general_nazgrim") { }

		struct boss_general_nazgrimAI : public BossAI
		{
			boss_general_nazgrimAI(Creature* creature) : BossAI(creature, DATA_GENERAL_NAZGRIM)
			{
				pInstance = creature->GetInstanceScript();
			}
			
			InstanceScript* pInstance;
			TeamId TeamIdInInstance;
			
			void Reset() override
			{
				_Reset();
				
				events.Reset();
				
				summons.DespawnAll();
				
				if (pInstance)
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

				me->setFaction(16);
				
				//damage
				const CreatureTemplate* cinfo = me->GetCreatureTemplate();

				switch (me->GetMap()->GetDifficulty())
				{
					case RAID_DIFFICULTY_10MAN_NORMAL:
						me->SetMaxHealth(349523325);
						me->SetHealth(349523325);
						break;
					case RAID_DIFFICULTY_10MAN_HEROIC:
						me->SetMaxHealth(523325523);
						me->SetHealth(523325523);
						if (cinfo)
						{
							me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.8 * cinfo->mindmg);
							me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.8 * cinfo->maxdmg);
							me->UpdateDamagePhysical(BASE_ATTACK);
						}
						break;
					case RAID_DIFFICULTY_25MAN_NORMAL:
						me->SetMaxHealth(976523325);
						me->SetHealth(976523325);
						if (cinfo)
						{
							me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 2 * cinfo->mindmg);
							me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 2 * cinfo->maxdmg);
							me->UpdateDamagePhysical(BASE_ATTACK);
						}
						break;
					case RAID_DIFFICULTY_25MAN_HEROIC:
						me->SetMaxHealth(1523325523);
						me->SetHealth(1523325523);
						if (cinfo)
						{
							me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 2.5 * cinfo->mindmg);
							me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 2.5 * cinfo->maxdmg);
							me->UpdateDamagePhysical(BASE_ATTACK);
						}
						break;
				}

				me->SetPower(POWER_RAGE, 0);
				me->SetInt32Value(UNIT_FIELD_POWER, 0);
				me->SetMaxPower(POWER_RAGE, 1000);
				me->SetInt32Value(UNIT_FIELD_MAX_POWER, 1000);
			}
			
			void JustReachedHome()
            {
                _JustReachedHome();

                if (pInstance)
                    pInstance->SetBossState(DATA_GENERAL_NAZGRIM, FAIL);
            }
			
			void EnterCombat(Unit* attacker)
            {
				_EnterCombat();

                if (pInstance)
                {
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                    pInstance->SetBossState(DATA_GENERAL_NAZGRIM, IN_PROGRESS);
                }

				events.ScheduleEvent(EVENT_BATTLE_STANCE, 0);
				events.ScheduleEvent(EVENT_SUMMON_ADDS_ONE, 45000);
				events.ScheduleEvent(EVENT_SUNDERING_BLOW, 10000);
				events.ScheduleEvent(EVENT_BONECRACKER, 30000);

				if (me->GetMap()->IsHeroic())
					events.ScheduleEvent(EVENT_EXECUTE, 15000);

				if (TeamIdInInstance == TEAM_ALLIANCE)
				{
					Talk(SAY_AGGRO);
				}
				else
				{
					Talk(SAY_AGGRO_HORDE);
				}
            }
			
			void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }
			
			void KilledUnit(Unit* who)
            {
				Talk(SAY_KILL);
            }
			
			void JustDied(Unit* killer)
            {
                _JustDied();

                if (pInstance)
                {
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                    pInstance->SetBossState(DATA_GENERAL_NAZGRIM, DONE);
                }

				if (TeamIdInInstance == TEAM_ALLIANCE)
				{
					Talk(SAY_DEATH);
				}
				else
				{
					Talk(SAY_DEATH_HORDE);
				}
            }
			
			void DamageTaken(Unit* attacker, uint32& damage)
			{
				if (me->HasAura(SPELL_DEFENSIVE_STANCE) && attacker->HasAura(SPELL_SUNDERING_BLOW))
					return;
				else if (me->HasAura(SPELL_DEFENSIVE_STANCE) && !attacker->HasAura(SPELL_SUNDERING_BLOW))
					events.ScheduleEvent(EVENT_DAMAGE_TAKEN, 1000); // Made it as event because rage gain can occur only once per second
			}

			void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
				uint32 RageAmount = me->GetPower(Powers(POWER_RAGE));

				if (RageAmount <= 490 && RageAmount >= 300)
				{
					if (me->HasAura(SPELL_COOLING_OFF))
					{

					}
					else
					{
						events.ScheduleEvent(EVENT_HEROIC_SHOCKWAVE_J, 0);
					}
				}

				if (RageAmount <= 690 && RageAmount >= 500)
				{
					if (me->HasAura(SPELL_COOLING_OFF))
					{

					}
					else
					{
						events.ScheduleEvent(EVENT_KORKRON_BANNER, 0);
					}
				}

				if (RageAmount <= 990 && RageAmount >= 700)
				{
					if (me->HasAura(SPELL_COOLING_OFF))
					{

					}
					else
					{
						events.ScheduleEvent(EVENT_WAR_SONG, 0);
					}
				}

				if (RageAmount == 1000)
				{
					if (me->HasAura(SPELL_COOLING_OFF))
					{

					}
					else
					{
						events.ScheduleEvent(EVENT_RAVAGER, 0);
					}
				}

				switch (events.ExecuteEvent())
				{
					case EVENT_BATTLE_STANCE:
					{
						if (me->HasAura(SPELL_DEFENSIVE_STANCE))
							me->RemoveAura(SPELL_DEFENSIVE_STANCE);

						DoCast(me, SPELL_BATTLE_STANCE);
						events.ScheduleEvent(EVENT_BATTLE_STANCE_RAGE, 1000);
						events.ScheduleEvent(EVENT_BERSERKER_STANCE, 60000);
						break;
					}

					case EVENT_BATTLE_STANCE_RAGE:
					{
						if (me->HasAura(SPELL_BATTLE_STANCE))
						{
							AddRage(me, 10, me->GetGUID());
							events.ScheduleEvent(EVENT_BATTLE_STANCE_RAGE, 1000);
						}

						break;
					}

					case EVENT_BERSERKER_STANCE:
					{
						DoCast(me, SPELL_BERSERKER_STANCE);
						events.ScheduleEvent(EVENT_DEFFENSIVE_STANCE, 60000);
						break;
					}

					case EVENT_DEFFENSIVE_STANCE:
					{
						DoCast(me, SPELL_DEFENSIVE_STANCE);
						events.ScheduleEvent(EVENT_BATTLE_STANCE, 60000);
						break;
					}

					case EVENT_DAMAGE_TAKEN:
					{
						if (me->HasAura(SPELL_DEFENSIVE_STANCE))
							AddRage(me, 50, me->GetGUID());

						break;
					}

					case EVENT_HEROIC_SHOCKWAVE_J: // Hackfix for jump...
					{
						if (Unit* target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						{
							float posX = target->GetPositionX();
							float posY = target->GetPositionY();
							float posZ = target->GetPositionZ();
							float posO = target->GetOrientation();
							me->GetMotionMaster()->MoveJump(posX, posY, posZ, 30.0f, 15.0f);
						}

						events.ScheduleEvent(EVENT_HEROIC_SHOCKWAVE, 1000);
						RemoveRage(me, 300, me->GetGUID());
						break;
					}

					case EVENT_HEROIC_SHOCKWAVE:
					{
						if (me->HasAura(SPELL_COOLING_OFF))
							return;

						DoCast(me, SPELL_HEROIC_SHOCKWAVE);
						DoCast(me, SPELL_COOLING_OFF);

						break;
					}

					case EVENT_KORKRON_BANNER:
					{
						if (me->HasAura(SPELL_COOLING_OFF))
							return;

						DoCast(SPELL_KORKRON_BANNER);
						DoCast(me, SPELL_COOLING_OFF);
						RemoveRage(me, 500, me->GetGUID());
						
						break;
					}

					case EVENT_WAR_SONG:
					{
						if (me->HasAura(SPELL_COOLING_OFF))
							return;

						DoCast(SPELL_WAR_SONG);
						me->AddAura(SPELL_COOLING_OFF, me);
						RemoveRage(me, 700, me->GetGUID());

						break;
					}

					case EVENT_RAVAGER:
					{
						if (me->HasAura(SPELL_COOLING_OFF))
							return;

						if (Unit* target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						{
							DoCast(target, SPELL_RAVAGER);
						}

						me->AddAura(SPELL_COOLING_OFF, me);
						RemoveRage(me, 1000, me->GetGUID());
						break;
					}

					case EVENT_SUMMON_ADDS_ONE:
					{
						int randomCreature = urand(1, 6);

						switch (randomCreature)
						{
							case 1: // Ironblade and Assassins - Done
							{
								me->SummonCreature(CREATURE_KORKRON_IRONBLADE, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
								me->SummonCreature(CREATURE_KORKRON_ASSASSINS, pos[1], TEMPSUMMON_MANUAL_DESPAWN);
								break;
							}

							case 2: // Ironblade and Arcweaver - Done
							{
								me->SummonCreature(CREATURE_KORKRON_IRONBLADE, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
								me->SummonCreature(CREATURE_KORKRON_ARCWEAVER, pos[1], TEMPSUMMON_MANUAL_DESPAWN);
								break;
							}

							case 3: // Ironblade and Warshaman - Done
							{
								me->SummonCreature(CREATURE_KORKRON_IRONBLADE, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
								me->SummonCreature(CREATURE_KORKRON_WARSHAMAN, pos[1], TEMPSUMMON_MANUAL_DESPAWN);
								break;
							}

							case 4: // Assassis and Arcweaver - Done
							{
								me->SummonCreature(CREATURE_KORKRON_ASSASSINS, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
								me->SummonCreature(CREATURE_KORKRON_ARCWEAVER, pos[1], TEMPSUMMON_MANUAL_DESPAWN);
								break;
							}

							case 5: // Assassin and Warshaman - Done
							{
								me->SummonCreature(CREATURE_KORKRON_ASSASSINS, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
								me->SummonCreature(CREATURE_KORKRON_WARSHAMAN, pos[1], TEMPSUMMON_MANUAL_DESPAWN);
								break;
							}

							case 6: // Arcweaver and Warshaman - Done
							{
								me->SummonCreature(CREATURE_KORKRON_ARCWEAVER, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
								me->SummonCreature(CREATURE_KORKRON_WARSHAMAN, pos[1], TEMPSUMMON_MANUAL_DESPAWN);
								break;
							}
						}

						if (me->GetMap()->IsHeroic())
							me->SummonCreature(CREATURE_KORKRON_SNIPERS, pos[2], TEMPSUMMON_MANUAL_DESPAWN);

						Talk(SAY_SUMMON);
						events.ScheduleEvent(EVENT_CREATURE_CHECK, 5000);
						break;
					}

					case EVENT_CREATURE_CHECK:
					{
						// Check for Kor'kron Ironblade
						std::list<Creature*> summonList;
						me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_IRONBLADE, 50.0f);
						if (!summonList.empty())
						{
							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ASSASSINS, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ARC_WAR, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ARCWEAVER, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ASS_WAR, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_WARSHAMAN, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ARC_ASS, 40000);
							}
						}

						// Check for Kor'kron Assassin
						me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ASSASSINS, 50.0f);
						if (!summonList.empty())
						{
							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_IRONBLADE, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_IRO_ASS, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ARCWEAVER, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ARC_ASS, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_WARSHAMAN, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ASS_WAR, 40000);
							}
						}

						// Check for Kor'kron Arcweaver
						me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ARCWEAVER, 50.0f);
						if (!summonList.empty())
						{
							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_IRONBLADE, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_IRO_ARC, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ASSASSINS, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ARC_ASS, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_WARSHAMAN, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ARC_WAR, 40000);
							}
						}

						// Check for Kor'kron Warshaman
						me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_WARSHAMAN, 50.0f);
						if (!summonList.empty())
						{
							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_IRONBLADE, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_IRO_WAR, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ASSASSINS, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ASS_WAR, 40000);
							}

							me->GetCreatureListWithEntryInGrid(summonList, CREATURE_KORKRON_ARCWEAVER, 50.0f);
							if (!summonList.empty())
							{
								events.ScheduleEvent(EVENT_SUMMON_ADD_ARC_WAR, 40000);
							}
						}

						break;
					}

					case EVENT_SUMMON_ADD_ARC_WAR:
					{
						me->SummonCreature(CREATURE_KORKRON_ARCWEAVER, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
						me->SummonCreature(CREATURE_KORKRON_WARSHAMAN, pos[1], TEMPSUMMON_MANUAL_DESPAWN);

						Talk(SAY_SUMMON);
						events.ScheduleEvent(EVENT_SUMMON_ADD_IRO_ASS, 45000);
						break;
					}

					case EVENT_SUMMON_ADD_IRO_ASS:
					{
						me->SummonCreature(CREATURE_KORKRON_IRONBLADE, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
						me->SummonCreature(CREATURE_KORKRON_ASSASSINS, pos[1], TEMPSUMMON_MANUAL_DESPAWN);

						Talk(SAY_SUMMON);
						events.ScheduleEvent(EVENT_SUMMON_ADD_ARC_WAR, 45000);
						break;
					}

					case EVENT_SUMMON_ADD_ASS_WAR:
					{
						me->SummonCreature(CREATURE_KORKRON_ASSASSINS, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
						me->SummonCreature(CREATURE_KORKRON_WARSHAMAN, pos[1], TEMPSUMMON_MANUAL_DESPAWN);

						Talk(SAY_SUMMON);
						events.ScheduleEvent(EVENT_SUMMON_ADD_IRO_ARC, 45000);
						break;
					}

					case EVENT_SUMMON_ADD_IRO_ARC:
					{
						me->SummonCreature(CREATURE_KORKRON_IRONBLADE, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
						me->SummonCreature(CREATURE_KORKRON_ARCWEAVER, pos[1], TEMPSUMMON_MANUAL_DESPAWN);

						Talk(SAY_SUMMON);
						events.ScheduleEvent(EVENT_SUMMON_ADD_ASS_WAR, 45000);
						break;
					}

					case EVENT_SUMMON_ADD_ARC_ASS:
					{
						me->SummonCreature(CREATURE_KORKRON_ASSASSINS, pos[0], TEMPSUMMON_MANUAL_DESPAWN);
						me->SummonCreature(CREATURE_KORKRON_ARCWEAVER, pos[1], TEMPSUMMON_MANUAL_DESPAWN);

						Talk(SAY_SUMMON);
						events.ScheduleEvent(EVENT_SUMMON_ADD_IRO_WAR, 45000);
						break;
					}

					case EVENT_SUNDERING_BLOW:
					{
						DoCastVictim(SPELL_SUNDERING_BLOW);

						events.ScheduleEvent(EVENT_SUNDERING_BLOW, urand(5000, 10000));
						break;
					}

					case EVENT_BONECRACKER:
					{
						if (Unit* target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
						{
							DoCast(target, SPELL_BONECRACKER);
						}

						events.ScheduleEvent(EVENT_BONECRACKER, 30000);
						break;
					}

					case EVENT_EXECUTE:
					{
						DoCastVictim(SPELL_EXECUTE);

						events.ScheduleEvent(EVENT_EXECUTE, 15000);
						break;
					}
				}

				DoMeleeAttackIfReady();
			}
		};

        CreatureAI* GetAI(Creature* pCreature) const
		{
			return new boss_general_nazgrimAI(pCreature);
		}
};

// Done
class mob_orgrimmar_faithful : public CreatureScript
{
    public:
        mob_orgrimmar_faithful() : CreatureScript("mob_orgrimmar_faithful") { }

        struct mob_orgrimmar_faithfulAI: public ScriptedAI
        { 
            mob_orgrimmar_faithfulAI(Creature* creature) :  ScriptedAI(creature)
            { 
                pInstance = creature->GetInstanceScript(); 
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset() override
            {
                events.Reset();
                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(2000000);
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(2000000);
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(5500000);
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(5500000);
                        break;
                }

                me->setFaction(16);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasAura(SPELL_KORKRON_BANNER_AURA))
                {

                    if (Creature * genNazgrim = pInstance->instance->GetCreature(pInstance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                    {
                        if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 10, genNazgrim->GetGUID());

                        if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 20, genNazgrim->GetGUID());

                        DoMeleeAttackIfReady();
                    }
                }
                else
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_orgrimmar_faithfulAI(creature);
        }
};

class mob_korkron_ironblade : public CreatureScript
{
    public:
        mob_korkron_ironblade() : CreatureScript("mob_korkron_ironblade") { }

        struct mob_korkron_ironbladeAI : public ScriptedAI
        {
            mob_korkron_ironbladeAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset() override
            {
                events.Reset();

                const CreatureTemplate* cinfo = me->GetCreatureTemplate();

                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(8000000);
                        me->SetHealth(8000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(9300000);
                        me->SetHealth(9300000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.35 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.35 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(22000000);
                        me->SetHealth(22000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.4 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.4 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(32000000);
                        me->SetHealth(32000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.55 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.55 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                }

                events.ScheduleEvent(EVENT_IRONSTORM, 10000);
                me->SetInCombatWithZone();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;


                events.Update(diff);

                if (HealthBelowPct(50))
                {
                    if (me->HasAura(SPELL_LAST_STAND))
                    {
                    
                    }
                    else
                    {
                        events.ScheduleEvent(EVENT_LAST_STAND, 0);
                    }
                }

                switch (events.ExecuteEvent())
                {
                    case EVENT_IRONSTORM:
                    {
                        DoCast(me, SPELL_IRONSTORM);
                        events.ScheduleEvent(EVENT_IRONSTORM, 30000);
                        break;
                    }

                    case EVENT_LAST_STAND:
                    {
                        DoCast(me, SPELL_LAST_STAND);
                        break;
                    }
                }

                if (me->HasAura(SPELL_KORKRON_BANNER_AURA))
                    if (Creature * genNazgrim = pInstance->instance->GetCreature(pInstance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                    {
                        if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 10, genNazgrim->GetGUID());

                        if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 20, genNazgrim->GetGUID());
                    }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_korkron_ironbladeAI(creature);
        }
};

class mob_korkron_arcweaver : public CreatureScript
{
    public:
        mob_korkron_arcweaver() : CreatureScript("mob_korkron_arcweaver") { }

        struct mob_korkron_arcweaverAI : public ScriptedAI
        {
            mob_korkron_arcweaverAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset() override
            {
                events.Reset();

                const CreatureTemplate* cinfo = me->GetCreatureTemplate();

                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(6000000);
                        me->SetHealth(6000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(7400000);
                        me->SetHealth(7400000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.35 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.35 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(17000000);
                        me->SetHealth(17000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.35 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.35 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(25600000);
                        me->SetHealth(25600000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.35 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.35 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                }

                events.ScheduleEvent(EVENT_ARCANE_SHOCK, urand(5000,8000));
                events.ScheduleEvent(EVENT_MAGISTRIKE, 20000);
                events.ScheduleEvent(EVENT_UNSTABLE_BLINK, 10000);
                me->SetInCombatWithZone();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_ARCANE_SHOCK:
                    {
                        std::list<Unit*> targets;
                        uint32 minTargets = RAID_MODE<uint32>(2, 5, 2, 5);
                        SelectTargetList(targets, minTargets, SELECT_TARGET_RANDOM, 80, true);
                        for (std::list<Unit*>::const_iterator i = targets.begin(); i != targets.end(); ++i)
                            DoCast(*i,SPELL_ARCANE_SHOCK);

                        events.ScheduleEvent(EVENT_ARCANE_SHOCK, urand(5000, 8000));
                        break;
                    }

                    case EVENT_MAGISTRIKE:
                    {
                        DoCastVictim(SPELL_MAGISTRIKE);
                        
                        events.ScheduleEvent(EVENT_MAGISTRIKE, 20000);
                        break;
                    }

                    case EVENT_UNSTABLE_BLINK:
                    {
                        DoCast(SPELL_UNSTABLE_BLINK);

                        events.ScheduleEvent(EVENT_UNSTABLE_BLINK, 10000);
                        break;
                    }		
                }

                if (me->HasAura(SPELL_KORKRON_BANNER_AURA))
                    if (Creature * genNazgrim = pInstance->instance->GetCreature(pInstance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                    {
                        if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 10, genNazgrim->GetGUID());

                        if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 20, genNazgrim->GetGUID());
                    }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_korkron_arcweaverAI(creature);
        }
};

class mob_korkron_assassin : public CreatureScript
{
    public:
        mob_korkron_assassin() : CreatureScript("mob_korkron_assassin") { }

        struct mob_korkron_assassinAI : public ScriptedAI
        {
            mob_korkron_assassinAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset() override
            {
                events.Reset();
                const CreatureTemplate* cinfo = me->GetCreatureTemplate();

                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(7000000);
                        me->SetHealth(7000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(8400000);
                        me->SetHealth(8400000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.5 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.5 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(20000000);
                        me->SetHealth(20000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.5 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.5 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(29500000);
                        me->SetHealth(29500000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.85 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.85 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                }

                DoCast(me, SPELL_STEALTH);
                DoCast(me, 29651);
                me->SetPower(POWER_ENERGY, 100);
                me->SetInt32Value(UNIT_FIELD_POWER, 100);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetInt32Value(UNIT_FIELD_MAX_POWER, 100);
                events.ScheduleEvent(EVENT_BACKSTAB, 5000);
                events.ScheduleEvent(EVENT_ASSASSINS_MARK, 1000);
                if (Unit* target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
                    DoCast(target, SPELL_ASSASSINS_MARK);
                me->SetInCombatWithZone();
            }


            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_ASSASSINS_MARK:
                    {
                        if (Unit* target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                            DoResetThreat();
                            DoCast(target, SPELL_ASSASSINS_MARK);
                            me->AddThreat(target, 1000000.0f);
                            AttackStart(target);
                        }
                        break;
                    }

                    case EVENT_BACKSTAB:
                    {
                        DoCastVictim(SPELL_BACKSTAB);

                        events.ScheduleEvent(EVENT_BACKSTAB, 5000);
                        break;
                    }
                }

                if (me->HasAura(SPELL_KORKRON_BANNER_AURA))
                    if (Creature * genNazgrim = pInstance->instance->GetCreature(pInstance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                    {
                        if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 10, genNazgrim->GetGUID());

                        if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 20, genNazgrim->GetGUID());
                    }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_korkron_assassinAI(creature);
        }
};

class mob_korkron_warshaman : public CreatureScript
{
    public:
        mob_korkron_warshaman() : CreatureScript("mob_korkron_warshaman") { }

        struct mob_korkron_warshamanAI : public ScriptedAI
        {
            mob_korkron_warshamanAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset()
            {
                events.Reset();
                const CreatureTemplate* cinfo = me->GetCreatureTemplate();

                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(7200000);
                        me->SetHealth(7200000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(8900000);
                        me->SetHealth(8900000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.35 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.35 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(20000000);
                        me->SetHealth(20000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.4 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.4 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(31000000);
                        me->SetHealth(31000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.8 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.8 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                }

                events.ScheduleEvent(EVENT_EARTH_SHIELD, urand(10000,15000));
                events.ScheduleEvent(EVENT_CHAIN_HEAL, urand(6000, 9000));
                events.ScheduleEvent(EVENT_HEALING_TIDE_TOTEM, urand(18500, 20500));
                me->SetInCombatWithZone();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_EARTH_SHIELD:
                    {
                        if (Unit* target = DoSelectLowestHpFriendly(50.0f))
                            DoCast(target, SPELL_EARTH_SHIELD);

                        events.ScheduleEvent(EVENT_EARTH_SHIELD, urand(10000, 15000), GLOBAL_COOLDOWN);
                        events.DelayEvents(2000, GLOBAL_COOLDOWN);
                        break;
                    }

                    case EVENT_CHAIN_HEAL:
                    {
                        if (Unit* target = DoSelectLowestHpFriendly(50.0f))
                            DoCast(target, SPELL_EMPOWERED_CHAIN_HEAL);

                        events.ScheduleEvent(EVENT_CHAIN_HEAL, urand(6000, 9000), GLOBAL_COOLDOWN);
                        events.DelayEvents(2000, GLOBAL_COOLDOWN);
                        break;
                    }

                    case EVENT_HEALING_TIDE_TOTEM:
                    {
                        DoCast(me,SPELL_HEALING_TIDE_TOTEM);

                        events.ScheduleEvent(EVENT_HEALING_TIDE_TOTEM, urand(18500, 20500), GLOBAL_COOLDOWN);
                        events.DelayEvents(2000, GLOBAL_COOLDOWN);
                        break;
                    }
                }

                if (me->HasAura(SPELL_KORKRON_BANNER_AURA))
                    if (Creature * genNazgrim = pInstance->instance->GetCreature(pInstance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                    {
                        if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 10, genNazgrim->GetGUID());

                        if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 20, genNazgrim->GetGUID());
                    }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_korkron_warshamanAI(creature);
        }
};

class mob_korkron_sniper : public CreatureScript
{
    public:
        mob_korkron_sniper() : CreatureScript("mob_korkron_sniper") { }

        struct mob_korkron_sniperAI : ScriptedAI
        {
            mob_korkron_sniperAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset() override
            {
                events.Reset();
                const CreatureTemplate* cinfo = me->GetCreatureTemplate();

                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(7000000);
                        me->SetHealth(7000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(8400000);
                        me->SetHealth(8400000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(20000000);
                        me->SetHealth(20000000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.35 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.35 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(29500000);
                        me->SetHealth(29500000);
                        if (cinfo)
                        {
                            me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.35 * cinfo->mindmg);
                            me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 1.35 * cinfo->maxdmg);
                            me->UpdateDamagePhysical(BASE_ATTACK);
                        }
                        break;
                }

                events.ScheduleEvent(EVENT_HUNTERS_MARK, 1000);
                events.ScheduleEvent(EVENT_SHOOT, 1500);
                events.ScheduleEvent(EVENT_MULTI_SHOT, 12000);
                me->SetInCombatWithZone();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_HUNTERS_MARK:
                    {
                        if (Unit* target = SelectTarget(SelectAggroTarget::SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        {
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                            DoResetThreat();
                            DoCast(target, SPELL_HUNTERS_MARK);
                            me->AddThreat(target, 1000000.0f);
                            AttackStart(target);
                        }

                        break;
                    }

                    case EVENT_SHOOT:
                    {
                        DoCastVictim(SPELL_SHOOT);
                        
                        events.ScheduleEvent(EVENT_SHOOT, 2000);
                        break;
                    }

                    case EVENT_MULTI_SHOT:
                    {
                        DoCastVictim(SPELL_MULTI_SHOT);

                        events.ScheduleEvent(EVENT_MULTI_SHOT, 10000);
                        break;
                    }
                }

                if (me->HasAura(SPELL_KORKRON_BANNER_AURA))
                    if (Creature * genNazgrim = pInstance->instance->GetCreature(pInstance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                    {
                        if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 10, genNazgrim->GetGUID());

                        if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 20, genNazgrim->GetGUID());
                    }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_korkron_sniperAI(creature);
        }
};

// Done
class mob_aftershock : public CreatureScript
{
    public:
        mob_aftershock() : CreatureScript("mob_aftershock") { }

        struct mob_aftershockAI : public ScriptedAI
        {
            mob_aftershockAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset()
            {
                events.Reset();
                me->SetReactState(ReactStates::REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_SELECTABLE);
                me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);

                DoCast(SPELL_AFTERSHOCK);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_aftershockAI(creature);
        }
};

// Done
class mob_korkron_banner : public CreatureScript
{
    public:
        mob_korkron_banner() : CreatureScript("mob_korkron_banner") { }

        struct mob_korkron_bannerAI : public ScriptedAI
        {
            mob_korkron_bannerAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset() override
            {
                events.Reset();

                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(1700000);
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(1700000);
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(2500000);
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(2500000);
                        break;
                }

                DoCast(me, SPELL_KORKRON_BANNER_AURA);
                me->SetReactState(ReactStates::REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_korkron_bannerAI(creature);
        }
};

// Done
class mob_healing_tide_totem : public CreatureScript
{
    public:
        mob_healing_tide_totem() : CreatureScript("mob_healing_tide_totem") { }

        struct mob_healing_tide_totemAI : public ScriptedAI
        {
            mob_healing_tide_totemAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
			EventMap events;

            void Reset() override
            {
                events.Reset();

                switch (me->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        me->SetMaxHealth(1200000);
                        me->SetHealth(1200000);
                        break;
                    case RAID_DIFFICULTY_10MAN_HEROIC:
                        me->SetMaxHealth(1200000);
                        me->SetHealth(1200000);
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        me->SetMaxHealth(2300000);
                        me->SetHealth(2300000);
                        break;
                    case RAID_DIFFICULTY_25MAN_HEROIC:
                        me->SetMaxHealth(2300000);
                        me->SetHealth(2300000);
                        break;
                }

                me->SetReactState(ReactStates::REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);

                DoCast(me, SPELL_HEALING_TIDE);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_healing_tide_totemAI(creature);
        }
};

// Done
class mob_ravager : public CreatureScript
{
    public:
        mob_ravager() : CreatureScript("mob_ravager") { }

        struct mob_ravagerAI : public ScriptedAI
        {
            mob_ravagerAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset() override
            {
                events.Reset();
                me->SetReactState(ReactStates::REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->GetMotionMaster()->MoveRandom(40.0f);

                DoCast(me, SPELL_RAVAGER_VISUAL);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_ravagerAI(creature);
        }
};

// Done
class spell_war_song : public SpellScriptLoader
{
    public:
        spell_war_song() : SpellScriptLoader("spell_war_song") { }

        class spell_war_song_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_war_song_SpellScript);

            void HandleOnHit()
            {
                if (Unit* target = GetHitUnit())
                {
                    uint32 health = target->GetMaxHealth();
                    uint32 damage = health / 2; // 50% of the players health is dealt as damage
                    SetHitDamage(damage);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_war_song_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_war_song_SpellScript();
        }
};

// Done
class spell_sundering_blow : public SpellScriptLoader
{
    public:
        spell_sundering_blow() : SpellScriptLoader("spell_sundering_blow") { }

        class spell_sundering_blow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sundering_blow_SpellScript);

            void HandleOnHit()
            {
                if (InstanceScript* m_Instance = GetCaster()->GetInstanceScript())
                    if (Creature * genNazgrim = m_Instance->instance->GetCreature(m_Instance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                    {
                        if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 13, genNazgrim->GetGUID()); // On hit it gives 5 rage to nazgrim

                        if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                            AddRage(genNazgrim, 26, genNazgrim->GetGUID()); // On hit it gives 10 rage to nazgrim

                        if (Unit* target = GetHitUnit())
                            if (Aura* sunderingBlow = target->GetAura(SPELL_SUNDERING_BLOW))
                            {
                                uint32 stacks = sunderingBlow->GetStackAmount();

                                if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                                    AddRage(genNazgrim, 13*stacks, genNazgrim->GetGUID()); // And for every stack of the debuff 5 more rage

                                if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                                    AddRage(genNazgrim, 26*stacks, genNazgrim->GetGUID()); // And for every stack of the debuff 10 more rage
                            }
                    }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_sundering_blow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sundering_blow_SpellScript();
        }
};

// Apply only on offensive spells from adds on boss General Nazgrim
class spell_korkron_banner_aura : public SpellScriptLoader
{
    public:
        spell_korkron_banner_aura() : SpellScriptLoader("spell_korkron_banner_aura") { }

        class spell_korkron_banner_aura_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_korkron_banner_aura_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(SPELL_KORKRON_BANNER_AURA))
                        if (InstanceScript* m_Instance = GetCaster()->GetInstanceScript())
                            if (Creature * genNazgrim = m_Instance->instance->GetCreature(m_Instance->GetData64(Data::DATA_GENERAL_NAZGRIM)))
                            {
                                if (!genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                                    AddRage(genNazgrim, 3, genNazgrim->GetGUID());

                                if (genNazgrim->HasAura(SPELL_BERSERKER_STANCE))
                                    AddRage(genNazgrim, 6, genNazgrim->GetGUID());
                            }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_korkron_banner_aura_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_korkron_banner_aura_SpellScript();
        }
};

void AddSC_boss_general_nazgrim()
{
    new boss_general_nazgrim();

    new mob_orgrimmar_faithful();
    new mob_korkron_ironblade();
    new mob_korkron_arcweaver();
    new mob_korkron_assassin();
    new mob_korkron_warshaman();
    new mob_korkron_sniper();
    new mob_aftershock();
    new mob_korkron_banner();
    new mob_healing_tide_totem();
    new mob_ravager();

    new spell_war_song();
    new spell_sundering_blow();
    new spell_korkron_banner_aura();
};
