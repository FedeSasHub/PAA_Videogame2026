#include "GridManager.h"
#include "Algo/Reverse.h"
#include "Math/UnrealMathUtility.h"
#include "Components/TextRenderComponent.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;
}

void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	if (!CellClassToSpawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Manca la classe CellClassToSpawn nel GridManager!"));
		return;
	}

	// Inizializziamo il seed per avere una mappa diversa a ogni avvio
	FMath::RandInit(FDateTime::Now().GetTicks());
	float SeedOffset = FMath::RandRange(-10000.0f, 10000.0f);

	// Array per salvare i valori grezzi e variabili per trovare i picchi assoluti
	TArray<float> RawNoiseValues;
	RawNoiseValues.SetNum(25 * 25);
	float MinNoise = 10000.0f;
	float MaxNoise = -10000.0f;

	// --- PASSAGGIO 1: Calcoliamo il rumore di Perlin e troviamo il minimo e il massimo ---
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

	// --- PASSAGGIO 2: Generiamo la griglia normalizzando i valori (0-4 livelli) ---
	for (int32 x = 0; x < 25; x++)
	{
		for (int32 y = 0; y < 25; y++)
		{
			float RawNoise = RawNoiseValues[x * 25 + y];
			float NormalizedNoise = (RawNoise - MinNoise) / (MaxNoise - MinNoise);
			int32 Elevation = FMath::RoundToInt(NormalizedNoise * 4.0f);

			// Calcoliamo la posizione 3D basata sulla tua logica originale
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

	// --- PASSAGGIO 3: PIAZZAMENTO TORRI ---
	if (TowerClassToSpawn)
	{
		SpawnTowerAt(12, 12);
		SpawnTowerAt(5, 12);
		SpawnTowerAt(19, 12);
	}

	// --- PASSAGGIO 4: COORDINATE VISIVE (NUMERI E LETTERE) ---
	for (int32 i = 0; i < 25; i++)
	{
		// 1. NUMERI (Asse Orizzontale - In basso da sinistra a destra)
		FVector NumLoc(-120.0f, i * CellSize, 50.0f);
		UTextRenderComponent* NumText = NewObject<UTextRenderComponent>(this);
		if (NumText)
		{
			NumText->RegisterComponent();
			NumText->SetWorldLocation(NumLoc);

			// Correzione specchio: Yaw a 180.0f
			NumText->SetWorldRotation(FRotator(90.0f, 180.0f, 0.0f));

			NumText->SetText(FText::AsNumber(i));
			NumText->SetHorizontalAlignment(EHTA_Center);

			// --- MODIFICHE VARIABILI EDITOR ---
			NumText->SetTextRenderColor(CoordinateColor);
			NumText->SetWorldScale3D(FVector(CoordinateScale));
			if (CoordinateFont)
			{
				NumText->SetFont(CoordinateFont);
			}
		}

		// 2. LETTERE (Asse Verticale - A sinistra dal basso all'alto)
		// Quota Z abbassata a 10.0f per non farle fluttuare troppo
		FVector LetLoc((i * CellSize) - (CellSize * 0.4f), -120.0f, 10.0f);
		UTextRenderComponent* LetText = NewObject<UTextRenderComponent>(this);
		if (LetText)
		{
			LetText->RegisterComponent();
			LetText->SetWorldLocation(LetLoc);

			// Correzione specchio: Yaw a 180.0f
			LetText->SetWorldRotation(FRotator(90.0f, 180.0f, 0.0f));

			TCHAR Lettera = 'A' + i;
			LetText->SetText(FText::FromString(FString::Printf(TEXT("%c"), Lettera)));
			LetText->SetHorizontalAlignment(EHTA_Center);

			// --- MODIFICHE VARIABILI EDITOR ---
			LetText->SetTextRenderColor(CoordinateColor);
			LetText->SetWorldScale3D(FVector(CoordinateScale));
			if (CoordinateFont)
			{
				LetText->SetFont(CoordinateFont);
			}
		}
	}
}

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
				// Se siamo fuori dai bordi della mappa, ignoriamo
				if (x < 0 || x >= 25 || y < 0 || y >= 25) continue;

				// Controlliamo solo il perimetro del raggio corrente
				if (FMath::Abs(x - StartX) == Radius || FMath::Abs(y - StartY) == Radius)
				{
					// Se l'elevazione è > 0 (non è acqua), abbiamo trovato il punto!
					if (GridCells[x][y] && GridCells[x][y]->ElevationLevel > 0)
					{
						return FVector2D(x, y);
					}
				}
			}
		}
		Radius++;
	}
	return FVector2D(StartX, StartY); // Fallback estremo
}

