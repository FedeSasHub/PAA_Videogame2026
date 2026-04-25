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
	int32 GCost; // Costo per arrivare qui (1 in piano, 2 in salita)
	int32 HCost; // Costo stimato per arrivare al bersaglio

	// Il costo totale F è la somma di G e H
	int32 FCost() const { return GCost + HCost; }

	// Costruttore
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
	// Permette di selezionare la classe della cella direttamente da Unreal
	UPROPERTY(EditAnywhere, Category = "Grid Setup")
	TSubclassOf<AGridCell> CellClassToSpawn;

	// Dimensione standard di un cubo in Unreal (100 cm)
	UPROPERTY(EditAnywhere, Category = "Grid Setup")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Grid Setup")
	TSubclassOf<ATower> TowerClassToSpawn;

	// --- IMPOSTAZIONI COORDINATE VISIBILI IN UNREAL ---
	UPROPERTY(EditAnywhere, Category = "Grid | Aesthetics")
	FColor CoordinateColor = FColor::White;

	UPROPERTY(EditAnywhere, Category = "Grid | Aesthetics")
	float CoordinateScale = 2.5f;

	// Se vuoi usare un font specifico, trascinalo qui in Unreal
	UPROPERTY(EditAnywhere, Category = "Grid | Aesthetics")
	class UFont* CoordinateFont;
	// La nostra mappa di celle per sapere dove sta l'acqua
	AGridCell* GridCells[25][25];
	int32 GetManhattanDistance(AGridCell* StartCell, AGridCell* TargetCell);

	// Il vero algoritmo A* che restituisce l'array di celle da attraversare
	TArray<AGridCell*> FindPath(AGridCell* StartCell, AGridCell* TargetCell);
	// Funzione per trovare la prima cella non d'acqua vicina
	FVector2D GetNearestValidTowerPosition(int32 StartX, int32 StartY);
	void SpawnTowerAt(int32 GridX, int32 GridY);
	TArray<AGridCell*> GetAttackableCells(int32 StartX, int32 StartY, int32 AttackRange);
	TArray<AGridCell*> GetReachableCells(int32 StartX, int32 StartY, int32 MaxMovement);
};