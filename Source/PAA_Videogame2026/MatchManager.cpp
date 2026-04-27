#include "MatchManager.h"
#include "GridManager.h"          
#include "Math/UnrealMathUtility.h"
#include "Sniper.h"
#include "Brawler.h"
#include "UnitBase.h"
#include "GridCell.h"  
#include "Tower.h"
#include "MatchHUD.h"
#include "EngineUtils.h"      
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"


AMatchManager::AMatchManager()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentPhase = EMatchPhase::CoinFlip;
}

// inizializza l'interfaccia a schermo e avvia il timer per il lancio della moneta
void AMatchManager::BeginPlay()
{
	Super::BeginPlay();
	FMath::RandInit(FDateTime::Now().GetTicks());

	if (HUDClass)
	{
		ActiveHUD = CreateWidget<UMatchHUD>(GetWorld(), HUDClass);
		if (ActiveHUD)
		{
			ActiveHUD->AddToViewport();
			ActiveHUD->SetTurnText(TEXT("IN ATTESA..."));
			ActiveHUD->SetTowersText(0, 0);
		}
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMatchManager::FlipCoin, 1.0f, false);
}

// decide casualmente chi avra il diritto di schierare la prima pedina
void AMatchManager::FlipCoin()
{
	bool bHumanWinsFlip = FMath::RandBool();
	CurrentPhase = EMatchPhase::Deployment;

	if (bHumanWinsFlip)
	{
		FirstPlayer = EPlayerTurn::HumanPlayer;
		CurrentTurn = EPlayerTurn::HumanPlayer;
		UE_LOG(LogTemp, Warning, TEXT("TESTA! Inizi tu a schierare la prima unita."));
	}
	else
	{
		FirstPlayer = EPlayerTurn::AIPlayer;
		CurrentTurn = EPlayerTurn::AIPlayer;
		UE_LOG(LogTemp, Warning, TEXT("CROCE! Inizia l'AI."));
		DeploySingleAIUnit();
		CurrentTurn = EPlayerTurn::HumanPlayer;
	}
}

// cerca una cella di terra ferma e posiziona automaticamente un'unita nemica
void AMatchManager::DeploySingleAIUnit()
{
	int32 RandX, RandY;
	bool bValidLocation = false;
	int32 Attempts = 0;

	while (!bValidLocation && Attempts < 50)
	{
		RandX = FMath::RandRange(0, 24);
		RandY = FMath::RandRange(22, 24);
		Attempts++;

		for (TActorIterator<AGridCell> It(GetWorld()); It; ++It)
		{
			if (It->GridPosition.X == RandX && It->GridPosition.Y == RandY)
			{
				if (It->ElevationLevel > 0)
				{
					bValidLocation = true;
					FVector AISpawnLoc(RandY * 100.0f, RandX * 100.0f, (It->ElevationLevel * 50.0f) + 100.0f);
					TSubclassOf<AActor> AIUnitToSpawn = (AIUnitsDeployed == 0) ? TSubclassOf<AActor>(SniperClass) : TSubclassOf<AActor>(BrawlerClass);

					if (AIUnitToSpawn)
					{
						AActor* SpawnedAIUnit = GetWorld()->SpawnActor<AActor>(AIUnitToSpawn, AISpawnLoc, FRotator(0, 180, 0));
						if (SpawnedAIUnit)
						{
							SetUnitColor(SpawnedAIUnit, FLinearColor(0.5f, 0.0f, 0.2f));
							SpawnedAIUnit->Tags.Add(FName("EnemyUnit"));

							AUnitBase* BaseAIUnit = Cast<AUnitBase>(SpawnedAIUnit);
							if (BaseAIUnit)
							{
								BaseAIUnit->GridX = RandX;
								BaseAIUnit->GridY = RandY;
								BaseAIUnit->OriginalGridX = RandX;
								BaseAIUnit->OriginalGridY = RandY;
								BaseAIUnit->MaxHealthPoints = BaseAIUnit->HealthPoints;
							}
							AIUnitsDeployed++;
							UE_LOG(LogTemp, Warning, TEXT("L'AI ha schierato in X:%d Y:%d (Elevazione: %d)"), RandX, RandY, It->ElevationLevel);
						}
					}
				}
				break;
			}
		}
	}
}

