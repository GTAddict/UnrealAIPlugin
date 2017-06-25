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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DecelerationCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardHeadingTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookAheadTimeModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WanderJitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WanderRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WanderDistance;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	FVector Seek(const FVector& Target);

	UFUNCTION(BlueprintCallable)
	FVector Flee(const FVector& Target, float TriggerDistance = -1.0f);

	UFUNCTION(BlueprintCallable)
	FVector Arrive(const FVector& Target);

	UFUNCTION(BlueprintCallable)
	FVector Pursuit(const AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	FVector Evade(const AActor* TargetActor, float TriggerDistance = -1.0f);

	UFUNCTION(BlueprintCallable)
	FVector Wander(float DeltaTime);

	float RandomClamped();

private:

	class UCharacterMovementComponent* mpMovementComponent;
	FVector mWanderTarget;
	
};
