#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tower.generated.h"

UENUM(BlueprintType)
enum class ETowerState : uint8 { Neutral, Captured, Contested };

UENUM(BlueprintType)
enum class ETowerOwner : uint8 { None, HumanPlayer, AIPlayer };

UCLASS()
class PAA_VIDEOGAME2026_API ATower : public AActor
{
	GENERATED_BODY()

public:
	ATower();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	UStaticMeshComponent* TowerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	FVector2D GridPosition;

	UPROPERTY(VisibleAnywhere, Category = "Tower State")
	ETowerState CurrentState = ETowerState::Neutral;

	UPROPERTY(VisibleAnywhere, Category = "Tower State")
	ETowerOwner CurrentOwner = ETowerOwner::None;

	UPROPERTY(EditAnywhere, Category = "Setup | Aesthetics")
	class UMaterialInterface* BaseTowerMaterial;

	void SetupTower(int32 X, int32 Y);
	void UpdateVisuals(FLinearColor Color);
};