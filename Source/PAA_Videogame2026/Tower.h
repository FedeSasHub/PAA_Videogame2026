#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tower.generated.h"

// 1. Definiamo gli stati della Torre
UENUM(BlueprintType)
enum class ETowerState : uint8 { Neutral, Captured, Contested };

// 2. Definiamo a chi appartiene la Torre
UENUM(BlueprintType)
enum class ETowerOwner : uint8 { None, HumanPlayer, AIPlayer };

UCLASS()
class PAA_VIDEOGAME2026_API ATower : public AActor
{
	GENERATED_BODY()

public:
	ATower();

	// Il modello 3D della Torre
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	UStaticMeshComponent* TowerMesh;

	// Coordinate in cui e' piazzata
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	FVector2D GridPosition;

	// --- STATO DELLA TORRE ---
	UPROPERTY(VisibleAnywhere, Category = "Tower State")
	ETowerState CurrentState = ETowerState::Neutral;

	UPROPERTY(VisibleAnywhere, Category = "Tower State")
	ETowerOwner CurrentOwner = ETowerOwner::None;

	// Materiale base da usare per colorarla dinamicamente
	UPROPERTY(EditAnywhere, Category = "Setup | Aesthetics")
	class UMaterialInterface* BaseTowerMaterial;

	void SetupTower(int32 X, int32 Y);
	void UpdateVisuals(FLinearColor Color);
};