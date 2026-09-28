#include "Data/CDHitShape.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

void FCDHitShape::FindActors(const UWorld* world, const FVector& origin, const FVector& forward, TArray<AActor*>& outActors) const
{
	// 1) 바운딩 구로 후보를 넓게 모으고 2) 도형 판정으로 거른다
	TArray<FOverlapResult> overlaps;
	world->OverlapMultiByObjectType(overlaps, origin, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(GetBoundingRadius()));

	for (const FOverlapResult& overlap : overlaps)
	{
		AActor* actor = overlap.GetActor();
		if (actor && Contains(origin, forward, actor->GetActorLocation()))
		{
			outActors.AddUnique(actor);
		}
	}
}

bool FCDHitShape::Contains(const FVector& origin, const FVector& forward, const FVector& point) const
{
	// 쿼터뷰이므로 2D 판정
	const FVector toPoint = FVector(point - origin) * FVector(1.f, 1.f, 0.f);
	const FVector direction = forward.GetSafeNormal2D();

	switch (type)
	{
	case ECDHitShapeType::Circle:
	{
		return toPoint.SizeSquared() <= FMath::Square(radius);
	}
	case ECDHitShapeType::Cone:
	{
		const float minDot = FMath::Cos(FMath::DegreesToRadians(angle * 0.5f));
		return toPoint.SizeSquared() <= FMath::Square(radius)
			&& FVector::DotProduct(direction, toPoint.GetSafeNormal()) >= minDot;
	}
	case ECDHitShapeType::Line:
	{
		const float along = FVector::DotProduct(direction, toPoint);
		const float side = FMath::Abs(FVector::CrossProduct(direction, toPoint).Z);
		return along >= 0.f && along <= length && side <= width * 0.5f;
	}
	}
	return false;
}

float FCDHitShape::GetBoundingRadius() const
{
	if (type == ECDHitShapeType::Line)
	{
		return FVector2D(length, width * 0.5f).Size();
	}
	return radius;
}

void FCDHitShape::DrawDebug(const UWorld* world, const FVector& origin, const FVector& forward, const FColor& color, float duration) const
{
#if ENABLE_DRAW_DEBUG
	const FVector direction = forward.GetSafeNormal2D();
	switch (type)
	{
	case ECDHitShapeType::Circle:
	{
		DrawDebugCircle(world, origin, radius, 32, color, false, duration, 0, 2.f, FVector::ForwardVector, FVector::RightVector, false);
		break;
	}
	case ECDHitShapeType::Cone:
	{
		DrawDebugCone(world, origin, direction, radius, FMath::DegreesToRadians(angle * 0.5f), 0.01f, 16, color, false, duration);
		break;
	}
	case ECDHitShapeType::Line:
	{
		const FVector center = origin + direction * length * 0.5f;
		DrawDebugBox(world, center, FVector(length * 0.5f, width * 0.5f, 50.f), direction.ToOrientationQuat(), color, false, duration);
		break;
	}
	}
#endif
}
