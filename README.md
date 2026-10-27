# 🧟 Last Stand: TPS Zombie Survival (Unreal Engine 5.6.1)

> **"Unreal Engine 5와 C++을 활용한 3인칭 슈팅 디펜스 게임"**<br>
> 플레이어가 좀비 웨이브를 막아내며 아이템을 통해 성장하고, 최종적으로 보스 몬스터를 처치하는 서바이벌 게임. AI 로직, 전투 시스템, 스킬 해금 구조를 C++ 클래스로 설계 및 구현.

<br>

## ⚠️ Repository Info
> **[알림]** 이 리포지토리는 프로젝트의 **핵심 C++ 소스 코드(`Source/`)**와 **설정 파일**을 포함하고 있음.<br>
> (저작권이 있는 유료 에셋 및 대용량 `Content` 폴더는 용량 제한으로 인해 제외하였음.)

<br>

## 📅 프로젝트 개요
* **개발 기간**: 2025년 2학기
* **개발 인원**: 1명 (개인 프로젝트)
* **개발 환경**: Unreal Engine 5.6.1
* **사용 언어**: `C++`
* **장르**: TPS (Third-Person Shooter) / Survival

<br>

## 🎮 플레이 시연 (Gameplay Demo)

### ⚔️ 메인 전투 & 플레이 (Gameplay)
FSM 기반의 좀비 AI와 TPS 슈팅 메커니즘이 적용된 전체 플레이 영상.
(아래 이미지를 클릭하면 유튜브 영상으로 이동.)

[![Last Stand Gameplay](http://img.youtube.com/vi/-crFOM1Vho0/0.jpg)](https://youtu.be/-crFOM1Vho0)

> **[영상 설명]** 좀비 웨이브 방어, 보스전, 총기 사격 및 스킬 시스템 시연

<br>

### 📸 주요 기능 구현 (Feature Highlights)

| 스킬 쿨타임 UI (Progress Bar) | 카메라 전환 시스템 (Camera View) |
| :---: | :---: |
| <img src="https://github.com/user-attachments/assets/f3a78931-4c2b-4064-ae89-9fe1eab87246" width="100%"> | <img src="https://github.com/user-attachments/assets/cec581d9-4e01-4fb1-ae42-2d81e5abe846" width="100%"> |
> **Left**: 스킬 사용 시 `GetWorldTimerManager`와 연동된 쿨타임 시각화<br>**Right**: `SetViewTargetWithBlend`를 활용한 역동적인 시점 전환

<br>

## 🚀 핵심 구현 기술 (Key Implementations)

보고서에 명시된 핵심 로직을 기반으로 C++로 구현했습니다.

### 1. 자동 사격 및 공격 속도 제어 (Auto Fire System)
* **구현 방식**: `UGameplayStatics::PlaySound2D`로 사운드를 재생하고, 공격 속도 스탯(`FinalAttackSpeed`)에 따라 타이머를 동적으로 제어.
* **기술적 디테일**:
    * `GetWorldTimerManager().SetTimer`를 사용하여 사격 간격을 조절.
    * 아이템 획득으로 공격 속도가 변할 경우, 기존 타이머를 갱신하여 즉시 반영되도록 로직 설계.

### 2. 마우스 커서 기반 회전 (Precise Aiming)
* **구현 방식**: TPS 시점에서 마우스 커서가 가리키는 3D 월드 좌표를 계산하여 캐릭터를 회전시킴.
* **기술적 디테일**:
    * `GetHitResultUnderCursor(ECC_Visibility, true, HitResult)`를 사용하여 2D 마우스 좌표를 3D 공간 좌표로 변환(Raycasting).
    * 캐릭터의 현재 위치와 목표 지점(LookTarget)의 벡터 차이를 구하고, `SetActorRotation`으로 Z축 회전값만 적용하여 자연스러운 조준 구현.

### 3. 스킬 시스템 및 해금 (Skill Architecture)
* **구현 방식**: 보스 처치 시 특정 스킬이 해금(`Unlock`)되며, 스킬 인덱스에 따라 switch-case 문으로 서로 다른 효과 발동.
* **스킬 구성**:
    * **Speed**: `MaxWalkSpeed`를 500.0f로 고정하여 이동 속도 증가.
    * **Shield**: `bIsShieldActive` 플래그를 통해 무적 상태 부여.
    * **Power/FireRate**: 데미지 배수(`DamageMultiplier`) 및 발사 지연(`FireDelayMultiplier`) 조절.
* **해금 로직**: `OnBossDied` 델리게이트가 호출될 때 보스 타입(중간/최종)을 확인하고 `UnlockSkill()` 함수를 트리거.

### 4. NavMesh 기반 적 스폰 (AI Spawning)
* **구현 방식**: 플레이어 주변 일정 반경 내에서 랜덤 위치를 산출하되, 유효한 이동 가능 구역(NavMesh) 위에만 스폰.
* **기술적 디테일**:
    * `FMath::RandPointInCircle`로 1차 랜덤 좌표 산출.
    * `UNavigationSystemV1::ProjectPointToNavigation` 함수를 사용하여 산출된 좌표가 실제 AI가 이동 가능한 NavMesh 위인지 검증 후 스폰하여 끼임 현상 방지.

<br>

## 🛠️ 트러블 슈팅 (Troubleshooting)

### 📌 1. Blueprint Reparenting 이슈
* **문제**: C++ 부모 클래스를 수정한 뒤 핫 리로드(Hot Reload) 시, 이를 상속받은 블루프린트(BP)가 부모를 인식하지 못하는 연결 끊김 현상.
* **해결**: 불안정한 핫 리로드 대신 에디터를 종료 후 Visual Studio에서 **Clean Build**를 수행. 이후 블루프린트 에디터의 **File -> Reparent Blueprint** 기능을 통해 부모 클래스를 재연결하여 해결.

### 📌 2. AI 공격 거리 오차 (Invisible Wall Issue)
* **문제**: 설정한 공격 사거리와 실제 AI 이동 정지 거리의 차이로 인해, 좀비가 공격하지 않고 플레이어 앞에서 멈추는 현상 발생.
* **해결**: `MoveToActor` 함수의 파라미터 중 **`AcceptanceRadius` (허용 반경)** 값을 조정하고, FSM의 공격 상태 전이 조건(거리 계산 로직)을 이에 맞춰 수정함으로써 부드러운 공격 전환 구현.

<br>

## 🕹️ 조작법 (Controls)
* **이동**: `W`, `A`, `S`, `D`
* **공격/조준**: `Mouse Left` / `Mouse Right`
* **스킬 사용**: `1`(이속), `2`(무적), `3`(공격력), `4`(공속)

<br>

## 📂 프로젝트 파일 구조 (Structure)
* `Source/LastStand`: 주요 C++ 소스 코드
* `Config/`: 프로젝트 설정 파일

<br>

## 📄 상세 보고서 (Project Report)
AI 설계 로직(FSM), 트러블 슈팅 과정, 클래스 구조도가 포함된 상세 발표 자료입니다.
👉 **[프로젝트 발표 자료(PDF) 보러 가기](./LastStand_presentation.pdf)**

---
*Developed by Kangjun Baek*
