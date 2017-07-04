// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorSteeringComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineGlobals.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>

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

FVector UActorSteeringComponent::Seek(const FVector& Target)
{
	AActor* Owner = GetOwner();

	FVector ToTarget = Target - Owner->GetActorLocation();
	ToTarget.Normalize();
	FVector DesiredVelocity = ToTarget *mpMovementComponent->GetMaxSpeed();

	return DesiredVelocity - Owner->GetVelocity();
}

FVector UActorSteeringComponent::Flee(const FVector& Target, float TriggerDistance)
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

FVector UActorSteeringComponent::Arrive(const FVector& Target)
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

FVector UActorSteeringComponent::Pursuit(const AActor* TargetActor)
{
	AActor* Owner = GetOwner();
	FVector TargetActorLocation = TargetActor->GetActorLocation();
	FVector TargetActorVelocity = TargetActor->GetVelocity();

	FVector ToTargetActor = TargetActorLocation - Owner->GetActorLocation();
	
	float LookAheadTime = (ToTargetActor.Size() / mpMovementComponent->GetMaxSpeed() + TargetActorVelocity.Size()) / LookAheadTimeModifier;
	return Seek(TargetActorLocation + TargetActorVelocity * LookAheadTime);
}

FVector UActorSteeringComponent::Evade(const AActor* TargetActor, float TriggerDistance)
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

FVector UActorSteeringComponent::ObstacleAvoidance()
{
	static int i = 0;
	i++;
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

float UActorSteeringComponent::RandomClamped()
{
	return FMath::FRand() - FMath::FRand();
}