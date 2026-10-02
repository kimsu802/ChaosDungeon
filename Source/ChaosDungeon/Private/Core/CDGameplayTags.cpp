#include "Core/CDGameplayTags.h"

namespace CDTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_Skill, "Input.Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Skill_1, "Input.Skill.1", "Q");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Skill_2, "Input.Skill.2", "W");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Skill_3, "Input.Skill.3", "E");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Skill_4, "Input.Skill.4", "R");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Attack, "Input.Attack", "일반 공격");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Dodge, "Input.Dodge", "Space");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill, "Ability.Skill");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge, "Ability.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_HitReact, "Ability.HitReact");

	// 10.01 Jun6 - 일반공격 GA 구분용 Tag 추가
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack, "Ability.Attack");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Casting, "State.Casting", "다른 스킬 사용 불가 구간. 회피는 가능");
	UE_DEFINE_GAMEPLAY_TAG(State_HitReact, "State.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Invincible, "State.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Immune_Stagger, "State.Immune.Stagger");
	UE_DEFINE_GAMEPLAY_TAG(State_Immune_Knockback, "State.Immune.Knockback");

	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Hit, "Event.Montage.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Recovery, "Event.Montage.Recovery");
	UE_DEFINE_GAMEPLAY_TAG(Event_Hit, "Event.Hit");

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Dodge, "Cooldown.Dodge");

	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Cooldown, "SetByCaller.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Coefficient, "SetByCaller.Coefficient");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Critical, "SetByCaller.Critical");

	UE_DEFINE_GAMEPLAY_TAG(Msg_Combat_Damage, "Combat.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Combat_Death, "Combat.Death");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Combat_BossAppeared, "Combat.BossAppeared");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Stage_Started, "Stage.Started");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Stage_Progress, "Stage.Progress");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Stage_Cleared, "Stage.Cleared");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Stage_PortalEntered, "Stage.PortalEntered");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Run_DeathCount, "Run.DeathCount");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Run_Finished, "Run.Finished");
	UE_DEFINE_GAMEPLAY_TAG(Msg_UI_FloatingText, "UI.FloatingText");

	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game, "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_GameMenu, "UI.Layer.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal, "UI.Layer.Modal");
}
