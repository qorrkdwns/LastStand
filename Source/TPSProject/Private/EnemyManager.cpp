// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyManager.h"
#include "Enemy.h"
#include "TPSCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"

// Sets default values
AEnemyManager::AEnemyManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AEnemyManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AEnemyManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyManager::CreateEnemy()
{
	float createTime = FMath::FRandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnTimer, this, &AEnemyManager::CreateEnemy, createTime, false);

	float CurrentTime = GetWorld()->GetTimeSeconds();
	bool bSpawnedFinalBossNow = false;

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player == nullptr) {
		return;
	}

	//최종 보스
	if (CurrentTime >= NextFinalBossSpawnTime && !bFinalBossAlive)
	{
		if (FinalBossClass)
		{
			FVector SpawnLoc = GetRandomSpawnLocation(Player);
			//강제 스폰 옵션 (끼임 방지)
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AEnemy* Boss = GetWorld()->SpawnActor<AEnemy>(FinalBossClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
			if (Boss)
			{
				Boss->MyManager = this; 
				Boss->BossType = 2;     //최종보스

				//보스가 죽어야 다음 스폰 시간 갱신
				bFinalBossAlive = true;
				bSpawnedFinalBossNow = true;
			}

			UE_LOG(LogTemp, Warning, TEXT("!!! FINAL BOSS!!!"));
		}
	}

	//중간 보스
	if (CurrentTime >= NextMidBossSpawnTime && !bMidBossAlive)
	{
		if (!bSpawnedFinalBossNow)
		{
			if (MidBossClass)
			{
				FVector SpawnLoc = GetRandomSpawnLocation(Player);
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				AEnemy* Boss = GetWorld()->SpawnActor<AEnemy>(MidBossClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
				if (Boss)
				{
					Boss->MyManager = this;
					Boss->BossType = 1; //중간보스

					bMidBossAlive = true;
				}
				UE_LOG(LogTemp, Warning, TEXT("Mid Boss Spawned"));
			}

			//중간 보스가 죽어야 다음 스폰 시간 갱신
			
		}
		else
		{
			NextMidBossSpawnTime = CurrentTime + 5.0f; // 최종 보스 때문에 스킵된 경우, 5초 뒤에 다시 체크하게 미룸
		}
	}

	if (enemyFactory)
	{
		FVector SpawnLoc = GetRandomSpawnLocation(Player); // 위치가 아니라, 계산된 랜덤 위치를 가져옴

		float RandomX = FMath::RandRange(-100.0f, 100.0f);
		float RandomY = FMath::RandRange(-100.0f, 100.0f);

		SpawnLoc.X += RandomX;
		SpawnLoc.Y += RandomY;

		GetWorld()->SpawnActor<AEnemy>(enemyFactory, SpawnLoc, FRotator::ZeroRotator); //적 생성
	}
}

// 플레이어 주변의 랜덤한 위치 반환
FVector AEnemyManager::GetRandomSpawnLocation(AActor* Player)
{
	if (!Player) return GetActorLocation();

	FVector PlayerLoc = Player->GetActorLocation();
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	FVector FinalLoc = PlayerLoc;
	bool bFound = false;

	float MinDist = 600.0f; 
	float MaxDist = SpawnRadius;

	for (int i = 0; i < 10; i++) // 최대 10번 시도 (무한 루프 방지)
	{
		FVector2D RandomPoint = FMath::RandPointInCircle(1.0f); // 랜덤 방향 구하기
		RandomPoint.Normalize();

		float RandomDist = FMath::RandRange(MinDist, MaxDist);

		FVector SpawnOffset = FVector(RandomPoint.X, RandomPoint.Y, 0.0f) * RandomDist; // 거리 적용 (랜덤 위치 가상 계산)
		FVector RandomLoc = PlayerLoc + SpawnOffset;

		//NavMesh 위에 있는지 확인하고 투영하기
		FNavLocation NavLoc;

		bool bProjectSuccess = NavSystem && NavSystem->ProjectPointToNavigation(RandomLoc, NavLoc, FVector(200.0f, 200.0f, 500.0f));

		UE_LOG(LogTemp, Warning, TEXT("Try %d: RandomDist: %f | Project Success: %s"), i, RandomDist, bProjectSuccess ? TEXT("TRUE") : TEXT("FALSE"));

		if (NavSystem && NavSystem->ProjectPointToNavigation(RandomLoc, NavLoc, FVector(200.0f, 200.0f, 500.0f)))
		{
			FinalLoc = NavLoc.Location; //NavMesh 위의 안전한 좌표를 찾음
			bFound = true;
			break;
		}
	}

	if (!bFound) // 10번 다 실패했으면 그냥 플레이어 위치(혹은 매니저 위치) 반환해서 오류 방지
	{
		// UE_LOG(LogTemp, Warning, TEXT("Failed to find spawn point!"));
		return PlayerLoc;
	}

	return FinalLoc;
}

void AEnemyManager::OnBossDied(int32 BossType)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (BossType == 1) // 중간 보스 사망
	{
		bMidBossAlive = false;
		NextMidBossSpawnTime = CurrentTime + MidBossSpawnInterval; //죽은 시점으로부터 쿨타임

		UE_LOG(LogTemp, Warning, TEXT("Mid Boss Died! Next in %f sec"), MidBossSpawnInterval);
	}
	else if (BossType == 2) // 최종 보스 사망
	{
		bFinalBossAlive = false;
		NextFinalBossSpawnTime = CurrentTime + FinalBossSpawnInterval; //죽은 시점으로부터 쿨타임

		UE_LOG(LogTemp, Warning, TEXT("Final Boss Died! Next in %f sec"), FinalBossSpawnInterval);

		ATPSCharacter* Player = Cast<ATPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		if (Player)
		{
			// 현재 몇 번째 보스냐에 따라 해금할 스킬 결정
			int32 SkillIndexToUnlock = BossKillCount;

			// 스킬 해금 요청
			Player->UnlockSkill(SkillIndexToUnlock);
		}

		BossKillCount++; // 보스 처치 횟수 증가

		if (BossKillCount >= MaxBossCount) //게임 승리
		{
			if (Player)
			{
				Player->OnGameClear();

				UGameplayStatics::SetGamePaused(GetWorld(), true);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Final Boss Died! Kill Count: %d"), BossKillCount);

	}
}

void AEnemyManager::StartSpawning()
{
	float createTime = FMath::FRandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnTimer, this, &AEnemyManager::CreateEnemy, createTime, false); //false: 한번만 실행

	float CurrentTime = GetWorld()->GetTimeSeconds();
	NextMidBossSpawnTime = CurrentTime + MidBossSpawnInterval;
	NextFinalBossSpawnTime = CurrentTime + FinalBossSpawnInterval;
}

