// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "TPSCharacter.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	SetRootComponent(CollisionComp);
	CollisionComp->SetSphereRadius(50.0f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RotationComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotationComp"));
	RotationComp->RotationRate = FRotator(0, 90.0f, 0); // 1초에 Y축으로 90도 회전
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnItemOverlap);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItem::OnItemOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 부딪힌 게 플레이어인지 확인
	ATPSCharacter* Player = Cast<ATPSCharacter>(OtherActor);

	if (Player)
	{
		switch (ItemType) // 아이템 종류에 따라 플레이어 스탯 성장 함수 호출
		{
		case EItemType::Health:
			Player->Heal(Value); // Value만큼 회복
			break;

		case EItemType::MaxHP:
			Player->UpgradeMaxHP(Value); // Value만큼 최대체력 증가
			break;

		case EItemType::Power:
			Player->UpgradeAttackPower(Value); // Value만큼 공격력 증가
			break;

		case EItemType::Speed:
			Player->UpgradeAttackSpeed(Value); // Value만큼 딜레이 감소 (0.05 등)
			break;

		case EItemType::MultiShot:
			Player->UpgradeProjectileCount((int32)Value); // Value(1.0)만큼 총알 수 증가
			break;
		}

		// (선택) 여기서 "띠링!" 하는 효과음이나 파티클 재생하면 좋음
		// UGameplayStatics::PlaySoundAtLocation(...)

		if (PickupSound)
		{
			// 2. PlaySoundAtLocation(월드, 소리파일, 위치)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
		}

		Destroy();
	}
}