// gestisce le fasi di schieramento e di pulizia cliccando su una cella vuota
void AMatchManager::OnCellClicked(int32 X, int32 Y, int32 Elevation)
{
	if (CurrentPhase == EMatchPhase::GameOver) return;

	if (CurrentPhase == EMatchPhase::Deployment && CurrentTurn == EPlayerTurn::HumanPlayer)
	{
		if (Y > 2) {
			UE_LOG(LogTemp, Error, TEXT("Puoi schierare solo nelle prime 3 righe (Y=0,1,2)!"));
			return;
		}
		if (Elevation == 0) {
			UE_LOG(LogTemp, Error, TEXT("Non puoi schierare unita in acqua!"));
			return;
		}

		FVector SpawnLoc(Y * 100.0f, X * 100.0f, (Elevation * 50.0f) + 100.0f);
		TSubclassOf<AActor> UnitToSpawn = (PlayerUnitsDeployed == 0) ? TSubclassOf<AActor>(SniperClass) : TSubclassOf<AActor>(BrawlerClass);

		if (UnitToSpawn)
		{
			AActor* NewUnit = GetWorld()->SpawnActor<AActor>(UnitToSpawn, SpawnLoc, FRotator::ZeroRotator);

			if (NewUnit)
			{
				SetUnitColor(NewUnit, FLinearColor(0.0f, 0.5f, 1.0f));
				NewUnit->Tags.Add(FName("PlayerUnit"));

				AUnitBase* BaseUnit = Cast<AUnitBase>(NewUnit);
				if (BaseUnit)
				{
					BaseUnit->GridX = X;
					BaseUnit->GridY = Y;
					BaseUnit->OriginalGridX = X;
					BaseUnit->OriginalGridY = Y;
					BaseUnit->MaxHealthPoints = BaseUnit->HealthPoints;
				}
				PlayerUnitsDeployed++;
				UE_LOG(LogTemp, Warning, TEXT("Hai schierato l'unita %d."), PlayerUnitsDeployed);

				if (AIUnitsDeployed < 2)
				{
					CurrentTurn = EPlayerTurn::AIPlayer;
					DeploySingleAIUnit();

					if (PlayerUnitsDeployed < 2)
					{
						CurrentTurn = EPlayerTurn::HumanPlayer;
					}
				}

				if (PlayerUnitsDeployed >= 2 && AIUnitsDeployed >= 2)
				{
					CurrentPhase = EMatchPhase::Playing;
					CurrentTurn = FirstPlayer;

					if (CurrentTurn == EPlayerTurn::HumanPlayer)
					{
						UE_LOG(LogTemp, Warning, TEXT("Schieramento completato. Inizia la battaglia! TOCCA A TE."));
						UpdateHealthHUD();
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Schieramento completato. Inizia la battaglia! TOCCA ALL'AI."));
						UpdateHealthHUD();
						CurrentTurn = EPlayerTurn::HumanPlayer;
						UE_LOG(LogTemp, Warning, TEXT("[DEBUG] L'AI salta il suo primo turno. Tocca a te!"));
					}
				}
			}
		}
		return;
	}

	if (CurrentPhase == EMatchPhase::Playing && CurrentTurn == EPlayerTurn::HumanPlayer)
	{
		if (SelectedUnit)
		{
			ClearHighlights();
			SetUnitColor(SelectedUnit, FLinearColor(0.0f, 0.5f, 1.0f));
			SelectedUnit = nullptr;
		}
	}
}

