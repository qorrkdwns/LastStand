// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerMove.h"
#include "PlayerFire.h"
#include "TPSProject.h"
#include "Enemy.h"
#include "EnemyFSM.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATPSCharacter::ATPSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->SetRelativeLocation(FVector(0, 0, 1400));
	springArmComp->TargetArmLength = 0;
	springArmComp->bUsePawnControlRotation = false; //Arm does not rotate relative to pawn 

	tpsCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamComp"));
	tpsCamComp->SetupAttachment(springArmComp);
	tpsCamComp->bUsePawnControlRotation = false; //Camera does not rotate relative to arm

	followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera")); // Third-person follow camera
	followCamera->SetupAttachment(tpsCamComp, USpringArmComponent::SocketName); // Attach the camera to the end of the spring arm
	followCamera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	hp = initialHP;
	LastDamageTime = -DamageDelay * 2.0f;

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ATPSCharacter::OnHitEvent);

	APlayerController* pc = Cast<APlayerController>(Controller);
	if (pc != nullptr) {
		pc->bShowMouseCursor = true; // Show the mouse cursor
		pc->bEnableClickEvents = true;
		pc->bEnableMouseOverEvents = true;
		UEnhancedInputLocalPlayerSubsystem* subsys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());

		if (subsys != nullptr) {
			subsys->AddMappingContext(imcDefault, 0);
			subsys->AddMappingContext(imcMouseLook, 0);
		}
	}

	NormalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed; //원래 이속 저장

	Skills.SetNum(4); //스킬 4개

	// Skill 1: 이속 (쿨 10초, 지속 3초)
	Skills[0].MaxCooldown = 10.0f;
	Skills[0].Duration = 3.0f;

	// Skill 2: 실드 (쿨 15초, 지속 3초)
	Skills[1].MaxCooldown = 15.0f;
	Skills[1].Duration = 3.0f;

	// Skill 3: 공격력 (쿨 20초, 지속 5초)
	Skills[2].MaxCooldown = 20.0f;
	Skills[2].Duration = 5.0f;

	// Skill 4: 공속 (쿨 20초, 지속 5초)
	Skills[3].MaxCooldown = 20.0f;
	Skills[3].Duration = 5.0f;
}

// Called every frame
void ATPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		FHitResult HitResult;
		PC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult); // 화면상의 커서 위치에 대한 히트 결과 가져오기

		if (HitResult.bBlockingHit)
		{
			FVector LookTarget = HitResult.Location;
			FVector CurrentLoc = GetActorLocation();

			LookTarget.Z = CurrentLoc.Z; // 수평 회전을 위해 Z 축 고정

			FVector Direction = (LookTarget - CurrentLoc).GetSafeNormal();

			FRotator NewRot = Direction.Rotation();
			SetActorRotation(NewRot);
		}
	}

	//매 프레임 스킬 쿨타임 줄이기
	for (int i = 0; i < Skills.Num(); i++)
	{
		if (Skills[i].CurrentCooldown > 0.0f)
		{
			Skills[i].CurrentCooldown -= DeltaTime;
			if (Skills[i].CurrentCooldown < 0.0f)
			{
				Skills[i].CurrentCooldown = 0.0f;
				// UI 갱신 등 알림 필요하면 여기서
			}
		}
	}

}

// Called to bind functionality to input
void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (enhancedInputComponent != nullptr) {
		onInputBindingDelegate.Broadcast(enhancedInputComponent); // Broadcast the delegate to all bound functions
		enhancedInputComponent->BindAction(ia_Skill1, ETriggerEvent::Started, this, &ATPSCharacter::Input_Skill1);
		enhancedInputComponent->BindAction(ia_Skill2, ETriggerEvent::Started, this, &ATPSCharacter::Input_Skill2);
		enhancedInputComponent->BindAction(ia_Skill3, ETriggerEvent::Started, this, &ATPSCharacter::Input_Skill3);
		enhancedInputComponent->BindAction(ia_Skill4, ETriggerEvent::Started, this, &ATPSCharacter::Input_Skill4);
	}

}

