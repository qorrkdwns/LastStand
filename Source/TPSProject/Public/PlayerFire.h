#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerFire.generated.h"

class ATPSCharacter;
class AWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TPSPROJECT_API UPlayerFire : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerFire();

protected:
	virtual void BeginPlay() override;

public:

	void AutoFire(); //자동 발사 함수

	void EquipWeapon(); //무기 장착 함수

public:

	UFUNCTION(BlueprintCallable)
	void StartFiring();

	bool bIsFireActive = false;

	UPROPERTY()
	class ATPSCharacter* me;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon") // 에디터에서 BP_Rifle 등을 넣을 변수
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon") // 실제로 스폰된 무기를 저장할 변수
	AWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	TSubclassOf<class ABullet> bulletFactory;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	class USoundBase* bulletSound;

	FTimerHandle FireTimerHandle; //자동 발사 타이머

	UPROPERTY(EditAnywhere, Category = "GunSettings")
	float fireRate = 0.5f;
};