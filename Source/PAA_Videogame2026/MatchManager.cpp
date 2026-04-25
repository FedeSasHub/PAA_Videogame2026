#include "MatchManager.h"
#include "GridManager.h"          // <--- QUESTO È QUELLO CHE FA CRASHARE IL COMPILATORE SE MANCA
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
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AMatchManager::AMatchManager()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentPhase = EMatchPhase::CoinFlip;
}

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
			// Test immediato dei testi
			ActiveHUD->SetTurnoText(TEXT("TEST CONNESSIONE OK"));
			ActiveHUD->SetTorriText(10, 10);
		}
	}
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMatchManager::FlipCoin, 1.0f, false);
}

void AMatchManager::FlipCoin()
{
	bool bHumanWinsFlip = FMath::RandBool();
	CurrentPhase = EMatchPhase::Deployment;

	if (bHumanWinsFlip)
	{
		FirstPlayer = EPlayerTurn::HumanPlayer; // <--- SALVIAMO IL VINCITORE
		CurrentTurn = EPlayerTurn::HumanPlayer;
		UE_LOG(LogTemp, Warning, TEXT("TESTA! Inizi tu a schierare la prima unita."));
	}
	else
	{
		FirstPlayer = EPlayerTurn::AIPlayer;   // <--- SALVIAMO IL VINCITORE
		CurrentTurn = EPlayerTurn::AIPlayer;
		UE_LOG(LogTemp, Warning, TEXT("CROCE! Inizia l'AI."));
		DeploySingleAIUnit();
		CurrentTurn = EPlayerTurn::HumanPlayer;
	}
}

void AMatchManager::OnCellClicked(int32 X, int32 Y, int32 Elevation)
{
	// --- FASE 1: SCHIERAMENTO ---
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
					// --- AGGIUNTE PER IL RESPAWN ---
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
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Schieramento completato. Inizia la battaglia! TOCCA ALL'AI."));
						CurrentTurn = EPlayerTurn::HumanPlayer;
						UE_LOG(LogTemp, Warning, TEXT("[DEBUG] L'AI salta il suo primo turno. Tocca a te!"));
					}
				}
			}
		}
		return; // Ferma l'esecuzione qui
	}

	// --- FASE 2: GIOCO ---
	// Se siamo in fase di gioco e clicchiamo col TASTO SINISTRO su una cella vuota,
	// deselezioniamo l'unità attualmente selezionata per fare pulizia a schermo.
	if (CurrentPhase == EMatchPhase::Playing && CurrentTurn == EPlayerTurn::HumanPlayer)
	{
		if (SelectedUnit)
		{
			ClearHighlights();
			SetUnitColor(SelectedUnit, FLinearColor(0.0f, 0.5f, 1.0f));
			SelectedUnit = nullptr;

			// Pulizia HUD
			if (ActiveHUD) ActiveHUD->SetStatisticheText(TEXT("Seleziona un'unita'"), 0, 0);
		}
	}
}

