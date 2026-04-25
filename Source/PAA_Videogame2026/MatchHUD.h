#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchHUD.generated.h"

class UTextBlock;

UCLASS()
class PAA_VIDEOGAME2026_API UMatchHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// I nomi devono essere IDENTICI a quelli nel Blueprint
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TURNO;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TORRI;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* STATISTICHE;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* STORICO_MOSSE;

	void AggiungiMossa(FString NuovaMossa);

	// Funzioni per cambiare i testi da C++
	void SetTurnoText(FString Testo);
	void SetTorriText(int32 Umano, int32 AI);
	void SetStatisticheText(FString Nome, int32 HP, int32 MaxHP);
};