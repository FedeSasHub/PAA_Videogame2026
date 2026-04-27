#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchHUD.generated.h"

class UTextBlock;
class URichTextBlock;

UCLASS()
class PAA_VIDEOGAME2026_API UMatchHUD : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TURNO;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TORRI;

	UPROPERTY(meta = (BindWidget))
	URichTextBlock* STORICO_MOSSE;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaMioSniper;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaMioBrawler;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaAISniper;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaAIBrawler;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_MessaggioCentrale;

	void SetTurnText(FString Text);
	void SetTowersText(int32 Human, int32 AI);
	void UpdateGlobalStats(int32 HumanSniperHP, int32 HumanBrawlerHP, int32 AISniperHP, int32 AIBrawlerHP);
	void AddMoveToHistory(FString NewMove);
	void ShowFinalMessage(FString Message, int32 SecondsRemaining);
};