// illumina le celle di movimento e attacco relative all'unita selezionata dal giocatore
void AMatchManager::OnUnitClicked(AActor* ClickedUnit)
{
	if (CurrentPhase != EMatchPhase::Playing || !ClickedUnit) return;
	if (CurrentTurn != EPlayerTurn::HumanPlayer) return;

	AUnitBase* ClickedBaseUnit = Cast<AUnitBase>(ClickedUnit);

	if (ClickedUnit->ActorHasTag(FName("EnemyUnit")))
	{
		if (SelectedUnit)
		{
			SetUnitColor(SelectedUnit, FLinearColor(0.0f, 0.5f, 1.0f));
			ClearHighlights();
			SelectedUnit = nullptr;
		}
		return;
	}

	if (SelectedUnit)
	{
		SetUnitColor(SelectedUnit, FLinearColor(0.0f, 0.5f, 1.0f));
	}

	SelectedUnit = ClickedUnit;
	SetUnitColor(SelectedUnit, FLinearColor(0.5f, 1.0f, 1.0f));

	if (ClickedBaseUnit)
	{
		if (ClickedBaseUnit->bHasMovedThisTurn)
		{
			UE_LOG(LogTemp, Warning, TEXT("Questa unita si e' gia mossa!"));
			ClearHighlights();
			SetUnitColor(SelectedUnit, FLinearColor(0.0f, 0.5f, 1.0f));
			SelectedUnit = nullptr;
			return;
		}

		for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
		{
			ClearHighlights();
			ValidMoveCells = It->GetReachableCells(ClickedBaseUnit->GridX, ClickedBaseUnit->GridY, ClickedBaseUnit->MovementRange);
			ValidAttackCells = It->GetAttackableCells(ClickedBaseUnit->GridX, ClickedBaseUnit->GridY, ClickedBaseUnit->AttackRange);

			TArray<AGridCell*> AllAffectedCells = ValidMoveCells;
			for (AGridCell* AC : ValidAttackCells) AllAffectedCells.AddUnique(AC);

			for (AGridCell* Cell : AllAffectedCells)
			{
				bool bCanMove = ValidMoveCells.Contains(Cell);
				bool bCanAttack = ValidAttackCells.Contains(Cell);
				if (bCanMove && bCanAttack) Cell->HighlightHybridCell(true);
				else if (bCanMove) Cell->HighlightCell(true);
				else if (bCanAttack) Cell->HighlightAttackCell(true);
			}
			break;
		}
	}
}

// verifica la cella di destinazione e invia ordini di combattimento o spostamento
void AMatchManager::HandleRightClick(int32 X, int32 Y)
{
	if (CurrentPhase == EMatchPhase::GameOver) return;
	if (!SelectedUnit || CurrentTurn != EPlayerTurn::HumanPlayer) return;

	AUnitBase* BaseUnit = Cast<AUnitBase>(SelectedUnit);
	if (!BaseUnit) return;

	AGridCell* TargetCell = nullptr;
	for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
	{
		TargetCell = It->GridCells[X][Y];
		break;
	}

	if (!TargetCell) return;

	if (ValidAttackCells.Contains(TargetCell))
	{
		for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
		{
			if (It->GridX == X && It->GridY == Y && It->ActorHasTag("EnemyUnit"))
			{
				AUnitBase* EnemyUnit = *It;
				ExecuteAttack(BaseUnit, EnemyUnit);
				return;
			}
		}
	}

	if (ValidMoveCells.Contains(TargetCell) && !TargetCell->bIsOccupied)
	{
		ExecuteMovement(BaseUnit, X, Y, TargetCell);
	}
}

// resetta le abilita della fazione giocante e aggiorna i testi dei turni
void AMatchManager::StartNewTurn(EPlayerTurn NewTurn)
{
	CurrentTurn = NewTurn;
	ClearHighlights();
	SelectedUnit = nullptr;

	for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
	{
		if (NewTurn == EPlayerTurn::HumanPlayer && It->ActorHasTag("PlayerUnit"))
		{
			It->bHasMovedThisTurn = false;
			SetUnitColor(*It, FLinearColor::White);
		}
		else if (NewTurn == EPlayerTurn::AIPlayer && It->ActorHasTag("EnemyUnit"))
		{
			It->bHasMovedThisTurn = false;
			SetUnitColor(*It, FLinearColor::White);
		}
	}

	if (ActiveHUD)
	{
		FString TextTurn = (CurrentTurn == EPlayerTurn::HumanPlayer) ? TEXT("TUO TURNO") : TEXT("TURNO AVVERSARIO");
		ActiveHUD->SetTurnText(TextTurn);
	}

	if (CurrentTurn == EPlayerTurn::AIPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AI] Sto calcolando i percorsi ottimali..."));
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMatchManager::ExecuteAITurn, 1.0f, false);
	}
}