void ATPSCharacter::OnHitEvent(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap Detected with: %s"), *OtherActor->GetName());
	if (OtherActor == this || OtherActor == nullptr) return;

	AEnemy* Enemy = Cast<AEnemy>(OtherActor); // 부딪힌 게 적인지 확인

	if (Enemy)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();

		UE_LOG(LogTemp, Warning, TEXT("Enemy Hit! Time: %f, Last: %f, Diff: %f"), CurrentTime, LastDamageTime, (CurrentTime - LastDamageTime));

		if (CurrentTime - LastDamageTime > DamageDelay)
		{
			UE_LOG(LogTemp, Error, TEXT("DAMAGE APPLIED!"));

			UEnemyFSM* EnemyFSM = Enemy->fsm;
			if (EnemyFSM)
			{
				// 데미지 입힘
				DecreaseHP(EnemyFSM->EnemyAttackPower);

				// ★ 로그 출력 (확인용)
				UE_LOG(LogTemp, Warning, TEXT("Ouch! Hit by Block! HP: %f"), hp);

				// ★ 마지막 맞은 시간 갱신 (이제 1초 동안 안 맞음)
				LastDamageTime = CurrentTime;
			}
		}
	}
}

void ATPSCharacter::OnGameOver_Implementation()
{
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

//체력 회복
void ATPSCharacter::Heal(float Amount)
{
	hp += Amount;

	if (hp > initialHP) // 최대 체력(initialHP)을 넘을 수 없음
	{
		hp = initialHP;
	}

	if (OnHPChanged.IsBound()) 
	{
		OnHPChanged.Broadcast(hp, initialHP);
	}

}

// 최대 체력 증가
void ATPSCharacter::UpgradeMaxHP(float Amount)
{
	initialHP += Amount; // 최대통 늘리기
	hp += Amount;        // 늘어난 만큼 현재 체력도 회복

	UE_LOG(LogTemp, Warning, TEXT("[C++] MaxHP Up! HP: %f / Max: %f"), hp, initialHP);

	if (OnHPChanged.IsBound()) 
	{
		OnHPChanged.Broadcast(hp, initialHP);
	}
}

//공격력 증가
void ATPSCharacter::UpgradeAttackPower(float Amount)
{
	AttackPower += Amount;
}

//공격 속도 증가 (제한 있음)
void ATPSCharacter::UpgradeAttackSpeed(float Amount)
{
	AttackSpeed -= Amount; // 딜레이 감소 = 속도 증가

	if (AttackSpeed < MIN_ATTACK_DELAY) // 0.1초보다 더 빨라지지 않게 제한
	{
		AttackSpeed = MIN_ATTACK_DELAY;
	}
}

//총알 개수 증가 (제한 있음)
void ATPSCharacter::UpgradeProjectileCount(int32 Amount)
{
	ProjectileCount += Amount;

	if (ProjectileCount > MAX_PROJECTILE_COUNT) // 5발 넘어가면 고정
	{
		ProjectileCount = MAX_PROJECTILE_COUNT;
	}
}

void ATPSCharacter::DecreaseHP(float DamageAmount)
{
	float PreHP = hp;

	if (bIsShieldActive)
	{
		// (선택) "BLOCK!" 같은 로그나 이펙트 띄워주면 좋음
		UE_LOG(LogTemp, Warning, TEXT("Shield Blocked Damage!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Damaged!!"));
	hp -= DamageAmount;

	if (hp <= 0)
	{
		hp = 0;
		UE_LOG(LogTemp, Warning, TEXT("Player is Dead!"));
		OnGameOver();
	}

	UE_LOG(LogTemp, Warning, TEXT("[C++] Hit! Damage: %f | HP: %f -> %f"), DamageAmount, PreHP, hp);

	if (OnHPChanged.IsBound()) 
	{
		OnHPChanged.Broadcast(hp, initialHP);
	}
}

FString ATPSCharacter::GetPlayTimeText()
{
	float TimeSeconds = GetWorld()->GetTimeSeconds(); //게임 시작 후 경과 시간(초)

	int32 Minutes = (int32)(TimeSeconds / 60.0f);
	int32 Seconds = (int32)((int32)TimeSeconds % 60);

	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds); //문자열 포맷팅 (00:00 형태)
}

void ATPSCharacter::UnlockSkill(int32 Index)
{
	if (Skills.IsValidIndex(Index))
	{
		if (Skills[Index].bUnlocked) return; // 이미 풀려있으면 패스

		Skills[Index].bUnlocked = true; //스킬 해금
		OnUnlockSkill(Index);

		// 로그 및 알림
		UE_LOG(LogTemp, Warning, TEXT("🎉 SKILL %d UNLOCKED! Press Key %d!"), Index + 1, Index + 1);

		// (나중에 여기에 "SKILL UNLOCKED" UI 띄우는 코드나 사운드 재생 추가)
	}
}

void ATPSCharacter::Input_Skill1() 
{ 
	ActivateSkill(0); 
}

void ATPSCharacter::Input_Skill2() 
{ 
	ActivateSkill(1); 
}

void ATPSCharacter::Input_Skill3() 
{ 
	ActivateSkill(2); 
}

void ATPSCharacter::Input_Skill4() 
{
	ActivateSkill(3); 
}

void ATPSCharacter::ActivateSkill(int32 Index)
{
	if (!Skills.IsValidIndex(Index)) return; // 인덱스 범위 벗어나면 무시

	if (Skills[Index].bUnlocked == false) return; //해금 안 됐으면 무시

	if (Skills[Index].CurrentCooldown > 0.0f) return; //쿨타임 중이면 무시

	Skills[Index].CurrentCooldown = Skills[Index].MaxCooldown; // 쿨타임 시작

	UE_LOG(LogTemp, Warning, TEXT("Skill %d Activated!"), Index + 1);

	//각 스킬별 효과 적용
	switch (Index)
	{
	case 0: // 이속 증가
		GetCharacterMovement()->MaxWalkSpeed = 500.0f; //고정 500
		GetWorldTimerManager().SetTimer(Timer_Skill1, this, &ATPSCharacter::EndSkill1, Skills[0].Duration, false);
		break;

	case 1: // 실드 (무적)
		bIsShieldActive = true;
		GetWorldTimerManager().SetTimer(Timer_Skill2, this, &ATPSCharacter::EndSkill2, Skills[1].Duration, false);
		break;

	case 2: // 공격력 증가
		DamageMultiplier = 2.0f; // 데미지 2배
		GetWorldTimerManager().SetTimer(Timer_Skill3, this, &ATPSCharacter::EndSkill3, Skills[2].Duration, false);
		break;

	case 3: // 공속 증가
		FireDelayMultiplier = 0.5f; // 딜레이 절반 = 공속 2배
		GetWorldTimerManager().SetTimer(Timer_Skill4, this, &ATPSCharacter::EndSkill4, Skills[3].Duration, false);
		break;
	}
}

//지속시간 종료 복구 함수
void ATPSCharacter::EndSkill1()
{
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
	UE_LOG(LogTemp, Warning, TEXT("Skill 1 Ended"));
}

void ATPSCharacter::EndSkill2()
{
	bIsShieldActive = false;
	UE_LOG(LogTemp, Warning, TEXT("Skill 2 Ended"));
}

void ATPSCharacter::EndSkill3()
{
	DamageMultiplier = 1.0f;
	UE_LOG(LogTemp, Warning, TEXT("Skill 3 Ended"));
}

void ATPSCharacter::EndSkill4()
{
	FireDelayMultiplier = 1.0f;
	UE_LOG(LogTemp, Warning, TEXT("Skill 4 Ended"));
}

float ATPSCharacter::GetTotalAttackPower() const
{
	return AttackPower * DamageMultiplier; // 기본 공격력 * 스킬 배율
}

float ATPSCharacter::GetTotalAttackSpeed() const
{
	return AttackSpeed * FireDelayMultiplier; // 기본 속도 * 스킬 배율
}

float ATPSCharacter::GetSkillCooldownRatio(int32 SkillIndex) const
{
	if (!Skills.IsValidIndex(SkillIndex)) return 0.0f;

	if (Skills[SkillIndex].MaxCooldown <= 0.0f) return 0.0f; // 최대 쿨타임이 0이면 에러 방지로 0 리턴

	return Skills[SkillIndex].CurrentCooldown / Skills[SkillIndex].MaxCooldown; // 남은 시간 / 전체 시간
}