void AMatchManager::DeploySingleAIUnit()
{
	int32 RandX, RandY;
	bool bValidLocation = false;
	int32 Tentativi = 0;

	// Ciclo finché non trova una cella valida (massimo 50 tentativi per evitare loop infiniti)
	while (!bValidLocation && Tentativi < 50)
	{
		RandX = FMath::RandRange(0, 24);
		RandY = FMath::RandRange(22, 24);
		Tentativi++;

		// Cerchiamo la cella nel mondo per controllare l'elevazione
		for (TActorIterator<AGridCell> It(GetWorld()); It; ++It)
		{
			if (It->GridPosition.X == RandX && It->GridPosition.Y == RandY)
			{
				if (It->ElevationLevel > 0) // NON È ACQUA
				{
					bValidLocation = true;

					// Procediamo con lo spawn
					FVector AISpawnLoc(RandY * 100.0f, RandX * 100.0f, (It->ElevationLevel * 50.0f) + 100.0f);
					TSubclassOf<AActor> AIUnitToSpawn = (AIUnitsDeployed == 0) ? TSubclassOf<AActor>(SniperClass) : TSubclassOf<AActor>(BrawlerClass);

					if (AIUnitToSpawn)
					{
						AActor* SpawnedAIUnit = GetWorld()->SpawnActor<AActor>(AIUnitToSpawn, AISpawnLoc, FRotator(0, 180, 0));
						if (SpawnedAIUnit)
						{
							SetUnitColor(SpawnedAIUnit, FLinearColor(0.5f, 0.0f, 0.2f));
							SpawnedAIUnit->Tags.Add(FName("EnemyUnit")); // Gia che ci siamo, etichettiamo anche loro!

							
							AUnitBase* BaseAIUnit = Cast<AUnitBase>(SpawnedAIUnit);
							if (BaseAIUnit)
							{
								BaseAIUnit->GridX = RandX;
								BaseAIUnit->GridY = RandY;
								// --- AGGIUNTE PER IL RESPAWN ---
								BaseAIUnit->OriginalGridX = RandX;
								BaseAIUnit->OriginalGridY = RandY;
								BaseAIUnit->MaxHealthPoints = BaseAIUnit->HealthPoints;
							}
							AIUnitsDeployed++;
							UE_LOG(LogTemp, Warning, TEXT("L'AI ha schierato in X:%d Y:%d (Elevazione: %d)"), RandX, RandY, It->ElevationLevel);
						}
					}
				}
				break; // Trovata la cella, esce dal ciclo TActorIterator
			}
		}
	}
}

void AMatchManager::SetUnitColor(AActor* Unit, FLinearColor Color)
{
	if (!Unit || !BaseUnitMaterial) return;

	// 1. Cerchiamo la Static Mesh dell'unità
	UStaticMeshComponent* Mesh = Unit->FindComponentByClass<UStaticMeshComponent>();
	if (Mesh)
	{
		// 2. Creiamo una copia dinamica del materiale base
		UMaterialInstanceDynamic* DynamicMat = Mesh->CreateDynamicMaterialInstance(0, BaseUnitMaterial);
		if (DynamicMat)
		{
			// 3. Cambiamo il parametro 'BodyColor' (stesso nome dato in Unreal!)
			DynamicMat->SetVectorParameterValue(FName("BodyColor"), Color);

			// Applichiamo il materiale dinamico alla mesh
			Mesh->SetMaterial(0, DynamicMat);
		}
	}
}
void AMatchManager::OnUnitClicked(AActor* ClickedUnit)
{
	// 1. REQUISITI MINIMI: Se non siamo in gioco, usciamo subito
	if (CurrentPhase != EMatchPhase::Playing || !ClickedUnit) return;

	// 2. AGGIORNAMENTO HUD (Deve essere qui, prima di ogni blocco/return!)
	AUnitBase* ClickedBaseUnit = Cast<AUnitBase>(ClickedUnit);
	if (ActiveHUD && ClickedBaseUnit)
	{
		// Controlliamo i tag per capire cosa scrivere nell'interfaccia
		FString Tipo = ClickedUnit->ActorHasTag(FName("PlayerUnit")) ? TEXT("ALLEATO") : TEXT("NEMICO");

		// Mandiamo i dati al Widget verde acqua
		ActiveHUD->SetStatisticheText(Tipo, ClickedBaseUnit->HealthPoints, ClickedBaseUnit->MaxHealthPoints);
	}

	// 3. FILTRO TURNO: Se non è il turno dell'umano, non permettiamo la selezione/movimento
	if (CurrentTurn != EPlayerTurn::HumanPlayer) return;

	// 4. FILTRO FAZIONE: Se clicchi un nemico durante il tuo turno, 
	// abbiamo già mostrato gli HP sopra, quindi ora usciamo per non "selezionarlo" come nostra pedina.
	if (ClickedUnit->ActorHasTag(FName("EnemyUnit")))
	{
		// Opzionale: puliamo i cerchi gialli se avevamo qualcun altro selezionato
		if (SelectedUnit)
		{
			SetUnitColor(SelectedUnit, FLinearColor(0.0f, 0.5f, 1.0f));
			ClearHighlights();
			SelectedUnit = nullptr;
		}
		return;
	}

	// 5. SELEZIONE ALLEATO (Logica di movimento/attacco)
	if (SelectedUnit)
	{
		SetUnitColor(SelectedUnit, FLinearColor(0.0f, 0.5f, 1.0f));
	}

	SelectedUnit = ClickedUnit;
	SetUnitColor(SelectedUnit, FLinearColor(0.5f, 1.0f, 1.0f)); // Azzurro selezione

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

		// Calcolo zone d'azione (Movimento/Attacco)
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
void AMatchManager::ClearHighlights()
{
	// Spegniamo tutto usando un unico ciclo su entrambe le liste
	for (AGridCell* Cell : ValidMoveCells) if (Cell) Cell->HighlightCell(false);
	for (AGridCell* Cell : ValidAttackCells) if (Cell) Cell->HighlightCell(false);

	ValidMoveCells.Empty();
	ValidAttackCells.Empty();
}

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
		EvaluateTowers(); // Valuta gli obiettivi a fine turno

		// ---> NUOVO: SE QUALCUNO HA VINTO, FERMA TUTTO! <---
		if (CurrentPhase == EMatchPhase::GameOver)
		{
			UE_LOG(LogTemp, Warning, TEXT("Partita Terminata! Il ciclo dei turni e' bloccato."));
			return; // Usciamo dalla funzione senza passare il turno
		}

		// Cambia turno (se stiamo ancora giocando)
		if (CurrentTurn == EPlayerTurn::HumanPlayer)
			StartNewTurn(EPlayerTurn::AIPlayer);
		else
			StartNewTurn(EPlayerTurn::HumanPlayer);
	}
	else if (CurrentTurn == EPlayerTurn::AIPlayer)
	{
		// L'IA non ha finito! Aspetta 1 secondo e muovi il soldato successivo
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMatchManager::ExecuteAITurn, 1.0f, false);
	}
}

