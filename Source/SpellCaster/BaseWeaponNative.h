// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseWeaponNative.generated.h"

UENUM(BlueprintType)
enum class EFirePhase : uint8
{
	EFM_None,
	EFM_FireOnPull,
	EFM_FireOnRelease,
	EFM_FireOnTick,
	EFM_ChargeAndRelease
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_None,
	EFT_ActorSpawn,
	EFT_InstantHit
};

UCLASS()
class SPELLCASTER_API ABaseWeaponNative : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeaponNative();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	
	float RunningTime;

	float LastFireTime = -1.0;

	// Which firemode index are we in?
	int32 ActiveMode;

	UPROPERTY(EditAnywhere)
	uint8 NumModes;

	UPROPERTY(EditAnywhere)
	TArray<float> FireInterval;

	UPROPERTY(EditAnywhere)
	TArray<EFirePhase> FirePhase;

	UPROPERTY(EditAnywhere)
	TArray<EFireType> FireType;

	UPROPERTY(EditAnywhere)
	TArray<UClass*> FiredActorClass;

	bool bIsFiring;

	bool bIsCharging;

	float ChargeAmount;

	UPROPERTY(EditAnywhere)
	TArray<float> ChargeRate;

	UPROPERTY(EditAnywhere)
	TArray<float> ChargeDecayRate;

	UPROPERTY(EditAnywhere)
	TArray<float> ChargeMax;

	UPROPERTY(EditAnywhere)
	TArray<float> ChargeRequired;



	UFUNCTION(BlueprintCallable, Category="SpellCaster Weapon")
	bool SetMode(int32 mode);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SpellCaster Weapon")
	void StartFiring();
	virtual void StartFiring_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SpellCaster Weapon")
	void StopFiring();
	virtual void StopFiring_Implementation();

	UFUNCTION(BlueprintCallable, Category = "SpellCaster Weapon")
	void StartCharging();

	UFUNCTION(BlueprintCallable, Category = "SpellCaster Weapon")
	void StopCharging();

	UFUNCTION(BlueprintCallable, Category = "SpellCaster Weapon")
	float AddCharge(float Amount);

	void Discharge();

	UFUNCTION(BlueprintNativeEvent, Category = "SpellCaster Weapon")
	bool CanFire();
	virtual bool CanFire_Implementation();

	bool TryFire();

	void Fire();
	void FireActor();
	void FireInstantHit();

	UFUNCTION(BlueprintCallable, Category = "SpellCaster Weapon")
	int32 GetActiveMode();

	EFirePhase GetCurrentFirePhase();

	EFireType GetCurrentFireType();

	float GetCurrentFireInterval();

	float GetCurrentChargeRate();

	float GetCurrentChargeDecayRate();

	float GetCurrentChargeMax();

	float GetCurrentChargeRequired();

	UFUNCTION(BlueprintCallable, Category = "SpellCaster Weapon")
	float GetCurrentChargeAmount();

	UFUNCTION(BlueprintNativeEvent, Category = "SpellCaster Weapon")
	FVector GetFireLocation();
	FVector GetFireLocation_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "SpellCaster Weapon")
	FVector GetFireDirection();
	FVector GetFireDirection_Implementation();

	UFUNCTION(BlueprintImplementableEvent, Category = "SpellCaster Weapon")
	void TickWeaponEffects(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "SpellCaster Weapon")
	void PlayChangeModeEffects(int32 PrevMode, int32 NewMode);

	UFUNCTION(BlueprintImplementableEvent, Category = "SpellCaster Weapon")
	void PlayFiringEffects(FVector FireLocation, FVector FireDirection);
};
