// Fill out your copyright notice in the Description page of Project Settings.

#include "SpellCaster.h"
#include "BaseWeaponNative.h"


// Sets default values
ABaseWeaponNative::ABaseWeaponNative()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseWeaponNative::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseWeaponNative::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	FVector NewLocation = GetActorLocation();
	float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
	NewLocation.Z += DeltaHeight * 20.0f;       //Scale our height by a factor of 20
	RunningTime += DeltaTime;
	//SetActorLocation(NewLocation);

	TickWeaponEffects(DeltaTime);

	if (GetCurrentFirePhase() == EFirePhase::EFM_FireOnTick)
	{
		if (bIsFiring)
			TryFire();
	}
	else if (GetCurrentFirePhase() == EFirePhase::EFM_ChargeAndRelease)
	{
		if (bIsCharging)
			AddCharge(GetCurrentChargeRate() * DeltaTime);
	}
}

bool ABaseWeaponNative::SetMode(int32 NewMode)
{
	if (NewMode < 0 || NewMode >= NumModes)
		return false;

	StopFiring();

	uint8 PrevMode = ActiveMode;
	ActiveMode = NewMode;

	if (PrevMode != NewMode)
		PlayChangeModeEffects(PrevMode, NewMode);

	return true;
}

void ABaseWeaponNative::StartFiring_Implementation()
{
	if (GetCurrentFirePhase() == EFirePhase::EFM_FireOnPull)
	{
		TryFire();
	}
	else if (GetCurrentFirePhase() == EFirePhase::EFM_FireOnTick)
	{
		bIsFiring = true;
	}
	else if (GetCurrentFirePhase() == EFirePhase::EFM_ChargeAndRelease)
	{
		StartCharging();
	}
}

void ABaseWeaponNative::StopFiring_Implementation()
{
	if (GetCurrentFirePhase() == EFirePhase::EFM_FireOnRelease)
	{
		TryFire();
	}
	else if (GetCurrentFirePhase() == EFirePhase::EFM_FireOnTick)
	{
		bIsFiring = false;
	}
	else if (GetCurrentFirePhase() == EFirePhase::EFM_ChargeAndRelease)
	{
		StopCharging();
		Discharge();
	}
}

bool ABaseWeaponNative::TryFire()
{
	if (CanFire())
	{
		Fire();
		return true;
	}

	return false;
}

void ABaseWeaponNative::Fire()
{
	LastFireTime = UGameplayStatics::GetTimeSeconds(GetWorld());

	if (GetCurrentFireType() == EFireType::EFT_ActorSpawn)
	{
		FireActor();
	}
	else if (GetCurrentFireType() == EFireType::EFT_InstantHit)
	{
		FireInstantHit();
	}
}

void ABaseWeaponNative::FireActor()
{
	uint8 Mode = GetActiveMode();

	if (Mode > FirePhase.Num())
		return;

	UClass* SpawnClass = FiredActorClass[Mode];
	FVector SpawnLocation = GetFireLocation();
	FVector SpawnDirection = GetFireDirection();
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();

	PlayFiringEffects(SpawnLocation, SpawnDirection);

	if (SpawnClass)
	{
		AActor *SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnClass, SpawnLocation, SpawnDirection.Rotation(), SpawnInfo);
		TArray<UActorComponent*> PrimitiveComponents = SpawnedActor->GetComponentsByClass(UPrimitiveComponent::StaticClass());
		for (UActorComponent* ActorComponent : PrimitiveComponents)
		{
			UPrimitiveComponent *PrimitiveComponent = Cast<UPrimitiveComponent>(ActorComponent);
			PrimitiveComponent->IgnoreActorWhenMoving(this, true);
		}
	}
}

