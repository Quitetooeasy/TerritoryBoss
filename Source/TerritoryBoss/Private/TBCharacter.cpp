// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "TerritoryBoss.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "TBCharacter.h"

//////////////////////////////////////////////////////////////////////////
// ATBCharacter

ATBCharacter::ATBCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our default camera zoom
	CameraZoom = 300.f;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Configure character movement
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->JumpZVelocity = 620.f;
	MoveComp->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SocketOffset = FVector(0, 35, 0);
	CameraBoom->TargetOffset = FVector(0, 0, 55);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->SetupAttachment(RootComponent);
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATBCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATBCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATBCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ATBCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATBCharacter::LookUpAtRate);

	// Handle zoom in and zoom out for camera
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ATBCharacter::CameraZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ATBCharacter::CameraZoomOut);
}

void ATBCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATBCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATBCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATBCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ATBCharacter::CameraZoomIn()
{
	CameraZoom = CameraZoom - 25.0f;
	if (CameraZoom <= 200.0f)
	{
		CameraBoom->TargetArmLength = 200.0f;
		CameraZoom = 200.0f;
	}
	else
	{
		CameraBoom->TargetArmLength = CameraZoom;
	}
}

void ATBCharacter::CameraZoomOut()
{
	CameraZoom = CameraZoom + 25.0f;
	if (CameraZoom >= 400.0f)
	{
		CameraBoom->TargetArmLength = 400.0f;
		CameraZoom = 400.0f;
	}
	else
	{
		CameraBoom->TargetArmLength = CameraZoom;
	}
}