void AMatchManager::StartNewTurn(EPlayerTurn NewTurn)
{
	CurrentTurn = NewTurn;
	ClearHighlights();
	SelectedUnit = nullptr;

	// Sblocchiamo tutte le unità del giocatore attivo
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

	// AGGIORNAMENTO HUD
	if (ActiveHUD)
	{
		FString TestoTurno = (CurrentTurn == EPlayerTurn::HumanPlayer) ? TEXT("TUO TURNO") : TEXT("TURNO AI");
		ActiveHUD->SetTurnoText(TestoTurno);
		ActiveHUD->SetStatisticheText(TEXT("Seleziona un'unita'"), 0, 0);
	}

	// LOGICA AI
	if (CurrentTurn == EPlayerTurn::AIPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AI] Sto calcolando i percorsi ottimali..."));
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AMatchManager::ExecuteAITurn, 1.0f, false);
	}
}

void AMatchManager::HandleRightClick(int32 X, int32 Y)
{
	if (!SelectedUnit || CurrentTurn != EPlayerTurn::HumanPlayer) return;

	AUnitBase* BaseUnit = Cast<AUnitBase>(SelectedUnit);
	if (!BaseUnit) return;

	// Troviamo la cella bersaglio nel GridManager
	AGridCell* TargetCell = nullptr;
	for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
	{
		TargetCell = It->GridCells[X][Y];
		break;
	}

	if (!TargetCell) return;

	// --- CASO A: ATTACCO ---
	// Se la cella è Rossa o Arancione (ValidAttackCells) E c'è un nemico
	if (ValidAttackCells.Contains(TargetCell))
	{
		for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
		{
			if (It->GridX == X && It->GridY == Y && It->ActorHasTag("EnemyUnit"))
			{
				// Abbiamo trovato il nemico, passiamolo alla funzione di attacco!
				AUnitBase* EnemyUnit = *It;
				ExecuteAttack(BaseUnit, EnemyUnit);
				return;
			}
		}
	}

	// --- CASO B: MOVIMENTO ---
	// Se la cella è Gialla o Arancione (ValidMoveCells) ed è libera
	if (ValidMoveCells.Contains(TargetCell) && !TargetCell->bIsOccupied)
	{
		// Usiamo la logica che avevi già nel OnCellClicked per spostare l'unità
		ExecuteMovement(BaseUnit, X, Y, TargetCell);
	}
}

