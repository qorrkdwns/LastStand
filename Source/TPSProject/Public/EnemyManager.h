// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyManager.generated.h"

UCLASS()
class TPSPROJECT_API AEnemyManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameRule")
	int32 MaxBossCount = 5;

	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float minTime = 1;

	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	float maxTime = 5;

	UPROPERTY(EditAnywhere, Category = "BossSettings")
	TSubclassOf<class AEnemy> MidBossClass; // 중간 보스 BP

	UPROPERTY(EditAnywhere, Category = "BossSettings")
	float MidBossSpawnInterval = 30.0f;

	UPROPERTY(EditAnywhere, Category = "BossSettings")
	TSubclassOf<class AEnemy> FinalBossClass; // 최종 보스 BP

	UPROPERTY(EditAnywhere, Category = "BossSettings")
	float FinalBossSpawnInterval = 60.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossStatus")
	int32 BossKillCount = 0;

	float NextMidBossSpawnTime = 0.0f;
	float NextFinalBossSpawnTime = 0.0f;

	bool bMidBossAlive = false;
	bool bFinalBossAlive = false;

	UPROPERTY(EditAnywhere, Category = SpawnSettings)
	TSubclassOf<class AEnemy> enemyFactory;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float SpawnRadius = 1500.0f;

	FTimerHandle spawnTimer;

	UFUNCTION(BlueprintCallable)
	void StartSpawning();

	void CreateEnemy();
	void OnBossDied(int32 BossType);

	FVector GetRandomSpawnLocation(AActor* Player);
};
