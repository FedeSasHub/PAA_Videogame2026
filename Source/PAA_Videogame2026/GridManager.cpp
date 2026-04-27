#include "GridManager.h"
#include "Algo/Reverse.h"
#include "Math/UnrealMathUtility.h"
#include "Components/TextRenderComponent.h"

// inizializza
AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;
}

// genera la griglia piazza le torri e aggiunge i testi delle coordinate ai bordi
void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	if (!CellClassToSpawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Missing CellClassToSpawn in GridManager!"));
		return;
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	float SeedOffset = FMath::RandRange(-10000.0f, 10000.0f);

	TArray<float> RawNoiseValues;
	RawNoiseValues.SetNum(25 * 25);
	float MinNoise = 10000.0f;
	float MaxNoise = -10000.0f;

	for (int32 x = 0; x < 25; x++)
	{
		for (int32 y = 0; y < 25; y++)
		{
			float NoiseValue = FMath::PerlinNoise2D(FVector2D(x * 0.1f + SeedOffset, y * 0.1f + SeedOffset));
			RawNoiseValues[x * 25 + y] = NoiseValue;

			if (NoiseValue < MinNoise) MinNoise = NoiseValue;
			if (NoiseValue > MaxNoise) MaxNoise = NoiseValue;
		}
	}

	for (int32 x = 0; x < 25; x++)
	{
		for (int32 y = 0; y < 25; y++)
		{
			float RawNoise = RawNoiseValues[x * 25 + y];
			float NormalizedNoise = (RawNoise - MinNoise) / (MaxNoise - MinNoise);
			int32 Elevation = FMath::RoundToInt(NormalizedNoise * 4.0f);

			FVector SpawnLocation(y * CellSize, x * CellSize, Elevation * (CellSize * 0.5f));
			FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

			AGridCell* SpawnedCell = GetWorld()->SpawnActor<AGridCell>(CellClassToSpawn, SpawnLocation, SpawnRotation);

			if (SpawnedCell)
			{
				SpawnedCell->SetupCell(x, y, Elevation);
				GridCells[x][y] = SpawnedCell;
			}
		}
	}

	if (TowerClassToSpawn)
	{
		SpawnTowerAt(12, 12);
		SpawnTowerAt(5, 12);
		SpawnTowerAt(19, 12);
	}

	for (int32 i = 0; i < 25; i++)
	{
		FVector NumLoc(-120.0f, i * CellSize, 50.0f);
		UTextRenderComponent* NumText = NewObject<UTextRenderComponent>(this);
		if (NumText)
		{
			NumText->RegisterComponent();
			NumText->SetWorldLocation(NumLoc);
			NumText->SetWorldRotation(FRotator(90.0f, 180.0f, 0.0f));
			NumText->SetText(FText::AsNumber(i));
			NumText->SetHorizontalAlignment(EHTA_Center);
			NumText->SetTextRenderColor(CoordinateColor);
			NumText->SetWorldScale3D(FVector(CoordinateScale));
			if (CoordinateFont) NumText->SetFont(CoordinateFont);
		}

		FVector LetLoc((i * CellSize) - (CellSize * 0.4f), -120.0f, 10.0f);
		UTextRenderComponent* LetText = NewObject<UTextRenderComponent>(this);
		if (LetText)
		{
			LetText->RegisterComponent();
			LetText->SetWorldLocation(LetLoc);
			LetText->SetWorldRotation(FRotator(90.0f, 180.0f, 0.0f));
			TCHAR Letter = 'A' + i;
			LetText->SetText(FText::FromString(FString::Printf(TEXT("%c"), Letter)));
			LetText->SetHorizontalAlignment(EHTA_Center);
			LetText->SetTextRenderColor(CoordinateColor);
			LetText->SetWorldScale3D(FVector(CoordinateScale));
			if (CoordinateFont) LetText->SetFont(CoordinateFont);
		}
	}
}

// calcola e genera un oggetto torre nella cella indicata
void AGridManager::SpawnTowerAt(int32 GridX, int32 GridY)
{
	float ZPos = GridCells[GridX][GridY]->ElevationLevel * (CellSize * 0.5f);
	FVector SpawnLoc(GridY * CellSize, GridX * CellSize, ZPos + 50.0f);

	ATower* NewTower = GetWorld()->SpawnActor<ATower>(TowerClassToSpawn, SpawnLoc, FRotator::ZeroRotator);
	if (NewTower)
	{
		NewTower->SetupTower(GridX, GridY);
		GridCells[GridX][GridY]->bIsOccupied = true;
	}
}