// assicura che il turno cambi solo se tutte le pedine della squadra attiva hanno agito
void AMatchManager::CheckEndTurn()
{
	bool bAllUnitsDone = true;

	for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
	{
		if (CurrentTurn == EPlayerTurn::HumanPlayer && It->ActorHasTag("PlayerUnit") && !It->bHasMovedThisTurn)
		{
			bAllUnitsDone = false;
			break;
		}
		if (CurrentTurn == EPlayerTurn::AIPlayer && It->ActorHasTag("EnemyUnit") && !It->bHasMovedThisTurn)
		{
			bAllUnitsDone = false;
			break;
		}
	}

	if (bAllUnitsDone)
	{
		EvaluateTowers();

		if (CurrentPhase == EMatchPhase::GameOver)
		{
			UE_LOG(LogTemp, Warning, TEXT("Partita Terminata! Il ciclo dei turni e' bloccato."));
			return;
		}

		if (CurrentTurn == EPlayerTurn::HumanPlayer)
			StartNewTurn(EPlayerTurn::AIPlayer);
		else
			StartNewTurn(EPlayerTurn::HumanPlayer);
	}
	else if (CurrentTurn == EPlayerTurn::AIPlayer)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMatchManager::ExecuteAITurn, 1.0f, false);
	}
}

// teletrasporta l'unita nella nuova posizione e aggiorna lo stato di occupazione della griglia
void AMatchManager::ExecuteMovement(AUnitBase* BaseUnit, int32 X, int32 Y, AGridCell* TargetCell)
{
	AGridManager* GridMgr = nullptr;
	for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
	{
		GridMgr = *It;
		break;
	}

	if (GridMgr && BaseUnit && TargetCell)
	{
		GridMgr->GridCells[BaseUnit->GridX][BaseUnit->GridY]->bIsOccupied = false;

		FString MoveLog = FString::Printf(TEXT("[%s] %s: %s -> %s"),
			(CurrentTurn == EPlayerTurn::HumanPlayer) ? TEXT("TU") : TEXT("AI"),
			Cast<ASniper>(BaseUnit) ? TEXT("S") : TEXT("B"),
			*GetChessCoordinate(BaseUnit->GridX, BaseUnit->GridY),
			*GetChessCoordinate(X, Y));

		if (ActiveHUD) ActiveHUD->AddMoveToHistory(MoveLog);

		FVector NewLocation(Y * 100.0f, X * 100.0f, (TargetCell->ElevationLevel * 50.0f) + 100.0f);
		BaseUnit->SetActorLocation(NewLocation);

		BaseUnit->GridX = X;
		BaseUnit->GridY = Y;
		TargetCell->bIsOccupied = true;

		BaseUnit->bHasMovedThisTurn = true;

		ClearHighlights();
		if (BaseUnit->ActorHasTag("PlayerUnit")) {
			SetUnitColor(BaseUnit, FLinearColor(0.0f, 0.5f, 1.0f));
		}
		else {
			SetUnitColor(BaseUnit, FLinearColor(0.5f, 0.0f, 0.2f));
		}
		SelectedUnit = nullptr;
		CheckEndTurn();
	}
}

