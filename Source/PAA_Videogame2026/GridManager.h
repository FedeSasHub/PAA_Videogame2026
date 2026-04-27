#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.h"
#include "Tower.h"
#include "GridManager.generated.h"

struct FAStarNode
{
	class AGridCell* Cell;
	FAStarNode* Parent;
	int32 GCost;
	int32 HCost;

	int32 FCost() const { return GCost + HCost; }

	FAStarNode(AGridCell* InCell) : Cell(InCell), Parent(nullptr), GCost(0), HCost(0) {}
};

UCLASS()
class PAA_VIDEOGAME2026_API AGridManager : public AActor
{
	GENERATED_BODY()

public:
	AGridManager();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, Category = "Grid Setup")
	TSubclassOf<AGridCell> CellClassToSpawn;

	UPROPERTY(EditAnywhere, Category = "Grid Setup")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Grid Setup")
	TSubclassOf<ATower> TowerClassToSpawn;

	UPROPERTY(EditAnywhere, Category = "Grid | Aesthetics")
	FColor CoordinateColor = FColor::White;

	UPROPERTY(EditAnywhere, Category = "Grid | Aesthetics")
	float CoordinateScale = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Grid | Aesthetics")
	class UFont* CoordinateFont;

	AGridCell* GridCells[25][25];

	void SpawnTowerAt(int32 GridX, int32 GridY);
	FVector2D GetNearestValidTowerPosition(int32 StartX, int32 StartY);

	TArray<AGridCell*> GetReachableCells(int32 StartX, int32 StartY, int32 MaxMovement);
	TArray<AGridCell*> GetAttackableCells(int32 StartX, int32 StartY, int32 AttackRange);

	int32 GetManhattanDistance(AGridCell* StartCell, AGridCell* TargetCell);
	TArray<AGridCell*> FindPath(AGridCell* StartCell, AGridCell* TargetCell);
};