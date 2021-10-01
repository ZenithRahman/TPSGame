// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Math/UnrealMathUtility.h"

#define OUT
// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 50.f, 70.f);
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//camera zoom properties for aiming
	CameraDefaultFOV = 0.0f;
	CameraZoomedFOV = 35.0f;
	CameraCurrentFOV = 0.0f;
	ZoomInterpSpeed = 20.f;

	//camera sensitivity while aiming
	HipTurnRate = 90.0f;
	HipLookUpRate = 90.0f;
	AimingTurnRate = 20.0f;
	AimingLookUpRate = 20.0f;
	MouseHipLookUpRate = 1.0f;
	MouseHipTurnRate = 1.0f;
	MouseAimingLookUpRate = 0.2f;
	MouseAimingTurnRate = 0.2f;

	//crosshair spread factor
	CrosshairAimFactor = 0.0f;
	CrosshairInAirFactor = 0.0f;
	CrosshairVelocityFactor = 0.0f;
	CrosshairShootingFactor = 0.0f;
	//bullet fire timerhandle variables
	ShootTimeDuration = 0.05f;
	bFiringBullet = false;

	//automatic fire variables
	AutomaticFireRate = 0.1f;
	bShouldFire = true;
	bFireButtonPressed = false;

	//character rotation
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	bAiming = false;

	//don't rotate the player when moving mouse
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	//configure rotation while pressing AD keys
	GetCharacterMovement()->bOrientRotationToMovement = false; //set the rotation
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); //... at this rate


	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	//aiming
	CameraDefaultFOV = FollowCamera->FieldOfView;
	CameraCurrentFOV = CameraDefaultFOV;
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Aiming To Hipfire interpolation
	CameraInterpZoom(DeltaTime);
	//change look rates based on sensitivity while aiming
	SetLookRates();
	//calculate values for crosshair spred
	CalculateCrosshairSpread(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);	
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller!=nullptr)&&Value!=0.0f)
	{
		const FRotator Rotation = FRotator(Controller->GetControlRotation());
		const FRotator YawRotation = FRotator(0.0f, Rotation.Yaw, 0.0f);
		const FVector Direction = FVector(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && Value != 0.0f)
	{
		const FRotator Rotation = FRotator(Controller->GetControlRotation());
		const FRotator YawRotation = FRotator(0.0f, Rotation.Yaw, 0.0f);
		const FVector Direction = FVector(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		
		AddMovementInput(Direction,Value);
		
	}

}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate*BaseLookUpRate*GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate*BaseTurnRate*GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactors{};
	if (bAiming)
	{
		TurnScaleFactors = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactors = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactors);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactors{};
	if (bAiming)
	{
		LookUpScaleFactors = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactors = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactors);
}

void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

	}
	//Fire Montage Play
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance&&HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	//start bullet fire timer for crosshair
	StartCrosshairBulletFire();

}
bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
	{
		//get current size of the viewport
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(OUT ViewportSize);
		}

		//get screen space location of crosshairs
		FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;
		//get world position and direction of crosshairs
		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, OUT CrosshairWorldPosition, OUT CrosshairWorldDirection);
		if (bScreenToWorld) //checking if deprojection was successful
		{
			FHitResult ScreenTraceHit;
			const FVector Start = FVector(CrosshairWorldPosition);
			const FVector End = FVector(CrosshairWorldPosition + CrosshairWorldDirection * 50000);

			//set beam end point to line trace end
			OutBeamLocation = End;

			//line trace from crosshair and world location
			GetWorld()->LineTraceSingleByChannel(OUT ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
			if (ScreenTraceHit.bBlockingHit)  //was there a trace hit ?
			{
				//beam end point is now trace hit location
				OutBeamLocation = ScreenTraceHit.Location;
			}
			//perform a second line trace ,this time from the gun barrel
			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart = FVector(MuzzleSocketLocation);
			const FVector WeaponTraceEnd = FVector(OutBeamLocation);
			GetWorld()->LineTraceSingleByChannel(OUT WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
			if (WeaponTraceHit.bBlockingHit)
			{
				OutBeamLocation = WeaponTraceHit.Location;
			}
			return true;
		}
			return false;

	}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	//Aiming Button Pressed?
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseLookUpRate = AimingLookUpRate;
		BaseTurnRate = AimingTurnRate;
	}
	else
	{
		BaseLookUpRate = HipLookUpRate;
		BaseTurnRate = HipTurnRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.0f,600.f };
	FVector2D VelocityMultiplierRange{ 0.0f,1.0f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.0f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	//calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling()) //is in air ?
	{
		//spread the crosshair slowly in the air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else//character is on the ground
	{
		//shrink the crosshair rapidly while going on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.f);
	}

	//calculate  the crosshair aim factor
	if (bAiming)//are we aiming?
	{
		//shrink crosshair a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else//not aiming
	{
		//spread crosshair back to normal very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	//true 0.05 seconds after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor,0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor,0.f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor+CrosshairInAirFactor-CrosshairAimFactor+CrosshairShootingFactor;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}

