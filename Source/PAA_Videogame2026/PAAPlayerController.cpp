#include "PAAPlayerController.h"
#include "MatchManager.h" 
#include "UnitBase.h"
#include "MatchHUD.h"
#include "GridCell.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

// abilita il cursore e gli eventi di interazione del mouse
APAAPlayerController::APAAPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

// collega le azioni mappate nell'editor alle funzioni del codice
void APAAPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("RightClick", IE_Pressed, this, &APAAPlayerController::OnRightClick);
	InputComponent->BindAction("LeftClick", IE_Pressed, this, &APAAPlayerController::OnLeftClick);
	InputComponent->BindAction("WaitAction", IE_Pressed, this, &APAAPlayerController::OnWaitPressed);
}

// impedisce al mouse di bloccarsi nella finestra di gioco permettendo di cliccare l'interfaccia
void APAAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

// cattura il clic sinistro e lo trasmette al match manager per selezionare celle o pedine
void APAAPlayerController::OnLeftClick()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) return;

		AGridCell* ClickedCell = Cast<AGridCell>(HitActor);
		if (ClickedCell)
		{
			for (TActorIterator<AMatchManager> It(GetWorld()); It; ++It)
			{
				It->OnCellClicked(ClickedCell->GridPosition.X, ClickedCell->GridPosition.Y, ClickedCell->ElevationLevel);
				break;
			}
		}
		else if (HitActor->ActorHasTag(FName("PlayerUnit")) || HitActor->ActorHasTag(FName("EnemyUnit")))
		{
			for (TActorIterator<AMatchManager> It(GetWorld()); It; ++It)
			{
				It->OnUnitClicked(HitActor);
				break;
			}
		}
	}
}

// cattura il clic destro per inviare ordini di movimento o attacco
void APAAPlayerController::OnRightClick()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.GetActor())
	{
		AGridCell* ClickedCell = Cast<AGridCell>(Hit.GetActor());
		AUnitBase* ClickedUnit = Cast<AUnitBase>(Hit.GetActor());

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

// fa terminare forzatamente il turno alla pedina selezionata in quel momento
void APAAPlayerController::OnWaitPressed()
{
	AMatchManager* FoundMatchManager = nullptr;
	for (TActorIterator<AMatchManager> It(GetWorld()); It; ++It)
	{
		FoundMatchManager = *It;
		break;
	}

	if (FoundMatchManager && FoundMatchManager->SelectedUnit)
	{
		FString UnitType = FoundMatchManager->SelectedUnit->GetName().Contains(TEXT("Sniper")) ? TEXT("S") : TEXT("B");
		FString WaitLog = FString::Printf(TEXT("[TU] %s: Resta in attesa"), *UnitType);

		if (FoundMatchManager->ActiveHUD)
		{
			FoundMatchManager->ActiveHUD->AddMoveToHistory(WaitLog);
		}

		AUnitBase* UnitBasePtr = Cast<AUnitBase>(FoundMatchManager->SelectedUnit);
		if (UnitBasePtr)
		{
			UnitBasePtr->bHasMovedThisTurn = true;
			UnitBasePtr->bHasAttackedThisTurn = true;
		}

		FoundMatchManager->ClearHighlights();
		FoundMatchManager->SelectedUnit = nullptr;
		FoundMatchManager->CheckEndTurn();
	}
}