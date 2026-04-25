#include "MatchHUD.h"
#include "Components/TextBlock.h"

void UMatchHUD::SetTurnoText(FString Testo)
{
	if (TURNO) TURNO->SetText(FText::FromString(Testo));
}

void UMatchHUD::SetTorriText(int32 Umano, int32 AI)
{
	if (TORRI)
	{
		FString Risultato = FString::Printf(TEXT("TORRI - Umano: %d | AI: %d"), Umano, AI);
		TORRI->SetText(FText::FromString(Risultato));
	}
}

void UMatchHUD::SetStatisticheText(FString Nome, int32 HP, int32 MaxHP)
{
	if (STATISTICHE)
	{
		FString Risultato = FString::Printf(TEXT("%s\nHP: %d/%d"), *Nome, HP, MaxHP);
		STATISTICHE->SetText(FText::FromString(Risultato));
	}
}