// effettua una ricerca a spirale per trovare la prima cella terrestre vicina in caso di spawn della torre in acqua
FVector2D AGridManager::GetNearestValidTowerPosition(int32 StartX, int32 StartY)
{
	int32 Radius = 0;
	int32 MaxRadius = 25;

	while (Radius < MaxRadius)
	{
		for (int32 x = StartX - Radius; x <= StartX + Radius; x++)
		{
			for (int32 y = StartY - Radius; y <= StartY + Radius; y++)
			{
				if (x < 0 || x >= 25 || y < 0 || y >= 25) continue;

				if (FMath::Abs(x - StartX) == Radius || FMath::Abs(y - StartY) == Radius)
				{
					if (GridCells[x][y] && GridCells[x][y]->ElevationLevel > 0)
					{
						return FVector2D(x, y);
					}
				}
			}
		}
		Radius++;
	}
	return FVector2D(StartX, StartY);
}

// controlla il costo in base all'elevazione per restituire l'array delle celle in cui la pedina puo camminare
TArray<AGridCell*> AGridManager::GetReachableCells(int32 StartX, int32 StartY, int32 MaxMovement)
{
	TArray<AGridCell*> ReachableCells;
	int32 CostSoFar[25][25];
	for (int32 i = 0; i < 25; i++)
	{
		for (int32 j = 0; j < 25; j++) CostSoFar[i][j] = 9999;
	}

	TArray<FIntPoint> Frontier;
	CostSoFar[StartX][StartY] = 0;
	Frontier.Add(FIntPoint(StartX, StartY));

	FIntPoint Directions[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

	while (Frontier.Num() > 0)
	{
		FIntPoint Current = Frontier[0];
		Frontier.RemoveAt(0);

		AGridCell* CurrentCell = GridCells[Current.X][Current.Y];

		for (const FIntPoint& Dir : Directions)
		{
			FIntPoint Next(Current.X + Dir.X, Current.Y + Dir.Y);

			if (Next.X < 0 || Next.X >= 25 || Next.Y < 0 || Next.Y >= 25) continue;

			AGridCell* NextCell = GridCells[Next.X][Next.Y];

			if (!NextCell || NextCell->ElevationLevel == 0 || NextCell->bIsOccupied) continue;

			int32 MoveCost = (NextCell->ElevationLevel > CurrentCell->ElevationLevel) ? 2 : 1;
			int32 NewCost = CostSoFar[Current.X][Current.Y] + MoveCost;

			if (NewCost <= MaxMovement && NewCost < CostSoFar[Next.X][Next.Y])
			{
				CostSoFar[Next.X][Next.Y] = NewCost;
				Frontier.Add(Next);
			}
		}
	}

	for (int32 i = 0; i < 25; i++)
	{
		for (int32 j = 0; j < 25; j++)
		{
			if (CostSoFar[i][j] > 0 && CostSoFar[i][j] <= MaxMovement)
			{
				ReachableCells.Add(GridCells[i][j]);
			}
		}
	}

	return ReachableCells;
}

// calcola l'array di celle in cui la pedina puo sparare basandosi sulla visuale e sull'altezza
TArray<AGridCell*> AGridManager::GetAttackableCells(int32 StartX, int32 StartY, int32 AttackRange)
{
	TArray<AGridCell*> AttackableCells;
	AGridCell* StartingCell = GridCells[StartX][StartY];

	if (!StartingCell) return AttackableCells;

	for (int32 x = StartX - AttackRange; x <= StartX + AttackRange; x++)
	{
		for (int32 y = StartY - AttackRange; y <= StartY + AttackRange; y++)
		{
			if (x < 0 || x >= 25 || y < 0 || y >= 25) continue;

			int32 Distance = FMath::Abs(x - StartX) + FMath::Abs(y - StartY);

			if (Distance > 0 && Distance <= AttackRange)
			{
				AGridCell* TargetCell = GridCells[x][y];

				if (TargetCell && TargetCell->ElevationLevel <= StartingCell->ElevationLevel)
				{
					AttackableCells.Add(TargetCell);
				}
			}
		}
	}

	return AttackableCells;
}

// fornisce la distanza base a croce tra due celle senza contare ostacoli
int32 AGridManager::GetManhattanDistance(AGridCell* StartCell, AGridCell* TargetCell)
{
	if (!StartCell || !TargetCell) return 0;
	return FMath::Abs(StartCell->GridPosition.X - TargetCell->GridPosition.X) +
		FMath::Abs(StartCell->GridPosition.Y - TargetCell->GridPosition.Y);
}

// utilizza l'algoritmo A* per calcolare il percorso ottimale che l'intelligenza artificiale usera per muoversi
TArray<AGridCell*> AGridManager::FindPath(AGridCell* StartCell, AGridCell* TargetCell)
{
	TArray<AGridCell*> Path;
	if (!StartCell || !TargetCell) return Path;

	TArray<FAStarNode*> OpenSet;
	TArray<FAStarNode*> ClosedSet;
	TArray<FAStarNode*> AllCreatedNodes;

	FAStarNode* StartNode = new FAStarNode(StartCell);
	AllCreatedNodes.Add(StartNode);
	OpenSet.Add(StartNode);

	FIntPoint Directions[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

	while (OpenSet.Num() > 0)
	{
		FAStarNode* CurrentNode = OpenSet[0];
		for (int32 i = 1; i < OpenSet.Num(); i++)
		{
			if (OpenSet[i]->FCost() < CurrentNode->FCost() ||
				(OpenSet[i]->FCost() == CurrentNode->FCost() && OpenSet[i]->HCost < CurrentNode->HCost))
			{
				CurrentNode = OpenSet[i];
			}
		}

		OpenSet.Remove(CurrentNode);
		ClosedSet.Add(CurrentNode);

		if (CurrentNode->Cell == TargetCell)
		{
			FAStarNode* RetraceNode = CurrentNode;
			while (RetraceNode != StartNode)
			{
				Path.Add(RetraceNode->Cell);
				RetraceNode = RetraceNode->Parent;
			}
			Algo::Reverse(Path);

			for (FAStarNode* Node : AllCreatedNodes) delete Node;
			return Path;
		}

		for (const FIntPoint& Dir : Directions)
		{
			int32 NextX = CurrentNode->Cell->GridPosition.X + Dir.X;
			int32 NextY = CurrentNode->Cell->GridPosition.Y + Dir.Y;

			if (NextX < 0 || NextX >= 25 || NextY < 0 || NextY >= 25) continue;

			AGridCell* NeighborCell = GridCells[NextX][NextY];

			if (NeighborCell->ElevationLevel == 0 || (NeighborCell->bIsOccupied && NeighborCell != TargetCell)) continue;

			bool bInClosedSet = false;
			for (FAStarNode* ClosedNode : ClosedSet) {
				if (ClosedNode->Cell == NeighborCell) { bInClosedSet = true; break; }
			}
			if (bInClosedSet) continue;

			int32 MoveCost = (NeighborCell->ElevationLevel > CurrentNode->Cell->ElevationLevel) ? 2 : 1;
			int32 NewGCost = CurrentNode->GCost + MoveCost;

			FAStarNode* NeighborNode = nullptr;
			bool bInOpenSet = false;
			for (FAStarNode* OpenNode : OpenSet) {
				if (OpenNode->Cell == NeighborCell) {
					NeighborNode = OpenNode;
					bInOpenSet = true;
					break;
				}
			}

			if (!bInOpenSet)
			{
				NeighborNode = new FAStarNode(NeighborCell);
				AllCreatedNodes.Add(NeighborNode);
			}

			if (NewGCost < NeighborNode->GCost || !bInOpenSet)
			{
				NeighborNode->GCost = NewGCost;
				NeighborNode->HCost = GetManhattanDistance(NeighborCell, TargetCell);
				NeighborNode->Parent = CurrentNode;

				if (!bInOpenSet) OpenSet.Add(NeighborNode);
			}
		}
	}

	for (FAStarNode* Node : AllCreatedNodes) delete Node;
	return Path;
}