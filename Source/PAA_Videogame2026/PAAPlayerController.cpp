#include "PAAPlayerController.h"
#include "MatchManager.h" 
#include "UnitBase.h"
#include "GridCell.h"
#include "Engine/World.h"
#include "EngineUtils.h" // <--- AGGIUNTO: Necessario per TActorIterator!
#include "Kismet/GameplayStatics.h"

APAAPlayerController::APAAPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void APAAPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("RightClick", IE_Pressed, this, &APAAPlayerController::OnRightClick);
	InputComponent->BindAction("LeftClick", IE_Pressed, this, &APAAPlayerController::OnLeftClick);
	InputComponent->BindAction("WaitAction", IE_Pressed, this, &APAAPlayerController::OnWaitPressed);
}

void APAAPlayerController::OnLeftClick()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) return;

		// 1. Controllo se è una cella
		AGridCell* ClickedCell = Cast<AGridCell>(HitActor);
		if (ClickedCell)
		{
			for (TActorIterator<AMatchManager> It(GetWorld()); It; ++It)
			{
				It->OnCellClicked(ClickedCell->GridPosition.X, ClickedCell->GridPosition.Y, ClickedCell->ElevationLevel);
				break;
			}
		}
		// 2. Controllo se è un'unità (Qualsiasi fazione)
		// Usiamo l'operatore || (OR) per includere anche i nemici
		else if (HitActor->ActorHasTag(FName("PlayerUnit")) || HitActor->ActorHasTag(FName("EnemyUnit")))
		{
			UE_LOG(LogTemp, Warning, TEXT("LineTrace: Unità cliccata! (Tag: %s)"), *HitActor->Tags[0].ToString());

			for (TActorIterator<AMatchManager> It(GetWorld()); It; ++It)
			{
				// Invece di filtrare qui, passiamo l'unità al MatchManager
				// Sarà lui a decidere se mostrarne solo le info o permettere il movimento
				It->OnUnitClicked(HitActor);
				break;
			}
		}
	}
}

void APAAPlayerController::OnRightClick()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.GetActor())
	{
		AGridCell* ClickedCell = Cast<AGridCell>(Hit.GetActor());
		AUnitBase* ClickedUnit = Cast<AUnitBase>(Hit.GetActor());

		// AGGIUNTO: Dobbiamo prima trovare il MatchManager nel mondo!
		AMatchManager* FoundMatchManager = nullptr;
		for (TActorIterator<AMatchManager> It(GetWorld()); It; ++It)
		{
			FoundMatchManager = *It;
			break;
		}

		if (FoundMatchManager)
		{
			if (ClickedCell)
			{
				FoundMatchManager->HandleRightClick(ClickedCell->GridPosition.X, ClickedCell->GridPosition.Y);
			}
			else if (ClickedUnit)
			{
				FoundMatchManager->HandleRightClick(ClickedUnit->GridX, ClickedUnit->GridY);
			}
		}
	}
}

void APAAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void APAAPlayerController::OnWaitPressed()
{
	AMatchManager* MM = nullptr;
	for (TActorIterator<AMatchManager> It(GetWorld()); It; ++It) { MM = *It; break; }

	if (MM && MM->SelectedUnit)
	{
		AUnitBase* Unit = Cast<AUnitBase>(MM->SelectedUnit);
		if (Unit && Unit->ActorHasTag("PlayerUnit"))
		{
			UE_LOG(LogTemp, Warning, TEXT("L'unita aspetta. Turno terminato per lei."));
			Unit->bHasMovedThisTurn = true;
			Unit->bHasAttackedThisTurn = true;

			// Opzionale: cambia colore per indicare che è esausta
			MM->ClearHighlights();
			MM->CheckEndTurn();
		}
	}
}