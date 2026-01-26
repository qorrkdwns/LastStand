#include "PlayerFire.h"
#include "TPSCharacter.h" 
#include "Bullet.h"
#include "Weapon.h"
#include "Kismet/GameplayStatics.h"

UPlayerFire::UPlayerFire()
{
	// Tick 안 쓰면 false 추천
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerFire::BeginPlay()
{
	Super::BeginPlay();

	// me 캐스팅
	me = Cast<ATPSCharacter>(GetOwner());

	if (me) {
		EquipWeapon(); //무기 장착

		bIsFireActive = false;
	}
}

void UPlayerFire::EquipWeapon()
{
	if (!me || !DefaultWeaponClass) return;
	if (CurrentWeapon) return; // 이미 무기가 있으면 패스

	FActorSpawnParameters Params; // 무기 스폰 파라미터
	Params.Owner = me;
	Params.Instigator = me;

	CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, me->GetActorLocation(), me->GetActorRotation(), Params); //무기 생성 (위치는 캐릭터 위치)

	if (CurrentWeapon)
	{
		FName SocketName = TEXT("hand_rSocket");
		CurrentWeapon->AttachToComponent(me->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	}
}

void UPlayerFire::AutoFire()
{
	if(bIsFireActive == false) return;

	if (!me || !CurrentWeapon || !bulletFactory) return;

	if (bulletSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), bulletSound); //사운드 재생
	}

	float FinalAttackSpeed = me->AttackSpeed * me->FireDelayMultiplier; //[공속 스킬 적용] 최종 공격 속도 계산 (기본 속도 * 배율)

	float CurrentTimerRate = me->GetWorldTimerManager().GetTimerRate(FireTimerHandle); // 현재 타이머 속도 가져오기(공격 속도)

	if (FMath::IsNearlyEqual(CurrentTimerRate, FinalAttackSpeed) == false)
	{
		me->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UPlayerFire::AutoFire, FinalAttackSpeed, true); // 새로운 공격 속도로 타이머 다시 시작
	}


	int32 Count = me->ProjectileCount; // 현재 총알 개수 (1~5)
	float Spacing = 40.0f;             // 총알 사이 간격

	float StartOffset = -((Count - 1) * Spacing) / 2.0f; // 시작점 계산 (총알들을 캐릭터 가운데 정렬하기 위함)

	FVector MuzzleLoc = CurrentWeapon->GetMuzzleLocation(); // 무기 총구 위치 가져오기
	FRotator FireRot = me->GetActorRotation(); //캐릭터 회전값 가져오기

	for (int32 i = 0; i < Count; i++) // 총알 개수만큼 반복 발사
	{
		FVector FireLocation = MuzzleLoc + (me->GetActorRightVector() * (StartOffset + (i * Spacing))); //// 총구 위치에서 오른쪽(RightVector)으로 살짝 이동한 위치 계산

		FActorSpawnParameters SpawnParams; // 스폰 파라미터
		SpawnParams.Owner = me;
		SpawnParams.Instigator = me;

		ABullet* SpawnedBullet = GetWorld()->SpawnActor<ABullet>(bulletFactory, FireLocation, FireRot, SpawnParams); // 총알 생성

		if (SpawnedBullet) // 데미지 주입 (캐릭터 공격력 적용)
		{
			SpawnedBullet->Damage = me->AttackPower * me->DamageMultiplier; //[공격력 스킬 적용] (기본 공격력 * 배율)
		}
	}
}

void UPlayerFire::StartFiring()
{
	if (bIsFireActive) return; //이미 발사 중이면 패스
	bIsFireActive = true;

	if (me)
	{
		me->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UPlayerFire::AutoFire, fireRate, true);
	}
}