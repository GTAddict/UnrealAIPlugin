// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorSteeringComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineGlobals.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"

// Sets default values for this component's properties
UActorSteeringComponent::UActorSteeringComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UActorSteeringComponent::BeginPlay()
{
	Super::BeginPlay();

	mpMovementComponent		= GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	mpCapsuleComponent		= GetOwner()->FindComponentByClass<UCapsuleComponent>();
}


// Called every frame
void UActorSteeringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FVector UActorSteeringComponent::UpdateNavigation() 
{
	if (mpFollowActor)
	{
		FVector FollowActorCurrentLocation = mpFollowActor->GetActorLocation();
		if (FollowActorCurrentLocation != mFollowActorPrevLocation)
		{
			if (FindPath(FollowActorCurrentLocation, mCurrentPath))
			{
				mFollowActorPrevLocation = FollowActorCurrentLocation;
			}
		}
	}

	if (mCurrentPath.Num() > 0)
	{
		FVector Location = mCurrentPath[0];
		
		if (FVector2D(GetOwner()->GetActorLocation()).Equals(FVector2D(Location), PathPointProximityTolerance))
		{
			mCurrentPath.RemoveAt(0);
			if (mCurrentPath.Num() > 0)
			{
				Location = mCurrentPath[0];
			}
		}
		
		if (mCurrentPath.Num() == 1)
		{
			return Arrive(Location);
		}
		else
		{
			return Seek(Location);
		}
	}

	return FVector();
}

