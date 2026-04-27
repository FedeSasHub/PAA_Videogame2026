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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 ElevationLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FVector2D GridPosition;

	void SetupCell(int32 X, int32 Y, int32 Elevation);

	void HighlightCell(bool bEnable);
	void HighlightAttackCell(bool bHighlight);
	void HighlightHybridCell(bool bHighlight);

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
	UPROPERTY()
	class UMaterialInterface* OriginalMaterial;
};