// applica i danni, gestisce i contrattacchi mirati e respawna le unita con zero hp
void AMatchManager::ExecuteAttack(AUnitBase* Attacker, AUnitBase* Defender)
{
	if (!Attacker || !Defender) return;

	int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage);
	Defender->HealthPoints -= Damage;

	FString AttackLog = FString::Printf(TEXT("[%s] %s: %s DMG: %d"),
		(CurrentTurn == EPlayerTurn::HumanPlayer) ? TEXT("TU") : TEXT("AI"),
		Cast<ASniper>(Attacker) ? TEXT("S") : TEXT("B"),
		*GetChessCoordinate(Defender->GridX, Defender->GridY),
		Damage);

	if (ActiveHUD) ActiveHUD->AddMoveToHistory(AttackLog);

	UE_LOG(LogTemp, Warning, TEXT("BAM! %s infligge %d danni a %s! (HP Rimanenti: %d)"),
		*Attacker->GetName(), Damage, *Defender->GetName(), Defender->HealthPoints);

	int32 CombatDistance = FMath::Abs(Attacker->GridX - Defender->GridX) + FMath::Abs(Attacker->GridY - Defender->GridY);
	bool bShouldCounterAttack = false;

	if (Cast<ASniper>(Attacker))
	{
		if (Cast<ASniper>(Defender))
		{
			bShouldCounterAttack = true;
		}
		else if (Cast<ABrawler>(Defender) && CombatDistance == 1)
		{
			bShouldCounterAttack = true;
		}
	}

	if (bShouldCounterAttack && Defender->HealthPoints > 0)
	{
		int32 CounterDamage = FMath::RandRange(1, 3);
		Attacker->HealthPoints -= CounterDamage;
		UE_LOG(LogTemp, Warning, TEXT("CONTRATTACCO! %s restituisce il favore infliggendo %d danni a %s! (HP Attaccante: %d)"),
			*Defender->GetName(), CounterDamage, *Attacker->GetName(), Attacker->HealthPoints);

		FString CounterLog = FString::Printf(TEXT("[%s] Contrattacco: %d DMG"),
			(CurrentTurn == EPlayerTurn::HumanPlayer) ? TEXT("AI") : TEXT("TU"),
			CounterDamage);

		if (ActiveHUD) ActiveHUD->AddMoveToHistory(CounterLog);

		if (Attacker->HealthPoints <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s e' MORTO DI CONTRATTACCO! Torna alla base."), *Attacker->GetName());
			AGridManager* GridMgr = nullptr;
			for (TActorIterator<AGridManager> It(GetWorld()); It; ++It) { GridMgr = *It; break; }
			if (GridMgr)
			{
				GridMgr->GridCells[Attacker->GridX][Attacker->GridY]->bIsOccupied = false;
				Attacker->HealthPoints = Attacker->MaxHealthPoints;
				Attacker->GridX = Attacker->OriginalGridX; Attacker->GridY = Attacker->OriginalGridY;
				AGridCell* SpawnCell = GridMgr->GridCells[Attacker->OriginalGridX][Attacker->OriginalGridY];
				SpawnCell->bIsOccupied = true;
				FVector RespawnLoc(Attacker->OriginalGridY * 100.0f, Attacker->OriginalGridX * 100.0f, (SpawnCell->ElevationLevel * 50.0f) + 100.0f);
				Attacker->SetActorLocation(RespawnLoc);
			}
		}
	}

	if (Defender->HealthPoints <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s e' STATO SCONFITTO! Torna alla base."), *Defender->GetName());

		AGridManager* GridMgr = nullptr;
		for (TActorIterator<AGridManager> It(GetWorld()); It; ++It) { GridMgr = *It; break; }
		if (GridMgr)
		{
			GridMgr->GridCells[Defender->GridX][Defender->GridY]->bIsOccupied = false;
			Defender->HealthPoints = Defender->MaxHealthPoints;
			Defender->GridX = Defender->OriginalGridX;
			Defender->GridY = Defender->OriginalGridY;
			AGridCell* SpawnCell = GridMgr->GridCells[Defender->OriginalGridX][Defender->OriginalGridY];
			SpawnCell->bIsOccupied = true;
			FVector RespawnLoc(Defender->OriginalGridY * 100.0f, Defender->OriginalGridX * 100.0f, (SpawnCell->ElevationLevel * 50.0f) + 100.0f);
			Defender->SetActorLocation(RespawnLoc);
		}
	}

	Attacker->bHasMovedThisTurn = true;
	Attacker->bHasAttackedThisTurn = true;

	ClearHighlights();
	if (Attacker->ActorHasTag("PlayerUnit")) {
		SetUnitColor(Attacker, FLinearColor(0.0f, 0.5f, 1.0f));
	}
	else {
		SetUnitColor(Attacker, FLinearColor(0.5f, 0.0f, 0.2f));
	}

	SelectedUnit = nullptr;
	UpdateHealthHUD();
	CheckEndTurn();
}

