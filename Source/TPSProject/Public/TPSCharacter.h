// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "TPSCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FInputBindingDelegate, class UEnhancedInputComponent*); // Declare a multicast delegate(Input)

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChangedDelegate, float, float); // Declare a multicast delegate(HP Change)

USTRUCT(BlueprintType)
struct FSkillInfo
{
	GENERATED_BODY()

public:
	// 스킬 해금 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxCooldown = 10.0f;

	// 현재 남은 쿨타임
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentCooldown = 0.0f;

	// 스킬 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0f;
};

UCLASS()
class TPSPROJECT_API ATPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSCharacter();

	FInputBindingDelegate onInputBindingDelegate; // Multicast delegate instance(Input)
	FOnHPChangedDelegate OnHPChanged; // Multicast delegate instance(HP Change)

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* springArmComp;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* tpsCamComp;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* followCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* imcDefault;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* imcMouseLook;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Skill1;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Skill2;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Skill3;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ia_Skill4;

	UFUNCTION(BlueprintPure, Category = "UI") //BlueprintPure : 실행 핀 없이 값만 리턴하는 노드
	FString GetPlayTimeText();

	const float MIN_ATTACK_DELAY = 0.3f;  // 공속 최대치 (초당 약 3.3발 제한)
	const int32 MAX_PROJECTILE_COUNT = 5; // 멀티샷 최대치 (5발 제한)

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PlayerStat")
	float hp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PlayerStat")
	float initialHP = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStat")
	float AttackPower = 10.0f; // [공격력]

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStat")
	int32 ProjectileCount = 1; // [총알 개수] (기본 1발)

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStat")
	float AttackSpeed = 0.8f; // [공격 속도] (기본 0.5초) -> 낮을수록 빠름

	UFUNCTION()
	void OnHitEvent(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStat")
	float LastDamageTime = -5.0f; // 마지막으로 맞은 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerStat")
	float DamageDelay = 1.0f;    // 무적 시간

	//UI에 배율이 적용된 최종 공격력 반환
	UFUNCTION(BlueprintPure, Category = "PlayerStat")
	float GetTotalAttackPower() const;

	//UI에 배율이 적용된 최종 공격 속도 반환
	UFUNCTION(BlueprintPure, Category = "PlayerStat")
	float GetTotalAttackSpeed() const;

	//UI에 스킬의 쿨타임 비율(0.0 ~ 1.0) 반환
	UFUNCTION(BlueprintPure, Category = "PlayerStat")
	float GetSkillCooldownRatio(int32 SkillIndex) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<FSkillInfo> Skills; //스킬 4개 관리 (배열)

	float NormalWalkSpeed; // 원래 속도 저장용

	bool bIsShieldActive = false; // 실드 활성화 여부

	float DamageMultiplier = 1.0f; // 공격력 배율 (기본 1.0, 스킬 쓰면 2.0)

	float FireDelayMultiplier = 1.0f; // 공속 배율 (기본 1.0, 스킬 쓰면 0.5 - 딜레이 감소)

	FTimerHandle Timer_Skill1;
	FTimerHandle Timer_Skill2;
	FTimerHandle Timer_Skill3;
	FTimerHandle Timer_Skill4;

	UFUNCTION(BlueprintImplementableEvent, Category = "Skill")
	void OnUnlockSkill(int32 SkillIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Health) // Declare the function as a BlueprintNativeEvent, making it overridable in Blueprints
	void OnGameOver();

	UFUNCTION(BlueprintImplementableEvent, Category = "GameEvent")
	void OnGameClear();

	void UpgradeAttackPower(float Amount);
	void UpgradeProjectileCount(int32 Amount);
	void UpgradeAttackSpeed(float Amount);
	void Heal(float Amount);
	void UpgradeMaxHP(float Amount);
	void DecreaseHP(float Amount);

	void UnlockSkill(int32 Index);

	void Input_Skill1();
	void Input_Skill2();
	void Input_Skill3();
	void Input_Skill4();

	void ActivateSkill(int32 Index); // 실제 스킬 발동

	void EndSkill1(); // 이속 복구
	void EndSkill2(); // 실드 해제
	void EndSkill3(); // 공격력 복구
	void EndSkill4(); // 공속 복구
};
