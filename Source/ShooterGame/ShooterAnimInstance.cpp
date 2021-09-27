// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter==nullptr)
	{
		//make sure to cast the character if it is failed in BeginPlay
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		//Get the speed of the player from Velocity
		FVector Velocity = FVector(ShooterCharacter->GetVelocity());
		Velocity.Z = 0.0f;
		Speed = Velocity.Size();

		//Checking the character is in air or not
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		//is the player accelerating ?
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() >0.0f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if (bIsAccelerating)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		/*FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation : %f"), AimRotation.Yaw);
		FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation : %f"),MovementRotation.Yaw);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Blue, RotationMessage);
			GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, MovementRotationMessage);
		}*/
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
