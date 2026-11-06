#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Templates/SubclassOf.h"
#include "CDProjectileData.generated.h"

class AActor;
class UAbilitySystemComponent;
class ACDProjectileBase;

UENUM(BlueprintType)
enum class ECDProjectileType : uint8
{
	Straight = 0,
	Parabolic,
};


USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDBossProjectileData
{
	GENERATED_BODY()

public:
	// 투사체 개수
	UPROPERTY(EditAnywhere, Category = Projectile)
	int32 projectileCount = 1;

	// 발사 간격
	UPROPERTY(EditAnywhere, Category = Projectile)
	float Interval = 0.f;

	// 각도
	UPROPERTY(EditAnywhere, Category = Projectile)
	float spreadAngle = 0.f;

	// 범위 
	UPROPERTY(EditAnywhere, Category = Projectile)
	float spawnRadius = 0.f;

	// 방향 전환 시간
	UPROPERTY(EditAnywhere, Category = Projectile)
	float redirectDelay = 0.f;

	// 장판 데미지 판정 간격
	UPROPERTY(EditAnywhere, Category = Projectile)
	float hitInterval = 0.f;
};

USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDProjectileBaseData
{
	GENERATED_BODY()

	// BP 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<ACDProjectileBase> projectileClass;

	// 기본 투사체 설정
	UPROPERTY(EditDefaultsOnly)
	ECDProjectileType projectileType = ECDProjectileType::Straight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float ProjectileGravityScale = 1.f;

	UPROPERTY(Transient)
	FVector launchLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector targetPoint = FVector::ZeroVector;

	// 발사 ASC
	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> sourceASC;

	// 발사한 캐릭터
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> sourceActor;

	// 방향 전환 시 위치를 확인할 대상
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> targetActor;

	// 명중 시 적용할 데미지 Spec
	UPROPERTY(Transient)
	FGameplayEffectSpecHandle damageSpec;

	// 명중 시 적용할 각성기 게이지 Spec
	UPROPERTY(Transient)
	FGameplayEffectSpecHandle awakenSpec;

	// 투사체가 피격 반응을 발생 여부
	UPROPERTY(Transient)
	bool bCausesHitReact = false;

	// 투사체의 넉백 세기
	UPROPERTY(Transient)
	float knockbackStrength = 0.f;

	// 이동 속도
	UPROPERTY(EditAnywhere, Category = Projectile)
	float speed = 1000.f;

	// 유지시간
	UPROPERTY(EditAnywhere, Category = Projectile)
	float lifeTime = 5.f;

	// 판정 크기
	UPROPERTY(EditAnywhere, Category = Projectile)
	float hitRadius = 30.f;

	// 보스용
	UPROPERTY(EditAnywhere, Category = "Projectile / Boss")
	FCDBossProjectileData bossData;

	// 비행 중 캐릭터 명중 검사
	bool bHitActorsFlight = true;
};