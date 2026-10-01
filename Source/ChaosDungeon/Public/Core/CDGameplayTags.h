#pragma once

#include "NativeGameplayTags.h"

/**
 * 코드에서 참조하는 GameplayTag는 전부 여기서 선언한다.
 * - 문자열 오타 방지, 태그 목록을 한 곳에서 확인 가능
 * - 스킬별 쿨다운 태그(Cooldown.Skill.XXX)처럼 데이터에서만 쓰는 태그는 에디터(DefaultGameplayTags.ini)에서 추가
 */
namespace CDTags
{
	// 입력 (InputConfig 에서 InputAction 과 연결)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Skill);  // 부모: 드래그 앤 드롭 교체 가능한 슬롯
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Skill_1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Skill_2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Skill_3);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Skill_4);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Dodge);

	// 어빌리티 분류
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_HitReact);

	// 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Casting);          // 선딜~(캔슬 불가 구간) 동안 유지
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invincible);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Immune_Stagger);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Immune_Knockback);

	// 이벤트
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Hit);      // 애님 노티파이: 타격 시점
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Recovery); // 애님 노티파이: 후딜 시작
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit);              // 피격 리액션 트리거

	// 쿨다운
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Dodge);

	// SetByCaller
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Cooldown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Coefficient);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Critical);

	// 메시지 채널 (UCDMessageSubsystem) — 페이로드 구조체는 CDMessages.h
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Combat_Damage);       // FCDDamageMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Combat_Death);        // FCDDeathMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Combat_BossAppeared); // FCDActorMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Stage_Started);       // FCDStageMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Stage_Progress);      // FCDStageMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Stage_Cleared);       // FCDStageMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Stage_PortalEntered); // FCDStageMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Run_DeathCount);      // FCDRunMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Run_Finished);        // FCDRunMessage
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_UI_FloatingText);     // FCDWorldTextMessage

	// UI 레이어 (UCDPrimaryLayout)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_GameMenu);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);
}
