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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* TowerMesh;

	UPROPERTY(EditAnywhere, Category = "Setup")
	UMaterialInterface* BaseTowerMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETowerState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETowerOwner CurrentOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D GridPosition;

	void SetupTower(int32 X, int32 Y);
	void UpdateVisuals(FLinearColor Color);

private:
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicTowerMat;
};