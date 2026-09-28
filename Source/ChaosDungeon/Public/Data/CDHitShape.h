#pragma once

#include "CoreMinimal.h"
#include "CDHitShape.generated.h"

/** 판정 도형 종류 */
UENUM(BlueprintType)
enum class ECDHitShapeType : uint8
{
	Circle,  // 원형 (장판 = 원형 + 커서 위치 기준)
	Cone,    // 부채꼴
	Line     // 직선 (찌르기/원거리/돌진)
};

/**
 * 판정 도형. 스킬 데이터에 값으로 들어간다.
 * 도형마다 클래스를 만들지 않고 타입 + 수치로 표현한다. (4종이 전부이므로 switch 하나로 충분)
 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDHitShape
{
	GENERATED_BODY()

public:
	/** origin/forward 기준 도형 안의 액터를 찾는다. (적/아군 구분은 호출 측 책임) */
	void FindActors(const UWorld* world, const FVector& origin, const FVector& forward, TArray<AActor*>& outActors) const;

	/** 범위 확인용 디버그 드로우 (콘솔: CD.DebugHitShape 1) */
	void DrawDebug(const UWorld* world, const FVector& origin, const FVector& forward, const FColor& color, float duration) const;

private:
	/** 점이 도형 안에 있는가 (2D 판정) */
	bool Contains(const FVector& origin, const FVector& forward, const FVector& point) const;

	/** 후보 검색용 바운딩 구 반지름 */
	float GetBoundingRadius() const;

public:
	/** 도형 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECDHitShapeType type = ECDHitShapeType::Circle;

	/** 원형/부채꼴 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "type != ECDHitShapeType::Line", EditConditionHides))
	float radius = 300.f;

	/** 부채꼴 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "type == ECDHitShapeType::Cone", EditConditionHides))
	float angle = 90.f;

	/** 직선 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "type == ECDHitShapeType::Line", EditConditionHides))
	float length = 600.f;

	/** 직선 폭 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "type == ECDHitShapeType::Line", EditConditionHides))
	float width = 150.f;
};
