#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.generated.h"

UCLASS()
class PAA_VIDEOGAME2026_API AGridCell : public AActor
{
	GENERATED_BODY()

public:
	AGridCell();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	UStaticMeshComponent* CellMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grid State")
	bool bIsOccupied = false;

	// Livello di elevazione (da 0 a 4)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 ElevationLevel;

	// Coordinate nella griglia (X, Y)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FVector2D GridPosition;

	void SetupCell(int32 X, int32 Y, int32 Elevation);

	// --- FUNZIONI EVIDENZIAZIONE ---
	void HighlightCell(bool bEnable);        // Giallo (Movimento)
	void HighlightAttackCell(bool bHighlight); // Rosso (Attacco)
	void HighlightHybridCell(bool bHighlight); // Arancione (Entrambi)

	// --- MATERIALI ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid Appearance")
	TArray<class UMaterialInterface*> ElevationMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid Appearance")
	class UMaterialInterface* MoveHighlightMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid Appearance")
	class UMaterialInterface* AttackHighlightMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid Appearance")
	class UMaterialInterface* HybridHighlightMaterial;

	UFUNCTION()
	void OnCellClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

private:
	// Memorizza il materiale originale del terreno per il ripristino
	UPROPERTY()
	class UMaterialInterface* OriginalMaterial;
};