// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActorSteeringComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALAIPLUGIN_API UActorSteeringComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActorSteeringComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float DecelerationCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float ForwardHeadingTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float LookAheadTimeModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float WanderJitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float WanderRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float WanderDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float WanderMaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Avoidance")
	float CollisionLookAhead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hiding")
	float DistanceFromCover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hiding")
	float SafeRaycastDistanceFromCover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hiding")
	float CoverSearchRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hiding")
	FName CoverTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool SeekEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool ArriveEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool FleeEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool WanderEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool PursuitEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool EvadeEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool ObstacleAvoidanceEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
	float SeekWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
	float ArriveWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
	float FleeWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
	float WanderWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
	float PursuitWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
	float EvadeWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights")
	float ObstacleAvoidanceWeight;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Steering")
	FVector Seek(const FVector& Target);

	UFUNCTION(BlueprintCallable, Category = "Steering")
	FVector Flee(const FVector& Target, float TriggerDistance = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Steering")
	FVector Arrive(const FVector& Target);

	UFUNCTION(BlueprintCallable, Category = "Steering")
	FVector Pursuit(const AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Steering")
	FVector Evade(const AActor* TargetActor, float TriggerDistance = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Steering")
	FVector Wander(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Obstacle Avoidance")
	FVector ObstacleAvoidance();

	UFUNCTION(BlueprintCallable, Category = "Hiding")
	FVector GetHidingSpot(const AActor* Obstacle, const FVector& Target);

	UFUNCTION(BlueprintCallable, Category = "Hiding")
	FVector Hide(const AActor* Target);

	bool IsHiddenBy(const AActor* Actor);

	float RandomClamped();

private:

	class UCharacterMovementComponent*	mpMovementComponent;
	class UCapsuleComponent*			mpCapsuleComponent;

	FVector								mWanderTarget;
};