void AGridManager::SpawnTowerAt(int32 GridX, int32 GridY)
{
	// Calcoliamo l'altezza Z in base alla cella che abbiamo trovato
	float ZPos = GridCells[GridX][GridY]->ElevationLevel * (CellSize * 0.5f);

	// Aggiungiamo 50 in Z così la torre non compenetra il terreno
	FVector SpawnLoc(GridY * CellSize, GridX * CellSize, ZPos + 50.0f);

	ATower* NewTower = GetWorld()->SpawnActor<ATower>(TowerClassToSpawn, SpawnLoc, FRotator::ZeroRotator);
	if (NewTower)
	{
		NewTower->SetupTower(GridX, GridY);
		GridCells[GridX][GridY]->bIsOccupied = true;
	}
}

TArray<AGridCell*> AGridManager::GetReachableCells(int32 StartX, int32 StartY, int32 MaxMovement)
{
	TArray<AGridCell*> ReachableCells;

	// Griglia dei costi: inizializziamo tutto a un numero altissimo (9999)
	int32 CostSoFar[25][25];
	for (int32 i = 0; i < 25; i++)
	{
		for (int32 j = 0; j < 25; j++) CostSoFar[i][j] = 9999;
	}

	// Lista delle celle da esplorare
	TArray<FIntPoint> Frontier;

	// Iniziamo dalla cella del soldato (Costo = 0)
	CostSoFar[StartX][StartY] = 0;
	Frontier.Add(FIntPoint(StartX, StartY));

	// Le 4 direzioni cardinali (Niente diagonali come da specifiche)
	FIntPoint Directions[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

	while (Frontier.Num() > 0)
	{
		// Prendiamo la cella esplorata per prima (FIFO base)
		FIntPoint Current = Frontier[0];
		Frontier.RemoveAt(0);

		AGridCell* CurrentCell = GridCells[Current.X][Current.Y];

		// Controlliamo i vicini
		for (const FIntPoint& Dir : Directions)
		{
			FIntPoint Next(Current.X + Dir.X, Current.Y + Dir.Y);

			// Se usciamo dalla mappa, ignoriamo
			if (Next.X < 0 || Next.X >= 25 || Next.Y < 0 || Next.Y >= 25) continue;

			AGridCell* NextCell = GridCells[Next.X][Next.Y];

			// OSTACOLI FISICI: Acqua (Elevazione 0) o celle occupate da Torri/Unità
			if (!NextCell || NextCell->ElevationLevel == 0 || NextCell->bIsOccupied) continue;

			// CALCOLO COSTO DI MOVIMENTO
			int32 MoveCost = 1; // Base per piano o discesa
			if (NextCell->ElevationLevel > CurrentCell->ElevationLevel)
			{
				MoveCost = 2; // Arrampicarsi costa il doppio!
			}

			int32 NewCost = CostSoFar[Current.X][Current.Y] + MoveCost;

			// Se abbiamo trovato un percorso più economico e rientriamo nel movimento massimo dell'unità
			if (NewCost <= MaxMovement && NewCost < CostSoFar[Next.X][Next.Y])
			{
				CostSoFar[Next.X][Next.Y] = NewCost;
				Frontier.Add(Next); // Aggiungiamo il vicino per esplorare oltre
			}
		}
	}

	// Ora raccogliamo tutte le celle con un costo valido
	for (int32 i = 0; i < 25; i++)
	{
		for (int32 j = 0; j < 25; j++)
		{
			// Se il costo è maggiore di 0 (non è la cella di partenza) e minore o uguale al range
			if (CostSoFar[i][j] > 0 && CostSoFar[i][j] <= MaxMovement)
			{
				ReachableCells.Add(GridCells[i][j]);
			}
		}
	}

	return ReachableCells;
}

TArray<AGridCell*> AGridManager::GetAttackableCells(int32 StartX, int32 StartY, int32 AttackRange)
{
	TArray<AGridCell*> AttackableCells;
	AGridCell* StartingCell = GridCells[StartX][StartY];

	if (!StartingCell) return AttackableCells;

	// Controlliamo tutte le celle in un "quadrato" attorno all'unita'
	for (int32 x = StartX - AttackRange; x <= StartX + AttackRange; x++)
	{
		for (int32 y = StartY - AttackRange; y <= StartY + AttackRange; y++)
		{
			// Saltiamo le celle fuori mappa
			if (x < 0 || x >= 25 || y < 0 || y >= 25) continue;

			// Calcoliamo la distanza "Manhattan" (a croce, niente diagonali)
			int32 Distance = FMath::Abs(x - StartX) + FMath::Abs(y - StartY);

			// Se la cella rientra nel raggio d'attacco ed NON e' la cella dove ci troviamo...
			if (Distance > 0 && Distance <= AttackRange)
			{
				AGridCell* TargetCell = GridCells[x][y];

				// REGOLE DI ATTACCO DAL PDF:
				// 1. Possiamo attaccare solo se il nemico e' allo stesso livello o inferiore
				if (TargetCell && TargetCell->ElevationLevel <= StartingCell->ElevationLevel)
				{
					AttackableCells.Add(TargetCell);
				}
			}
		}
	}

	return AttackableCells;
}

int32 AGridManager::GetManhattanDistance(AGridCell* StartCell, AGridCell* TargetCell)
{
	if (!StartCell || !TargetCell) return 0;
	// Distanza a croce (quanti passi mancano)
	return FMath::Abs(StartCell->GridPosition.X - TargetCell->GridPosition.X) +
		FMath::Abs(StartCell->GridPosition.Y - TargetCell->GridPosition.Y);
}

TArray<AGridCell*> AGridManager::FindPath(AGridCell* StartCell, AGridCell* TargetCell)
{
	TArray<AGridCell*> Path;
	if (!StartCell || !TargetCell) return Path;

	TArray<FAStarNode*> OpenSet;
	TArray<FAStarNode*> ClosedSet;

	// Teniamo traccia di tutti i nodi creati per pulire la memoria alla fine
	TArray<FAStarNode*> AllCreatedNodes;

	FAStarNode* StartNode = new FAStarNode(StartCell);
	AllCreatedNodes.Add(StartNode);
	OpenSet.Add(StartNode);

	FIntPoint Directions[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

	while (OpenSet.Num() > 0)
	{
		// 1. Trova il nodo con il costo F minore (il più promettente)
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

		// 2. SIAMO ARRIVATI AL BERSAGLIO! Ricostruiamo il percorso al contrario
		if (CurrentNode->Cell == TargetCell)
		{
			FAStarNode* RetraceNode = CurrentNode;
			while (RetraceNode != StartNode)
			{
				Path.Add(RetraceNode->Cell);
				RetraceNode = RetraceNode->Parent;
			}
			Algo::Reverse(Path); // Ribaltiamo l'array per averlo dall'inizio alla fine

			for (FAStarNode* Node : AllCreatedNodes) delete Node; // Pulizia memoria
			return Path;
		}

		// 3. Esploriamo i vicini (le 4 mattonelle a croce)
		for (const FIntPoint& Dir : Directions)
		{
			int32 NextX = CurrentNode->Cell->GridPosition.X + Dir.X;
			int32 NextY = CurrentNode->Cell->GridPosition.Y + Dir.Y;

			if (NextX < 0 || NextX >= 25 || NextY < 0 || NextY >= 25) continue; // Fuori mappa

			AGridCell* NeighborCell = GridCells[NextX][NextY];

			// È un ostacolo? (Acqua o cella occupata, a meno che non sia proprio il bersaglio finale da colpire)
			if (NeighborCell->ElevationLevel == 0 || (NeighborCell->bIsOccupied && NeighborCell != TargetCell)) continue;

			// Se è già stato esplorato, lo ignoriamo
			bool bInClosedSet = false;
			for (FAStarNode* ClosedNode : ClosedSet) {
				if (ClosedNode->Cell == NeighborCell) { bInClosedSet = true; break; }
			}
			if (bInClosedSet) continue;

			// Calcolo Costo G: +1 in piano/discesa, +2 in salita
			int32 MoveCost = (NeighborCell->ElevationLevel > CurrentNode->Cell->ElevationLevel) ? 2 : 1;
			int32 NewGCost = CurrentNode->GCost + MoveCost;

			// Cerchiamo se il vicino è già nell'OpenSet
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

			// Se abbiamo trovato un percorso migliore (o è un nodo nuovo)
			if (NewGCost < NeighborNode->GCost || !bInOpenSet)
			{
				NeighborNode->GCost = NewGCost;
				NeighborNode->HCost = GetManhattanDistance(NeighborCell, TargetCell);
				NeighborNode->Parent = CurrentNode;

				if (!bInOpenSet) OpenSet.Add(NeighborNode);
			}
		}
	}

	// Se non c'è strada, puliamo la memoria e torniamo vuoti
	for (FAStarNode* Node : AllCreatedNodes) delete Node;
	return Path;
}