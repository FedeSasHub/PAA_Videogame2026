#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchHUD.generated.h"

class UTextBlock;
class URichTextBlock; // <--- AGGIUNTO PER IL TESTO COLORATO

UCLASS()
class PAA_VIDEOGAME2026_API UMatchHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// I nomi originali
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TURNO;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TORRI;

	// <--- TRASFORMATO IN RICH TEXT BLOCK
	UPROPERTY(meta = (BindWidget))
	URichTextBlock* STORICO_MOSSE;

	// --- STATISTICHE GLOBALI ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaMioSniper;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaMioBrawler;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaAISniper;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_VitaAIBrawler;
	// ------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TXT_MessaggioCentrale;
	void AggiungiMossa(FString NuovaMossa);

	// Funzioni per cambiare i testi da C++
	void SetTurnoText(FString Testo);
	void SetTorriText(int32 Umano, int32 AI);
	void MostraMessaggioFinale(FString Messaggio, int32 SecondiRimanenti);
	// Nuova funzione per aggiornare tutte e 4 le vite in un colpo solo
	
	void AggiornaStatisticheGlobali(int32 VitaMS, int32 VitaMB, int32 VitaAS, int32 VitaAB);
};