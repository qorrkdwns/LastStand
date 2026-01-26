// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EnemyFSM.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	fsm = CreateDefaultSubobject<UEnemyFSM>(TEXT("FSM"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned; //placed in world or spawned, auto possess to AIController

	GetCharacterMovement()->bEnablePhysicsInteraction = false;

	//미는 힘(Force) 관련 변수들을 전부 0으로 죽임
	GetCharacterMovement()->PushForceFactor = 0.0f;
	GetCharacterMovement()->InitialPushForceFactor = 0.0f;
	GetCharacterMovement()->TouchForceFactor = 0.0f; // 닿았을 때 미는 힘
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (fsm)
	{
		fsm->OnDamageProcess(ActualDamage);
	}
	return ActualDamage;
}

