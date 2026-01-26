// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AWeapon::GetMuzzleLocation() const
{
	if (MeshComp && MeshComp->DoesSocketExist(MuzzleSocketName))
	{
		return MeshComp->GetSocketLocation(MuzzleSocketName);
	}
	
	return GetActorLocation(); // 소켓 없으면 그냥 무기 위치 반환
}