void AMatchManager::ExecuteMovement(AUnitBase* BaseUnit, int32 X, int32 Y, AGridCell* TargetCell)
{
	// Cerchiamo il GridManager per poter aggiornare l'occupazione delle celle
	AGridManager* GridMgr = nullptr;
	for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
	{
		GridMgr = *It;
		break;
	}

	if (GridMgr && BaseUnit && TargetCell)
	{
		// 1. Liberiamo la vecchia mattonella!
		GridMgr->GridCells[BaseUnit->GridX][BaseUnit->GridY]->bIsOccupied = false;

		// 2. Spostiamo fisicamente l'attore (usando l'elevazione della nuova cella)
		FVector NewLocation(Y * 100.0f, X * 100.0f, (TargetCell->ElevationLevel * 50.0f) + 100.0f);
		BaseUnit->SetActorLocation(NewLocation);

		// 3. Aggiorniamo il GPS e occupiamo la nuova mattonella!
		BaseUnit->GridX = X;
		BaseUnit->GridY = Y;
		TargetCell->bIsOccupied = true;

		// 4. BLOCCIAMO I FUTURI MOVIMENTI PER QUESTO TURNO
		BaseUnit->bHasMovedThisTurn = true;

		UE_LOG(LogTemp, Warning, TEXT("Movimento eseguito tramite Tasto Destro su X:%d Y:%d."), X, Y);

		// 5. Spegniamo le luci e passiamo il turno
		ClearHighlights();
		if (BaseUnit->ActorHasTag("PlayerUnit")) {
			SetUnitColor(BaseUnit, FLinearColor(0.0f, 0.5f, 1.0f)); // Azzurro
		}
		else {
			SetUnitColor(BaseUnit, FLinearColor(0.5f, 0.0f, 0.2f)); // Rosso Scuro
		}
		SelectedUnit = nullptr;
		CheckEndTurn();
	}
}

void AMatchManager::ExecuteAttack(AUnitBase* Attacker, AUnitBase* Defender)
{
	if (!Attacker || !Defender) return;

	// 1. CALCOLO DEI DANNI DELL'ATTACCANTE
	int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage);
	Defender->HealthPoints -= Damage;

	UE_LOG(LogTemp, Warning, TEXT("BAM! %s infligge %d danni a %s! (HP Rimanenti: %d)"),
		*Attacker->GetName(), Damage, *Defender->GetName(), Defender->HealthPoints);

	// --- NUOVO: CONTRATTACCO (Requisito 8) ---
	int32 DistanzaCombattimento = FMath::Abs(Attacker->GridX - Defender->GridX) + FMath::Abs(Attacker->GridY - Defender->GridY);

	// Se sono adiacenti (Distanza 1) E il difensore è sopravvissuto, c'è il contrattacco!
	if (DistanzaCombattimento == 1 && Defender->HealthPoints > 0)
	{
		int32 CounterDamage = FMath::RandRange(1, 3);
		Attacker->HealthPoints -= CounterDamage;
		UE_LOG(LogTemp, Warning, TEXT("CONTRATTACCO! %s restituisce il favore infliggendo %d danni a %s! (HP Attaccante: %d)"),
			*Defender->GetName(), CounterDamage, *Attacker->GetName(), Attacker->HealthPoints);

		// Dobbiamo controllare se l'ATTACCANTE è morto per il contrattacco!
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
	// ----------------------------------------

	// 2. CONTROLLO SCONFITTA E RESPAWN DEL DIFENSORE
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

	// 3. BLOCCIAMO L'UNITÀ ATTACCANTE (Il suo turno finisce)
	Attacker->bHasMovedThisTurn = true;
	Attacker->bHasAttackedThisTurn = true;

	// 4. Puliamo la grafica e deselezioniamo (ERRORE CORRETTO QUI!)
	ClearHighlights();
	if (Attacker->ActorHasTag("PlayerUnit")) {
		SetUnitColor(Attacker, FLinearColor(0.0f, 0.5f, 1.0f)); // Azzurro
	}
	else {
		SetUnitColor(Attacker, FLinearColor(0.5f, 0.0f, 0.2f)); // Rosso Scuro
	}
	SelectedUnit = nullptr;

	// 5. Controlliamo se il turno deve passare
	CheckEndTurn();
}

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
			HumanTowersControlled++;
		}
		else if (AIUnitsInZone > 0) {
			Tower->CurrentState = ETowerState::Captured;
			Tower->CurrentOwner = ETowerOwner::AIPlayer;
			Tower->UpdateVisuals(FLinearColor::Red);
			AITowersControlled++;
		}
		else {
			Tower->CurrentState = ETowerState::Neutral;
			Tower->CurrentOwner = ETowerOwner::None;
			Tower->UpdateVisuals(FLinearColor::Gray);
		}
	}

	// AGGIORNAMENTO HUD
	if (ActiveHUD)
	{
		ActiveHUD->SetTorriText(HumanTowersControlled, AITowersControlled);
	}

	// CONTROLLO CONDIZIONE DI VITTORIA (2 Torri per 2 Turni)
	if (CurrentTurn == EPlayerTurn::HumanPlayer)
	{
		if (HumanTowersControlled >= 2) {
			HumanConsecutiveTurns++;
			if (HumanConsecutiveTurns >= 2) {
				CurrentPhase = EMatchPhase::GameOver;
				if (ActiveHUD) ActiveHUD->SetTurnoText(TEXT("VITTORIA UMANO!"));
			}
		}
		else HumanConsecutiveTurns = 0;
	}
	else
	{
		if (AITowersControlled >= 2) {
			AIConsecutiveTurns++;
			if (AIConsecutiveTurns >= 2) {
				CurrentPhase = EMatchPhase::GameOver;
				if (ActiveHUD) ActiveHUD->SetTurnoText(TEXT("VITTORIA AI!"));
			}
		}
		else AIConsecutiveTurns = 0;
	}
}