// logica centrale dell'avversario per prendere decisioni tra sparare, muoversi e presidiare
void AMatchManager::ExecuteAITurn()
{
	AGridManager* GridMgr = nullptr;
	for (TActorIterator<AGridManager> It(GetWorld()); It; ++It) { GridMgr = *It; break; }
	if (!GridMgr) return;

	AUnitBase* ActiveAI = nullptr;
	for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It) {
		if (It->ActorHasTag("EnemyUnit") && !It->bHasMovedThisTurn) {
			ActiveAI = *It;
			break;
		}
	}

	if (!ActiveAI) {
		CheckEndTurn();
		return;
	}

	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		if (It->CurrentOwner == ETowerOwner::AIPlayer)
		{
			int32 DistX = FMath::Abs(ActiveAI->GridX - (int32)It->GridPosition.X);
			int32 DistY = FMath::Abs(ActiveAI->GridY - (int32)It->GridPosition.Y);

			if (DistX <= 2 && DistY <= 2)
			{
				bool bAltroAlleatoPresente = false;
				for (TActorIterator<AUnitBase> Altro(GetWorld()); Altro; ++Altro)
				{
					if (Altro->ActorHasTag("EnemyUnit") && *Altro != ActiveAI)
					{
						int32 AltroDistX = FMath::Abs(Altro->GridX - (int32)It->GridPosition.X);
						int32 AltroDistY = FMath::Abs(Altro->GridY - (int32)It->GridPosition.Y);
						if (AltroDistX <= 2 && AltroDistY <= 2)
						{
							bAltroAlleatoPresente = true;
							break;
						}
					}
				}

				if (!bAltroAlleatoPresente)
				{
					TArray<AGridCell*> CelleAttaccoStatica = GridMgr->GetAttackableCells(ActiveAI->GridX, ActiveAI->GridY, ActiveAI->AttackRange);
					for (AGridCell* Cell : CelleAttaccoStatica) {
						for (TActorIterator<AUnitBase> UnitIt(GetWorld()); UnitIt; ++UnitIt) {
							if (UnitIt->ActorHasTag("PlayerUnit") && UnitIt->GridX == Cell->GridPosition.X && UnitIt->GridY == Cell->GridPosition.Y) {
								ExecuteAttack(ActiveAI, *UnitIt);
								return;
							}
						}
					}

					ActiveAI->bHasMovedThisTurn = true;
					SetUnitColor(ActiveAI, FLinearColor(0.5f, 0.0f, 0.2f));
					CheckEndTurn();
					return;
				}
			}
		}
	}

	AGridCell* StartCell = GridMgr->GridCells[ActiveAI->GridX][ActiveAI->GridY];

	TArray<AGridCell*> CelleDiAttacco = GridMgr->GetAttackableCells(ActiveAI->GridX, ActiveAI->GridY, ActiveAI->AttackRange);
	for (AGridCell* Cell : CelleDiAttacco) {
		for (TActorIterator<AUnitBase> UnitIt(GetWorld()); UnitIt; ++UnitIt) {
			if (UnitIt->ActorHasTag("PlayerUnit") && UnitIt->GridX == Cell->GridPosition.X && UnitIt->GridY == Cell->GridPosition.Y) {
				ExecuteAttack(ActiveAI, *UnitIt);
				return;
			}
		}
	}

	AGridCell* TargetIdeal = nullptr;
	int32 MinDist = 9999;

	for (TActorIterator<ATower> It(GetWorld()); It; ++It) {
		if (It->CurrentOwner != ETowerOwner::AIPlayer) {
			int32 Dist = FMath::Abs((int32)It->GridPosition.X - ActiveAI->GridX) + FMath::Abs((int32)It->GridPosition.Y - ActiveAI->GridY);
			if (Dist < MinDist) {
				MinDist = Dist;
				TargetIdeal = GridMgr->GridCells[(int32)It->GridPosition.X][(int32)It->GridPosition.Y];
			}
		}
	}

	if (!TargetIdeal) {
		for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It) {
			if (It->ActorHasTag("PlayerUnit")) {
				int32 Dist = FMath::Abs(It->GridX - ActiveAI->GridX) + FMath::Abs(It->GridY - ActiveAI->GridY);
				if (Dist < MinDist) {
					MinDist = Dist;
					TargetIdeal = GridMgr->GridCells[It->GridX][It->GridY];
				}
			}
		}
	}

	if (TargetIdeal) {
		TArray<AGridCell*> Path = GridMgr->FindPath(StartCell, TargetIdeal);

		if (Path.Num() > 0) {
			AGridCell* ArrivalCell = StartCell;
			int32 CurrentCost = 0;

			for (AGridCell* StepCell : Path) {
				if (StepCell->bIsOccupied) break;

				bool bTowerCell = false;
				for (TActorIterator<ATower> Twr(GetWorld()); Twr; ++Twr) {
					if ((int32)Twr->GridPosition.X == StepCell->GridPosition.X && (int32)Twr->GridPosition.Y == StepCell->GridPosition.Y) {
						bTowerCell = true; break;
					}
				}
				if (bTowerCell) break;

				int32 StepCost = (StepCell->ElevationLevel > ArrivalCell->ElevationLevel) ? 2 : 1;

				if (CurrentCost + StepCost <= ActiveAI->MovementRange) {
					CurrentCost += StepCost;
					ArrivalCell = StepCell;
				}
				else {
					break;
				}
			}

			if (ArrivalCell != StartCell) {
				ExecuteMovement(ActiveAI, ArrivalCell->GridPosition.X, ArrivalCell->GridPosition.Y, ArrivalCell);
				return;
			}
		}
	}

	ActiveAI->bHasMovedThisTurn = true;
	SetUnitColor(ActiveAI, FLinearColor(0.5f, 0.0f, 0.2f));
	CheckEndTurn();
}

