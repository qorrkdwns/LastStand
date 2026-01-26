// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyFSM.generated.h"

class AItem;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Move,
	Attack,
	Damage,
	Die,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSPROJECT_API UEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FSM)
	EEnemyState mState = EEnemyState::Idle;

	void IdleState();
	void MoveState();
	void AttackState();
	void DieState();

	void OnDamageProcess(float damage);

	float currentTime = 0;

	UPROPERTY(EditDefaultsOnly, Category = FSM)
	class ATPSCharacter* target;

	UPROPERTY()
	class AEnemy* me;

	UPROPERTY(EditAnywhere, Category = FSM)
	float attackRange = 100.0f;

	UPROPERTY(EditAnywhere, Category = FSM)
	float attackDelayTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
	float hp = 30.0f;
	
	UPROPERTY(EditAnywhere, Category = FSM)
	float damageDelayTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = FSM)
	float dieSpeed = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "FSM")
	void AttackCheck(); // 공격 판정 함수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	UAnimMontage* EnemyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	UAnimMontage* EnemyDeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	UAnimMontage* EnemySkillMontage;

	UPROPERTY()
	class UEnemyAnim* anim;

	UPROPERTY()
	class AAIController* ai;

	FVector randomPos;
	bool GetRandomPositionInNavMesh(FVector centerLocation, float radius, FVector& dest);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FSM)
	float EnemyAttackPower = 10.0f;

	//드랍 아이템 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<TSubclassOf<class AItem>> DropItems;

	// 드랍 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float DropChance = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* DieSound;
};