// --- LOGICA AI E GESTIONE TURNO ---

void AMatchManager::ExecuteAITurn()
{
	UE_LOG(LogTemp, Warning, TEXT("L'AI sta analizzando la mappa..."));

	AGridManager* GridMgr = nullptr;
	for (TActorIterator<AGridManager> It(GetWorld()); It; ++It) { GridMgr = *It; break; }
	if (!GridMgr) return;

	// 1. Troviamo il primo soldato nemico pronto all'azione
	AUnitBase* ActiveAI = nullptr;
	for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It) {
		if (It->ActorHasTag("EnemyUnit") && !It->bHasMovedThisTurn) {
			ActiveAI = *It;
			break;
		}
	}

	// Sicurezza: se non c'è nessuno da muovere, passa il turno
	if (!ActiveAI) {
		CheckEndTurn();
		return;
	}

	AGridCell* StartCell = GridMgr->GridCells[ActiveAI->GridX][ActiveAI->GridY];

	// --- PRIORITÀ 1: SCONTRO A FUOCO ---
	// L'IA controlla se c'è un umano nel suo raggio di attacco. Se sì, spara e sta ferma.
	TArray<AGridCell*> CelleDiAttacco = GridMgr->GetAttackableCells(ActiveAI->GridX, ActiveAI->GridY, ActiveAI->AttackRange);
	for (AGridCell* Cell : CelleDiAttacco) {
		for (TActorIterator<AUnitBase> UnitIt(GetWorld()); UnitIt; ++UnitIt) {
			if (UnitIt->ActorHasTag("PlayerUnit") && UnitIt->GridX == Cell->GridPosition.X && UnitIt->GridY == Cell->GridPosition.Y) {
				UE_LOG(LogTemp, Warning, TEXT("L'AI ha un bersaglio a tiro! FUOCO!"));
				ExecuteAttack(ActiveAI, *UnitIt);
				return; // ExecuteAttack passa il turno da solo, quindi usciamo qui!
			}
		}
	}

	// --- PRIORITÀ 2: CATTURA TORRI O INSEGUIMENTO ---
	AGridCell* BersaglioIdeale = nullptr;
	int32 DistanzaMinima = 9999;

	// Cerca la torre neutrale o umana più vicina
	for (TActorIterator<ATower> It(GetWorld()); It; ++It) {
		if (It->CurrentOwner != ETowerOwner::AIPlayer) {
			int32 Dist = FMath::Abs(It->GridPosition.X - ActiveAI->GridX) + FMath::Abs(It->GridPosition.Y - ActiveAI->GridY);
			if (Dist < DistanzaMinima) {
				DistanzaMinima = Dist;
				BersaglioIdeale = GridMgr->GridCells[(int32)It->GridPosition.X][(int32)It->GridPosition.Y];
			}
		}
	}

	// Se ha già tutte le torri, bracca il giocatore
	if (!BersaglioIdeale) {
		for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It) {
			if (It->ActorHasTag("PlayerUnit")) {
				int32 Dist = FMath::Abs(It->GridX - ActiveAI->GridX) + FMath::Abs(It->GridY - ActiveAI->GridY);
				if (Dist < DistanzaMinima) {
					DistanzaMinima = Dist;
					BersaglioIdeale = GridMgr->GridCells[It->GridX][It->GridY];
				}
			}
		}
	}

	// --- 3. NAVIGAZIONE (GPS) ---
	if (BersaglioIdeale) {
		TArray<AGridCell*> Percorso = GridMgr->FindPath(StartCell, BersaglioIdeale);

		if (Percorso.Num() > 0) {
			AGridCell* CellaArrivo = StartCell;
			int32 CostoAttuale = 0;

			// Percorriamo la strada virtualmente mattonella per mattonella
			for (AGridCell* StepCell : Percorso) {

				// Ostacolo 1: Cella già occupata da qualcuno
				if (StepCell->bIsOccupied) break;

				// Ostacolo 2: È una Torre? (Ci fermiamo un passo prima per non salirci sopra)
				bool bCellaConTorre = false;
				for (TActorIterator<ATower> Twr(GetWorld()); Twr; ++Twr) {
					if (Twr->GridPosition.X == StepCell->GridPosition.X && Twr->GridPosition.Y == StepCell->GridPosition.Y) {
						bCellaConTorre = true; break;
					}
				}
				if (bCellaConTorre) break;

				// Calcolo fatica (Salita = 2, Piano = 1)
				int32 CostoPasso = (StepCell->ElevationLevel > CellaArrivo->ElevationLevel) ? 2 : 1;

				if (CostoAttuale + CostoPasso <= ActiveAI->MovementRange) {
					CostoAttuale += CostoPasso;
					CellaArrivo = StepCell;
				}
				else {
					break; // Punti movimento esauriti!
				}
			}

			// Se la cella calcolata è diversa da quella di partenza, muoviamoci fisicamente
			if (CellaArrivo != StartCell) {
				ExecuteMovement(ActiveAI, CellaArrivo->GridPosition.X, CellaArrivo->GridPosition.Y, CellaArrivo);
				return; // ExecuteMovement passa il turno da solo!
			}
		}
	}

	// Se l'unità arriva fin qui, significa che NON ha attaccato e NON si è mossa
	// (magari era già vicina alla torre a difenderla, o era incastrata).
	ActiveAI->bHasMovedThisTurn = true;

	// ---> AGGIUNGI QUESTA RIGA <---
	SetUnitColor(ActiveAI, FLinearColor(0.5f, 0.0f, 0.2f));

	CheckEndTurn();
}

void AMatchManager::CheckGameOver()
{
	int32 PlayerUnits = 0;
	int32 AIUnits = 0;

	// Contiamo quante unità sono rimaste in campo
	for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag("PlayerUnit")) PlayerUnits++;
		else if (It->ActorHasTag("EnemyUnit")) AIUnits++;
	}

	if (AIUnits <= 0 || HumanTowersControlled >= 3) // Esempio: 3 torri o 0 nemici
	{
		CurrentPhase = EMatchPhase::GameOver;
		if (ActiveHUD) ActiveHUD->SetTurnoText(TEXT("VITTORIA! HAI VINTO!"));
		UE_LOG(LogTemp, Warning, TEXT("PARTITA FINITA: Vittoria Umana"));
	}
	else if (PlayerUnits <= 0 || AITowersControlled >= 3)
	{
		CurrentPhase = EMatchPhase::GameOver;
		if (ActiveHUD) ActiveHUD->SetTurnoText(TEXT("SCONFITTA... Riprova!"));
		UE_LOG(LogTemp, Error, TEXT("PARTITA FINITA: Vittoria AI"));
	}
}

