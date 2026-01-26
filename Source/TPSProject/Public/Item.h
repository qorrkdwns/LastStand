// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

// 아이템 종류 정의
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Health      UMETA(DisplayName = "Health"),            // 체력 회복
	MaxHP       UMETA(DisplayName = "Max HP Up"),         // 최대 체력 증가
	Power       UMETA(DisplayName = "Power Up"),          // 공격력 증가
	Speed       UMETA(DisplayName = "Attack Speed Up"),   // 공격 속도 증가
	MultiShot   UMETA(DisplayName = "Multi Shot")         // 멀티샷(평행 발사)
};

UCLASS()
class TPSPROJECT_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Item")
	class USphereComponent* CollisionComp; 

	UPROPERTY(VisibleAnywhere, Category = "Item")
	class UStaticMeshComponent* MeshComp; 

	UPROPERTY(VisibleAnywhere, Category = "Item")
	class URotatingMovementComponent* RotationComp; // 회전 효과

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType = EItemType::Health; // 아이템 종류

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Value = 10.0f; // 증가량 (예: 공격력 2, 공속 0.05 등)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	class USoundBase* PickupSound;

	UFUNCTION()
	void OnItemOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