// calcola quante pedine occupano le zone torri e le assegna per stabilire il vincitore
void AMatchManager::EvaluateTowers()
{
	HumanTowersControlled = 0;
	AITowersControlled = 0;

	for (TActorIterator<ATower> It(GetWorld()); It; ++It)
	{
		ATower* Tower = *It;
		int32 PlayerUnitsInZone = 0;
		int32 AIUnitsInZone = 0;

		for (TActorIterator<AUnitBase> UnitIt(GetWorld()); UnitIt; ++UnitIt)
		{
			int32 DistX = FMath::Abs(UnitIt->GridX - Tower->GridPosition.X);
			int32 DistY = FMath::Abs(UnitIt->GridY - Tower->GridPosition.Y);

			if (DistX <= 2 && DistY <= 2)
			{
				if (UnitIt->ActorHasTag("PlayerUnit")) PlayerUnitsInZone++;
				else if (UnitIt->ActorHasTag("EnemyUnit")) AIUnitsInZone++;
			}
		}

		if (PlayerUnitsInZone > 0 && AIUnitsInZone > 0) {
			Tower->CurrentState = ETowerState::Contested;
			Tower->CurrentOwner = ETowerOwner::None;
			Tower->UpdateVisuals(FLinearColor::White);
		}
		else if (PlayerUnitsInZone > 0) {
			Tower->CurrentState = ETowerState::Captured;
			Tower->CurrentOwner = ETowerOwner::HumanPlayer;
			Tower->UpdateVisuals(FLinearColor::Blue);
		}
		else if (AIUnitsInZone > 0) {
			Tower->CurrentState = ETowerState::Captured;
			Tower->CurrentOwner = ETowerOwner::AIPlayer;
			Tower->UpdateVisuals(FLinearColor::Red);
		}
		else {
			if (Tower->CurrentState == ETowerState::Contested) {
				Tower->CurrentState = ETowerState::Neutral;
				Tower->CurrentOwner = ETowerOwner::None;
				Tower->UpdateVisuals(FLinearColor::Gray);
			}
		}

		if (Tower->CurrentOwner == ETowerOwner::HumanPlayer) HumanTowersControlled++;
		else if (Tower->CurrentOwner == ETowerOwner::AIPlayer) AITowersControlled++;
	}

	if (ActiveHUD)
	{
		ActiveHUD->SetTowersText(HumanTowersControlled, AITowersControlled);
	}

	bool bGameShouldEnd = false;

	if (CurrentTurn == EPlayerTurn::HumanPlayer)
	{
		if (HumanTowersControlled >= 2) {
			HumanConsecutiveTurns++;
			if (HumanConsecutiveTurns >= 2) bGameShouldEnd = true;
		}
		else HumanConsecutiveTurns = 0;
	}
	else
	{
		if (AITowersControlled >= 2) {
			AIConsecutiveTurns++;
			if (AIConsecutiveTurns >= 2) bGameShouldEnd = true;
		}
		else AIConsecutiveTurns = 0;
	}

	if (bGameShouldEnd && CurrentPhase != EMatchPhase::GameOver)
	{
		CurrentPhase = EMatchPhase::GameOver;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Reset, this, &AMatchManager::RunCountdown, 1.0f, true);
	}
}