#pragma optimize("", off)
bool UActorSteeringComponent::FindPath(const FVector& Target, TArray<FVector>& OutPath) const
{
	if (DoesStraightPathExist(Target))
	{
		OutPath = { Target };
		return true;
	}
	else
	{
		float GameTime = GetWorld()->GetTimeSeconds();
		if (GameTime - mLastPathFind >= PathFindInterval)
		{
			mLastPathFind = GameTime;
			UNavigationPath* Path = GetWorld()->GetNavigationSystem()->FindPathToLocationSynchronously(GetWorld(), GetOwner()->GetActorLocation(), Target);
			if (Path && Path->IsValid() && Path->GetPath()->IsValid())
			{
				TArray<FNavPathPoint>& PathPoints = Path->GetPath()->GetPathPoints();
				OutPath.Empty(PathPoints.Num());

				for (auto& Point : PathPoints)
				{
					OutPath.Push(Point.Location);
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

}
#pragma optimize("", on)

FVector UActorSteeringComponent::Seek(const FVector& Target) const
{
	AActor* Owner = GetOwner();

	FVector ToTarget = Target - Owner->GetActorLocation();
	ToTarget.Normalize();
	FVector DesiredVelocity = ToTarget *mpMovementComponent->GetMaxSpeed();

	return DesiredVelocity - Owner->GetVelocity();
}

FVector UActorSteeringComponent::Flee(const FVector& Target, float TriggerDistance) const
{
	AActor* Owner = GetOwner();

	FVector FromTarget = Owner->GetActorLocation() - Target;

	if (TriggerDistance < 0 || FromTarget.SizeSquared() <= TriggerDistance * TriggerDistance)
	{
		FromTarget.Normalize();
		FVector DesiredVelocity = FromTarget * mpMovementComponent->GetMaxSpeed();
		return DesiredVelocity - Owner->GetVelocity();
	}

	return FVector(0, 0, 0);
}

FVector UActorSteeringComponent::Arrive(const FVector& Target) const
{
	AActor* Owner = GetOwner();

	FVector ToTarget = Target - Owner->GetActorLocation();
	float Distance = ToTarget.Size();

	if (Distance > 0)
	{
		float Speed = Distance / DecelerationCoefficient;
		Speed = FMath::Min(Speed, mpMovementComponent->GetMaxSpeed());

		FVector DesiredVelocity = ToTarget / Distance * Speed;
		return DesiredVelocity - Owner->GetVelocity();
	}

	return FVector(0, 0, 0);
}

FVector UActorSteeringComponent::Pursuit(const AActor* TargetActor) const
{
	AActor* Owner = GetOwner();
	FVector TargetActorLocation = TargetActor->GetActorLocation();
	FVector TargetActorVelocity = TargetActor->GetVelocity();

	FVector ToTargetActor = TargetActorLocation - Owner->GetActorLocation();
	
	float LookAheadTime = (ToTargetActor.Size() / mpMovementComponent->GetMaxSpeed() + TargetActorVelocity.Size()) / LookAheadTimeModifier;
	return Seek(TargetActorLocation + TargetActorVelocity * LookAheadTime);
}

FVector UActorSteeringComponent::Evade(const AActor* TargetActor, float TriggerDistance) const
{
	AActor* Owner = GetOwner();
	FVector TargetActorLocation = TargetActor->GetActorLocation();
	FVector TargetActorVelocity = TargetActor->GetVelocity();

	FVector ToTargetActor = TargetActorLocation - Owner->GetActorLocation();

	if (TriggerDistance < 0 || ToTargetActor.SizeSquared() <= TriggerDistance * TriggerDistance)
	{
		float LookAheadTime = (ToTargetActor.Size() / mpMovementComponent->GetMaxSpeed() + TargetActorVelocity.Size()) / LookAheadTimeModifier;
		return Flee(TargetActorLocation + TargetActorVelocity * LookAheadTime);
	}

	return FVector(0, 0, 0);
}

FVector UActorSteeringComponent::Wander(float DeltaTime)
{
	float Jitter = WanderJitter * DeltaTime;
	mWanderTarget += FVector(RandomClamped() * Jitter, RandomClamped() * Jitter, 0);
	mWanderTarget.Normalize();
	mWanderTarget *= WanderRadius;
	FVector Target = mWanderTarget + FVector(WanderDistance, 0, 0);
	Target = GetOwner()->GetTransform().TransformPosition(Target);
	FVector ToPos = Target - GetOwner()->GetActorLocation();
	ToPos.Normalize();
	FVector DesiredVelocity = ToPos * WanderMaxSpeed;
	return DesiredVelocity - GetOwner()->GetVelocity();
}

FVector UActorSteeringComponent::ObstacleAvoidance() const
{
	AActor* pOwner = GetOwner();
	FHitResult Hit;
	
	float CollisionLookahead			= CollisionLookAhead;
	const FVector StartLocation			= mpCapsuleComponent->GetComponentLocation();
	const FVector EndLocation			= StartLocation + pOwner->GetActorForwardVector() * CollisionLookahead;
	const ECollisionChannel Channel		= mpCapsuleComponent->GetCollisionObjectType();

	FCollisionQueryParams QueryParams(NAME_None, false, pOwner);
	FCollisionResponseParams ResponseParam;
	mpCapsuleComponent->InitSweepCollisionParams(QueryParams, ResponseParam);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, Channel, QueryParams, ResponseParam);
	if (bHit)
	{
		FVector PenetratedAlongHit = Hit.ImpactPoint - EndLocation;
		FVector PenetratedAlongNormal = PenetratedAlongHit.ProjectOnToNormal(Hit.ImpactNormal);

		// Hit.PenetrationDepth will always be 0 because it's a SingleLineTrace, not Multi
		float PenetrationDepth = PenetratedAlongNormal.Size();
		// Consider: Zero out Z-component before returning?
		return (Hit.ImpactNormal * PenetrationDepth);
	}

	return FVector(0, 0, 0);
}

FVector UActorSteeringComponent::GetHidingSpot(const AActor* Obstacle, const FVector& Target) const
{
	FVector ToObstacle = Obstacle->GetActorLocation() - Target;
	ToObstacle.Normalize();

	FVector CheckFromPoint = Obstacle->GetActorLocation() + ToObstacle * SafeRaycastDistanceFromCover;
	FVector OutPoint;
	UPrimitiveComponent* OutComponent;
	Obstacle->ActorGetDistanceToCollision(CheckFromPoint, ECC_WorldStatic, OutPoint, &OutComponent);

	return OutPoint + ToObstacle * DistanceFromCover;
}

// TODO: Perhaps we shouldn't find the closest obstacle and then find the
// hiding spot? Perhaps we should calculate all the hiding spots and seek
// to the nearest one? i.e., the nearest obstacle might not have the nearest
// hiding spot. However that might be expensive? Maybe limit it to 5 closest
// obstacles?
FVector UActorSteeringComponent::Hide(const AActor* Target)
{
	AActor* pOwner = GetOwner();

	static ETraceTypeQuery TQuery = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);

	TArray<AActor*> ActorsToIgnore = { pOwner };
	TArray<FHitResult> OutHits;

	if (UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		pOwner->GetActorLocation(),
		pOwner->GetActorLocation(),
		CoverSearchRadius,
		TQuery,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame,
		OutHits,
		true))
	{
		// Find closest object
		FHitResult ClosestHit;
		ClosestHit.Distance = CoverSearchRadius;
		for (auto Hit : OutHits)
		{
			// Make sure object is not under me or over me
			if (Hit.GetActor()->ActorHasTag(CoverTag) && Hit.Distance < ClosestHit.Distance && IsHiddenBy(Hit.GetActor()))
			{
				ClosestHit = Hit;
			}
		}

		if (ClosestHit.GetActor())
		{
			// Seek works better than Arrive here, and also it's probably better
			// since one won't "arrive" to a hiding spot - you'll probably run
			// full velocity there
			return Seek(GetHidingSpot(ClosestHit.GetActor(), Target->GetActorLocation()));
		}
	}

	return Evade(Target);
}

void UActorSteeringComponent::FollowActor(AActor* Target)
{
	mpFollowActor = Target;
}

void UActorSteeringComponent::StopFollowActor()
{
	mpFollowActor = nullptr;
}

bool UActorSteeringComponent::DoesStraightPathExist(const FVector& Target) const
{
	FVector StartPosition	= GetOwner()->GetActorLocation();
	FVector EndPosition		= FVector(Target.X, Target.Y, StartPosition.Z);	// Check for a path parallel to XY-plane - no jumping or falling

	AActor* pOwner = GetOwner();
	FHitResult OutHit;
	static TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes =
	{
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_PhysicsBody)
	};


	return !UKismetSystemLibrary::CapsuleTraceSingleForObjects(
		GetWorld(),
		StartPosition,
		EndPosition,
		mpCapsuleComponent->GetScaledCapsuleRadius(),
		mpCapsuleComponent->GetScaledCapsuleHalfHeight(),
		ObjectTypes,
		true,
		{ pOwner },
		EDrawDebugTrace::None,
		OutHit,
		true
	);
}

// Make sure that the cover object is not under the capsule, and also make sure
// that the lowest point of the cover object is under atleast the midpoint of capsule
bool UActorSteeringComponent::IsHiddenBy(const AActor* Actor) const
{
	FVector Origin, Extent;
	Actor->GetActorBounds(true, Origin, Extent);

	float ActorHighestZ = Origin.Z + Extent.Z;
	float CapsuleLowestZ = mpCapsuleComponent->GetComponentLocation().Z - mpCapsuleComponent->GetScaledCapsuleHalfHeight();

	float ActorLowestZ = Origin.Z - Extent.Z;
	float CapsuleOrigin = mpCapsuleComponent->GetComponentLocation().Z;

	return CapsuleLowestZ < ActorHighestZ && ActorLowestZ < CapsuleOrigin;
}

float UActorSteeringComponent::RandomClamped()
{
	return FMath::FRand() - FMath::FRand();
}