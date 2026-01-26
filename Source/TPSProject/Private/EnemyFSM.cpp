// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include "TPSCharacter.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "TPSProject.h"
#include "Components/CapsuleComponent.h"
#include "EnemyAnim.h"
#include "EnemyManager.h"
#include "Item.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

// Sets default values for this component's properties
UEnemyFSM::UEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), ATPSCharacter::StaticClass());
	target = Cast<ATPSCharacter>(actor);
	me = Cast<AEnemy>(GetOwner());

	anim = Cast<UEnemyAnim>(me->GetMesh()->GetAnimInstance());

	ai = Cast<AAIController>(me->GetController());
	
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//FString logMsg = UEnum::GetValueAsString(mState);
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, logMsg);
	
	switch (mState) {
	case EEnemyState::Idle:
		IdleState();
		break;
	case EEnemyState::Move:
		MoveState();
		break;
	case EEnemyState::Attack:
		AttackState();
		break;
	case EEnemyState::Die:
		DieState();
		break;
	}
}

void UEnemyFSM::IdleState()
{
	if (target == nullptr)
	{
		return;
	}

	mState = EEnemyState::Move;
	anim->animState = mState;
}

void UEnemyFSM::MoveState()
{
	FVector destination = target->GetActorLocation();
	FVector direction = destination - me->GetActorLocation();

	ai->MoveToActor(target, 10.0f); //ai가 타겟을 향해 이동하도록 지시

	if(direction.Size() < attackRange)
	{
		ai->StopMovement();

		mState = EEnemyState::Attack;
		anim->animState = mState;
		currentTime = attackDelayTime; //공격 상태로 전환 되자마자 바로 공격하도록 설정
	}
}

void UEnemyFSM::AttackState()
{
	currentTime += GetWorld()->GetDeltaSeconds();

	//애니메이션 재생 중이라면 리턴
	if (EnemyAttackMontage && me->GetMesh()->GetAnimInstance()->Montage_IsPlaying(EnemyAttackMontage))
	{

		return;
	}

	// 쿨타임 안 찼으면 리턴 (공통)
	if (currentTime < attackDelayTime)
	{
		return;
	}

	// 거리 체크 및 상태 전환 (공통)
	float distance = FVector::Distance(target->GetActorLocation(), me->GetActorLocation());

	if (distance > attackRange)
	{
		mState = EEnemyState::Move;
		anim->animState = mState;
		GetRandomPositionInNavMesh(me->GetActorLocation(), 500, randomPos);
		return;
	}

	// ★ [수정] 공격 실행 분기
	currentTime = 0; // 쿨타임 초기화

	if (EnemyAttackMontage)
	{
		if (!me->GetMesh()->GetAnimInstance()->Montage_IsPlaying(EnemyAttackMontage))
		{
			// 랜덤 섹션 재생 로직
			int32 RandVal = FMath::RandRange(1, 2);
			FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), RandVal));
			me->PlayAnimMontage(EnemyAttackMontage, 1.0f, SectionName);
		}
	}
	else 
	{
		anim->bAttackPlay = true;
	}
}


void UEnemyFSM::DieState()
{
	if (anim->bDieDone == false) {
		return;
	}

	FVector P0 = me->GetActorLocation();
	FVector vt = FVector::DownVector * dieSpeed * GetWorld()->DeltaTimeSeconds;
	FVector P = P0 + vt;
	me->SetActorLocation(P);

	if (P.Z < -200.0f) {
		me->Destroy();
	}
}

void UEnemyFSM::OnDamageProcess(float damage)
{
	// 1. 체력 감소
	hp -= damage;

	// 2. 죽음 판정
	if (hp <= 0)
	{
		mState = EEnemyState::Die;
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ai->StopMovement();

		if (DieSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), DieSound, me->GetActorLocation());
		}

		// ★ [수정] 호환성 유지 로직 (몽타주 vs 레거시)
		if (EnemyDeathMontage) // 1. 몽타주가 있으면 (신형/보스)
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> Montage Found! Playing Montage..."));
			me->PlayAnimMontage(EnemyDeathMontage);
		}
		

		if (me->MyManager && me->BossType > 0) // 보스일 때만 보고
		{
			me->MyManager->OnBossDied(me->BossType);
		}

		if (FMath::RandRange(0.0f, 1.0f) <= DropChance)
		{
			if (DropItems.Num() > 0) // 드랍 아이템이 하나라도 있을 때
			{
				int32 RandIndex = FMath::RandRange(0, DropItems.Num() - 1);
				TSubclassOf<AItem> ItemToSpawn = DropItems[RandIndex];

				if (ItemToSpawn)
				{
					UCapsuleComponent* Cap = me->GetCapsuleComponent();
					float FloorZ = me->GetActorLocation().Z - Cap->GetScaledCapsuleHalfHeight() + 10.0f;
					FVector DropPos = FVector(me->GetActorLocation().X, me->GetActorLocation().Y, FloorZ);
					GetWorld()->SpawnActor<AItem>(ItemToSpawn, DropPos, FRotator::ZeroRotator);
				}
			}
		}
	}
}

bool UEnemyFSM::GetRandomPositionInNavMesh(FVector centerLocation, float radius, FVector& dest)
{
	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	FNavLocation loc;
	bool result = ns->GetRandomReachablePointInRadius(centerLocation, radius, loc);
	dest = loc.Location;
	return result;
}

void UEnemyFSM::AttackCheck()
{
	if (target == nullptr) return;

	
	float AttackRadius = attackRange + 30.0f; // 사거리 계산 (멈추는 거리 + 주먹 길이 여유분)

	float Distance = FVector::Dist(me->GetActorLocation(), target->GetActorLocation()); //거리 체크 (내 위치와 플레이어 위치 사이의 거리)

	
	if (Distance <= AttackRadius) //사거리 안에 들어왔다면 데미지
	{
		UE_LOG(LogTemp, Warning, TEXT("펀치 적중! 데미지: %f"), EnemyAttackPower);

		// 플레이어의 함수를 직접 호출해서 데미지 줌
		target->DecreaseHP(EnemyAttackPower);
	}
}

