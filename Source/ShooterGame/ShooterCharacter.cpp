// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
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

