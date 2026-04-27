#include "MatchHUD.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h" // <--- AGGIUNTO PER IL TESTO COLORATO

void UMatchHUD::SetTurnoText(FString Testo)
{
	if (TURNO) TURNO->SetText(FText::FromString(Testo));
}

void UMatchHUD::SetTorriText(int32 Umano, int32 AI)
{
	if (TORRI)
	{
		FString Risultato = FString::Printf(TEXT("TORRI - Giocatore: %d | AI: %d"), Umano, AI);
		TORRI->SetText(FText::FromString(Risultato));
	}
}

void UMatchHUD::AggiungiMossa(FString NuovaMossa)
{
	if (STORICO_MOSSE)
	{
		// 1. Aggiungiamo i tag HTML-style in base a chi ha fatto la mossa
		FString MossaFormattata = NuovaMossa;
		if (NuovaMossa.Contains(TEXT("[TU]")))
		{
			MossaFormattata = FString::Printf(TEXT("<Tu>%s</>"), *NuovaMossa);
		}
		else if (NuovaMossa.Contains(TEXT("[AI]")))
		{
			MossaFormattata = FString::Printf(TEXT("<Ai>%s</>"), *NuovaMossa);
		}

		// 2. Uniamo al testo esistente
		FString TestoAttuale = STORICO_MOSSE->GetText().ToString();
		FString NuovoTesto = TestoAttuale.IsEmpty() ? MossaFormattata : MossaFormattata + TEXT("\n") + TestoAttuale;

		STORICO_MOSSE->SetText(FText::FromString(NuovoTesto));
	}
}

// --- NUOVA FUNZIONE: Aggiorna le vite globali ---
void UMatchHUD::AggiornaStatisticheGlobali(int32 VitaMS, int32 VitaMB, int32 VitaAS, int32 VitaAB)
{
	if (TXT_VitaMioSniper) TXT_VitaMioSniper->SetText(FText::FromString(FString::Printf(TEXT("[TU] Sniper: %d/20 HP"), VitaMS)));
	if (TXT_VitaMioBrawler) TXT_VitaMioBrawler->SetText(FText::FromString(FString::Printf(TEXT("[TU] Brawler: %d/40 HP"), VitaMB)));
	if (TXT_VitaAISniper) TXT_VitaAISniper->SetText(FText::FromString(FString::Printf(TEXT("[AI] Sniper: %d/20 HP"), VitaAS)));
	if (TXT_VitaAIBrawler) TXT_VitaAIBrawler->SetText(FText::FromString(FString::Printf(TEXT("[AI] Brawler: %d/40 HP"), VitaAB)));
}
void UMatchHUD::MostraMessaggioFinale(FString Messaggio, int32 SecondiRimanenti)
{
	// Aggiorniamo il testo del turno come countdown (piccolo in alto)
	if (TURNO)
	{
		FString TestoCountdown = FString::Printf(TEXT("Reset in %d..."), SecondiRimanenti);
		TURNO->SetText(FText::FromString(TestoCountdown));
	}

	// Mostriamo il messaggio GIGANTE al centro
	if (TXT_MessaggioCentrale)
	{
		TXT_MessaggioCentrale->SetText(FText::FromString(Messaggio));
		TXT_MessaggioCentrale->SetVisibility(ESlateVisibility::Visible);

		// Colore dinamico: Oro per vittoria, Rosso scuro per sconfitta
		FLinearColor ColoreTesto = Messaggio.Contains(TEXT("VITTORIA")) ? FLinearColor(1.f, 0.8f, 0.1f) : FLinearColor(0.5f, 0.f, 0.f);
		TXT_MessaggioCentrale->SetColorAndOpacity(FSlateColor(ColoreTesto));
	}
}