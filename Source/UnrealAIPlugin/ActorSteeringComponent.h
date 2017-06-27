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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Steering")
	float DecelerationCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Steering")
	float ForwardHeadingTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Steering")
	float LookAheadTimeModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Steering")
	float WanderJitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Steering")
	float WanderRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Steering")
	float WanderDistance;

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

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Character Steering")
	FVector Seek(const FVector& Target);

	UFUNCTION(BlueprintCallable, Category = "Character Steering")
	FVector Flee(const FVector& Target, float TriggerDistance = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Character Steering")
	FVector Arrive(const FVector& Target);

	UFUNCTION(BlueprintCallable, Category = "Character Steering")
	FVector Pursuit(const AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Character Steering")
	FVector Evade(const AActor* TargetActor, float TriggerDistance = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Character Steering")
	FVector Wander(float DeltaTime);

	float RandomClamped();

private:

	class UCharacterMovementComponent* mpMovementComponent;
	FVector mWanderTarget;
	
};
