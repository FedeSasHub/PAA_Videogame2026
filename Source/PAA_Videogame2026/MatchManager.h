#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MatchManager.generated.h"

class ASniper;
class ABrawler;
class AUnitBase;
class AGridCell;
class UMatchHUD; // Pre-dichiarazione per l'HUD

UENUM(BlueprintType)
enum class EMatchPhase : uint8 { CoinFlip, Deployment, Playing, GameOver };

UENUM(BlueprintType)
enum class EPlayerTurn : uint8 { HumanPlayer, AIPlayer };

UCLASS()
class PAA_VIDEOGAME2026_API AMatchManager : public AActor
{
	GENERATED_BODY()

public:
	AMatchManager();
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State")
	EMatchPhase CurrentPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State")
	EPlayerTurn CurrentTurn;

	UPROPERTY(BlueprintReadOnly, Category = "Match State")
	EPlayerTurn FirstPlayer;

	UPROPERTY(BlueprintReadOnly, Category = "Match State")
	AActor* SelectedUnit = nullptr;

	// --- FUNZIONI DI GIOCO ---
	void FlipCoin();
	void OnCellClicked(int32 X, int32 Y, int32 Elevation);
	void OnUnitClicked(AActor* ClickedUnit);
	void HandleRightClick(int32 X, int32 Y);
	void ClearHighlights();
	void CheckEndTurn();
	void CheckGameOver();
	FString GetChessCoordinate(int32 X, int32 Y);
	void EvaluateTowers();
	void StartNewTurn(EPlayerTurn NewTurn);
	void ExecuteMovement(AUnitBase* Unit, int32 TargetX, int32 TargetY, AGridCell* TargetCell);
	void ExecuteAttack(AUnitBase* Attacker, AUnitBase* Defender);
	void AggiornaViteHUD();
	void DeploySingleAIUnit();
	void ExecuteAITurn();

	// --- STATISTICHE PER UI ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State")
	int32 HumanTowersControlled = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State")
	int32 AITowersControlled = 0;

	// --- SETUP ---
	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<ASniper> SniperClass;

	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<ABrawler> BrawlerClass;

	UPROPERTY(EditAnywhere, Category = "Setup")
	class UMaterialInterface* BaseUnitMaterial;

	// --- UI SYSTEM ---
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMatchHUD> HUDClass;

	UPROPERTY()
	UMatchHUD* ActiveHUD;
protected:
	int32 CountdownReset = 10;
	FTimerHandle TimerHandle_Reset;

	void EseguiCountdown();
	void RestartGame();
private:
	int32 PlayerUnitsDeployed = 0;
	int32 AIUnitsDeployed = 0;
	int32 HumanConsecutiveTurns = 0;
	int32 AIConsecutiveTurns = 0;

	TArray<AGridCell*> ValidMoveCells;
	TArray<AGridCell*> ValidAttackCells;

	void SetUnitColor(AActor* Unit, FLinearColor Color);
};