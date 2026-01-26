// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBaseComponent.h"
#include "PlayerMove.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (PlayerComponent), meta = (BlueprintSpawnableComponent))
class TPSPROJECT_API UPlayerMove : public UPlayerBaseComponent
{
	GENERATED_BODY()

public:
	//Sets default values for this component's properties
	UPlayerMove();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetupInputBinding(class UEnhancedInputComponent	* playerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* iaMouseLook;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* iaMove;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* ia_Run;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* iaJump;

	FVector direction; //direction to move

	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float walkSpeed = 275;

	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float runSpeed = 600;

	void Move(const struct FInputActionValue& value);
	void Look(const struct FInputActionValue& value);
	void InputRun(const struct FInputActionValue& inputValue);
	void InputJump(const struct FInputActionValue& inputValue);
	void InputStopJump(const struct FInputActionValue& inputValue);

	void PlayerMove();

};
