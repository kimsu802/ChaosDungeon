#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "Character/CDBossCharacter.h"
#include "Data/CDBossData.h"
#include "Data/CDBossSkillData.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/ScopeExit.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCDBossPhaseSkillsTest,
    "ChaosDungeon.Boss.PhaseSkills",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCDBossPhaseSkillsTest::RunTest(const FString& parameters)
{
    UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Test world is created"), world))
    {
        return false;
    }
    FWorldContext& worldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    worldContext.SetCurrentWorld(world);
    ON_SCOPE_EXIT
    {
        GEngine->DestroyWorldContext(world);
        world->DestroyWorld(false);
    };
    // 캐릭터 컴포넌트 초기화가 AttributeSet을 ASC에 등록하도록 한다.
    world->InitializeActorsForPlay(FURL());

    UCDBossData* data = NewObject<UCDBossData>(world);
    for (int32 i = 0; i < 4; ++i)
    {
        data->attackSkills.Add(NewObject<UCDBossSkillData>(data));
    }
    for (int32 i = 0; i < 3; ++i)
    {
        data->phase2Skill.Add(NewObject<UCDBossSkillData>(data));
    }

    // 같은 원본 데이터를 사용하는 다른 보스에게 전환이 전파되면 안 된다.
    ACDBossCharacter* boss = world->SpawnActor<ACDBossCharacter>();
    ACDBossCharacter* otherBoss = world->SpawnActor<ACDBossCharacter>();
    if (!TestNotNull(TEXT("Boss is spawned"), boss)
        || !TestNotNull(TEXT("Other boss is spawned"), otherBoss))
    {
        return false;
    }

    const FCDDifficultySettings difficulty;
    // 풀 매니저와 동일하게 부모 클래스의 공개 진입점으로 활성화한다.
    static_cast<ACDMonsterBase*>(boss)->ActivateFromPool(data, difficulty, FVector::ZeroVector);
    static_cast<ACDMonsterBase*>(otherBoss)->ActivateFromPool(data, difficulty, FVector(1000.f, 0.f, 0.f));
    UCDAbilitySystemComponent* abilitySystem = boss->GetCDAbilitySystemComponent();
    const int32 initialAbilityCount = abilitySystem->GetActivatableAbilities().Num();

    auto HasGrantedSkill = [abilitySystem](const UCDSkillData* skill)
    {
        for (const FGameplayAbilitySpec& spec : abilitySystem->GetActivatableAbilities())
        {
            if (spec.SourceObject.Get() == skill)
            {
                return true;
            }
        }
        return false;
    };

    TestEqual(TEXT("Phase 1 has four attack candidates"), boss->GetActiveAttackSkills().Num(), 4);
    for (const UCDSkillData* skill : data->phase2Skill)
    {
        TestFalse(TEXT("Phase 2 skills are not granted before transition"), HasGrantedSkill(skill));
    }

    boss->EnterPhase2();
    TestEqual(TEXT("Phase 2 has seven attack candidates"), boss->GetActiveAttackSkills().Num(), 7);
    TestEqual(TEXT("Exactly three abilities are added"), abilitySystem->GetActivatableAbilities().Num(), initialAbilityCount + 3);
    for (const UCDSkillData* skill : data->attackSkills)
    {
        TestTrue(TEXT("Base attacks remain available"), boss->GetActiveAttackSkills().Contains(skill));
        TestTrue(TEXT("Base abilities remain granted"), HasGrantedSkill(skill));
    }
    for (const UCDSkillData* skill : data->phase2Skill)
    {
        TestTrue(TEXT("Additional attacks become candidates"), boss->GetActiveAttackSkills().Contains(skill));
        TestTrue(TEXT("Additional abilities are granted"), HasGrantedSkill(skill));
    }
    TestEqual(TEXT("Other boss stays in phase 1"), otherBoss->GetActiveAttackSkills().Num(), 4);
    TestEqual(TEXT("Shared base data is unchanged"), data->attackSkills.Num(), 4);

    boss->EnterPhase2();
    TestEqual(TEXT("Repeated transition does not duplicate candidates"), boss->GetActiveAttackSkills().Num(), 7);
    TestEqual(TEXT("Repeated transition does not duplicate abilities"), abilitySystem->GetActivatableAbilities().Num(), initialAbilityCount + 3);

    static_cast<ACDMonsterBase*>(boss)->DeactivateToPool();
    static_cast<ACDMonsterBase*>(boss)->ActivateFromPool(data, difficulty, FVector::ZeroVector);
    TestEqual(TEXT("Pool reuse restores four base attacks"), boss->GetActiveAttackSkills().Num(), 4);
    TestEqual(TEXT("Pool reuse removes additional abilities"), abilitySystem->GetActivatableAbilities().Num(), initialAbilityCount);
    for (const UCDSkillData* skill : data->phase2Skill)
    {
        TestFalse(TEXT("Phase 2 abilities are removed on reuse"), HasGrantedSkill(skill));
    }

    // 잘못 중복 등록한 데이터와 빈 항목도 추가 부여하지 않는다.
    UCDSkillData* duplicateSkill = data->phase2Skill[0].Get();
    data->phase2Skill.Add(data->attackSkills[0]);
    data->phase2Skill.Add(duplicateSkill);
    data->phase2Skill.Add(nullptr);
    boss->EnterPhase2();
    TestEqual(TEXT("Second life still adds only three unique attacks"), boss->GetActiveAttackSkills().Num(), 7);
    TestEqual(TEXT("Invalid entries do not add abilities"), abilitySystem->GetActivatableAbilities().Num(), initialAbilityCount + 3);

    return true;
}

#endif
