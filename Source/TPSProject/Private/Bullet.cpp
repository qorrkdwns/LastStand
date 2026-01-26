// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "Enemy.h"
#include "TPSCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	collisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	collisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	collisionComp->SetSphereRadius(13);
	RootComponent = collisionComp;

	bodyMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComp"));
	bodyMeshComp->SetupAttachment(collisionComp);
	bodyMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bodyMeshComp->SetRelativeScale3D(FVector(0.25f));

	movementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
	movementComp->SetUpdatedComponent(collisionComp);
	movementComp->InitialSpeed = 5000;
	movementComp->MaxSpeed = 5000;
	movementComp->bShouldBounce = false; // Enable bouncing
	movementComp->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 2.0f;

}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	// ★ 1. 충돌체(CollisionComponent) 설정 변경
	// (변수명이 CollisionComp 혹은 BoxComp, SphereComp 일 수 있음 확인 필요!)
	if (collisionComp)
	{

		// 이벤트 연결
		collisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnBulletOverlap);
	}

	// ★ 2. 총알 수명 (3초 뒤엔 무조건 삭제 - 렉 방지)
	SetLifeSpan(3.0f);
	
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABullet::OnBulletOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. 나 자신이나 내 주인(플레이어)과는 충돌하지 않음
	if (OtherActor == this || OtherActor == GetOwner()) return;

	AEnemy* HitEnemy = Cast<AEnemy>(OtherActor);

	// 2. 부딪힌 게 '적(Enemy)'인지 확인
	// (태그로 확인하거나, 캐스팅으로 확인)
	if (HitEnemy)
	{
		// 3. 데미지 주기 (10 데미지)
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

		// ★ [핵심] 관통 횟수 차감
		penetrationCount--;

		// 4. 관통력이 다 떨어졌으면 총알 삭제
		if (penetrationCount <= 0)
		{
			Destroy();
		}

		// (선택) 타격 이펙트나 소리는 여기서 재생
		// UGameplayStatics::SpawnEmitterAtLocation(...);
	}
	else
	{
		// 적이 아닌 벽(Wall) 같은 거에 닿으면 그냥 삭제? 
		// (벽도 뚫게 할지, 벽에는 막힐지 결정 필요. 일단은 벽 닿으면 삭제 추천)
		if (Cast<ATPSCharacter>(OtherActor)) // 플레이어만 아니면
		{
			Destroy();
		}
	}
}