void ABaseWeaponNative::FireInstantHit()
{
	const FName TraceTag("MyTraceTag");
	FVector SpawnLocation = GetFireLocation();
	FRotator SpawnRotation = GetFireDirection().Rotation();
	FVector EndLocation = SpawnLocation + (SpawnRotation.Vector() * 5000);

	ECollisionChannel TraceChannel = ECC_Visibility;
	float Radius = 32.0f;
	FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Trace")), true);
	TraceParams.bTraceComplex = true;
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.TraceTag = TraceTag;
	FHitResult HitInfo = FHitResult(ForceInit);
	//Ignore Actors
	//TraceParams.AddIgnoredActor(this);
	/*
	
	GetWorld()->SweepSingle(
		HitInfo,
		SpawnLocation,
		EndLocation,
		FQuat(),
		TraceChannel,
		FCollisionShape::MakeSphere(Radius),
		TraceParams
	);*/
	GetWorld()->DebugDrawTraceTag = TraceTag;
	GetWorld()->SweepSingleByChannel(HitInfo, SpawnLocation, EndLocation, FQuat(), TraceChannel, FCollisionShape::MakeSphere(Radius), TraceParams);
}

bool ABaseWeaponNative::CanFire_Implementation()
{
	float CurrentTime = UGameplayStatics::GetTimeSeconds(GetWorld());

	return CurrentTime > (LastFireTime + GetCurrentFireInterval()) || LastFireTime < 0;
}

int32 ABaseWeaponNative::GetActiveMode()
{
	return ActiveMode;
}

EFirePhase ABaseWeaponNative::GetCurrentFirePhase()
{
	int32 Mode = GetActiveMode();

	if (Mode >= FirePhase.Num())
		return EFirePhase::EFM_None;
	else
		return FirePhase[Mode];
}

float ABaseWeaponNative::GetCurrentFireInterval()
{
	int32 Mode = GetActiveMode();

	if (Mode >= FireInterval.Num())
		return 0;
	else
		return FireInterval[Mode];
}

EFireType ABaseWeaponNative::GetCurrentFireType()
{
	int32 Mode = GetActiveMode();

	if (Mode >= FireType.Num())
		return EFireType::EFT_None;
	else
		return FireType[Mode];
}

FVector ABaseWeaponNative::GetFireLocation_Implementation()
{
	return GetActorLocation();
}

FVector ABaseWeaponNative::GetFireDirection_Implementation()
{
	return GetActorRotation().Vector();
}

void ABaseWeaponNative::StartCharging()
{
	bIsCharging = true;
}

void ABaseWeaponNative::StopCharging()
{
	bIsCharging = false;
}

void ABaseWeaponNative::Discharge()
{
	bool FiredSuccessfully = TryFire();
	ChargeAmount = 0.0f;
}

float ABaseWeaponNative::AddCharge(float ChargeToAdd)
{
	float OriginalChargeAmount = ChargeAmount;
	ChargeAmount = FMath::Max(0.0f, FMath::Min(ChargeAmount + ChargeToAdd, GetCurrentChargeMax()));
	return FMath::Abs(ChargeAmount - OriginalChargeAmount);
}

float ABaseWeaponNative::GetCurrentChargeAmount()
{
	return ChargeAmount;
}

float ABaseWeaponNative::GetCurrentChargeRate()
{
	int32 Mode = GetActiveMode();

	if (Mode >= ChargeRate.Num())
		return 0;
	else
		return ChargeRate[Mode];
}

float ABaseWeaponNative::GetCurrentChargeDecayRate()
{
	int32 Mode = GetActiveMode();

	if (Mode >= ChargeDecayRate.Num())
		return 0;
	else
		return ChargeDecayRate[Mode];
}

float ABaseWeaponNative::GetCurrentChargeMax()
{
	int32 Mode = GetActiveMode();

	if (Mode >= ChargeMax.Num())
		return 0;
	else
		return ChargeMax[Mode];
}

float ABaseWeaponNative::GetCurrentChargeRequired()
{
	int32 Mode = GetActiveMode();

	if (Mode >= ChargeRequired.Num())
		return 0;
	else
		return ChargeRequired[Mode];
}