// valuta a fine turno se una squadra ha vinto scatenando il reset
void AMatchManager::CheckGameOver()
{
	int32 PlayerUnitsCount = 0;
	int32 AIUnitsCount = 0;

	for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag("PlayerUnit")) PlayerUnitsCount++;
		else if (It->ActorHasTag("EnemyUnit")) AIUnitsCount++;
	}

	bool bGameShouldEnd = false;

	if (AIUnitsCount <= 0 || HumanTowersControlled >= 3)
	{
		bGameShouldEnd = true;
	}
	else if (PlayerUnitsCount <= 0 || AITowersControlled >= 3)
	{
		bGameShouldEnd = true;
	}

	if (bGameShouldEnd && CurrentPhase != EMatchPhase::GameOver)
	{
		CurrentPhase = EMatchPhase::GameOver;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Reset, this, &AMatchManager::RunCountdown, 1.0f, true);
	}
}

// crea una copia dinamica del materiale per colorare liberamente le pedine 3d
void AMatchManager::SetUnitColor(AActor* Unit, FLinearColor Color)
{
	if (!Unit || !BaseUnitMaterial) return;

	UStaticMeshComponent* Mesh = Unit->FindComponentByClass<UStaticMeshComponent>();
	if (Mesh)
	{
		UMaterialInstanceDynamic* DynamicMat = Mesh->CreateDynamicMaterialInstance(0, BaseUnitMaterial);
		if (DynamicMat)
		{
			DynamicMat->SetVectorParameterValue(FName("BodyColor"), Color);
			Mesh->SetMaterial(0, DynamicMat);
		}
	}
}

// spegne la colorazione speciale ripristinando le celle gialle e rosse al materiale standard
void AMatchManager::ClearHighlights()
{
	for (AGridCell* Cell : ValidMoveCells) if (Cell) Cell->HighlightCell(false);
	for (AGridCell* Cell : ValidAttackCells) if (Cell) Cell->HighlightCell(false);

	ValidMoveCells.Empty();
	ValidAttackCells.Empty();
}

// traduce le righe e colonne in coordinate stile battaglia navale
FString AMatchManager::GetChessCoordinate(int32 X, int32 Y)
{
	char Letter = 'A' + Y;
	return FString::Printf(TEXT("%c%d"), Letter, X);
}

// ripesca i punti ferita aggiornati e li spara sul pannello visivo laterale
void AMatchManager::UpdateHealthHUD()
{
	if (!ActiveHUD) return;

	int32 MySniperHP = 0;
	int32 MyBrawlerHP = 0;
	int32 AISniperHP = 0;
	int32 AIBrawlerHP = 0;

	for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
	{
		AUnitBase* Unit = *It;

		if (Unit->ActorHasTag(FName("PlayerUnit")))
		{
			if (Cast<ASniper>(Unit)) MySniperHP = Unit->HealthPoints;
			else if (Cast<ABrawler>(Unit)) MyBrawlerHP = Unit->HealthPoints;
		}
		else if (Unit->ActorHasTag(FName("EnemyUnit")))
		{
			if (Cast<ASniper>(Unit)) AISniperHP = Unit->HealthPoints;
			else if (Cast<ABrawler>(Unit)) AIBrawlerHP = Unit->HealthPoints;
		}
	}

	ActiveHUD->UpdateGlobalStats(MySniperHP, MyBrawlerHP, AISniperHP, AIBrawlerHP);
}

// esegue lo scorrere dei secondi centrali e ricarica il sistema arrivato a zero
void AMatchManager::RunCountdown()
{
	CountdownReset--;

	if (ActiveHUD)
	{
		FString Message = (HumanTowersControlled >= 2 || AIUnitsDeployed == 0) ? TEXT("VITTORIA!") : TEXT("SCONFITTA!");
		ActiveHUD->ShowFinalMessage(Message, CountdownReset);
	}

	if (CountdownReset <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Reset);
		RestartGame();
	}
}

// spazza via tutti i dati sporchi della partita forzando il motore a riaprire il file mappa
void AMatchManager::RestartGame()
{
	FName LevelName = *GetWorld()->GetMapName();
	FString LevelString = LevelName.ToString();
	LevelString.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelString));
}