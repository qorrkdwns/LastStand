// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMove.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

UPlayerMove::UPlayerMove()
{
	PrimaryComponentTick.bCanEverTick = true; //because need to move every frame
}

void UPlayerMove::BeginPlay()
{
	Super::BeginPlay();

	moveComp->MaxWalkSpeed = walkSpeed;
}

void UPlayerMove::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PlayerMove();
}

void UPlayerMove::SetupInputBinding(class UEnhancedInputComponent* playerInputComponent)
{
	//playerInputComponent->BindAction(iaMouseLook, ETriggerEvent::Triggered, this, &UPlayerMove::Look);
	playerInputComponent->BindAction(iaMove, ETriggerEvent::Triggered, this, &UPlayerMove::Move);
	playerInputComponent->BindAction(ia_Run, ETriggerEvent::Started, this, &UPlayerMove::InputRun);
	playerInputComponent->BindAction(ia_Run, ETriggerEvent::Completed, this, &UPlayerMove::InputRun);
	playerInputComponent->BindAction(iaJump, ETriggerEvent::Started, this, &UPlayerMove::InputJump);
	playerInputComponent->BindAction(iaJump, ETriggerEvent::Completed, this, &UPlayerMove::InputStopJump);
}

void UPlayerMove::Look(const FInputActionValue& inputValue)
{
	FVector2D value = inputValue.Get<FVector2D>();
	me->AddControllerYawInput(value.X);
	me->AddControllerPitchInput(value.Y);
}

void UPlayerMove::Move(const FInputActionValue& inputValue)
{
	FVector2D value = inputValue.Get<FVector2D>();
	direction.X = value.Y;
	direction.Y = value.X;
}

void UPlayerMove::InputRun(const struct FInputActionValue& inputValue)
{
	//auto movement = GetCharacterMovement();
	if (moveComp->MaxWalkSpeed > walkSpeed) {
		moveComp->MaxWalkSpeed = walkSpeed;
	}
	else {
		moveComp->MaxWalkSpeed = runSpeed;
	}
}

void UPlayerMove::InputJump(const struct FInputActionValue& inputValue)
{
	me->Jump();
}

void UPlayerMove::InputStopJump(const struct FInputActionValue& inputValue)
{
	me->StopJumping();
}

void UPlayerMove::PlayerMove()
{
	//direction = FTransform(me->GetControlRotation()).TransformVector(direction); //coordinate transform(world -> local)


	if (direction.X != 0.0f) //if there is X axis value(W/S), move forward/backward in world
	{
		me->AddMovementInput(FVector::ForwardVector, direction.X);
	}

	if (direction.Y != 0.0f) //if there is Y axis value(A/D), move right/left in world
	{
		me->AddMovementInput(FVector::RightVector, direction.Y);
	}

	direction = FVector::ZeroVector; //initialize direction each frame
}

