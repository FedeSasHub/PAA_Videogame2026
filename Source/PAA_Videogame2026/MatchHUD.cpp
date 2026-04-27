// matchhud gestisce l'interfaccia a schermo del giocatore. 
// si occupa di aggiornare i testi, i contatori vitali e lo storico mosse durante la partita.

#include "MatchHUD.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

// aggiorna il testo in alto al centro che indica di chi e' il turno
void UMatchHUD::SetTurnText(FString Text)
{
	if (TURNO) TURNO->SetText(FText::FromString(Text));
}

// aggiorna il contatore in alto a sinistra con le torri possedute
void UMatchHUD::SetTowersText(int32 Human, int32 AI)
{
	if (TORRI)
	{
		FString Result = FString::Printf(TEXT("TORRI - Giocatore: %d | AI: %d"), Human, AI);
		TORRI->SetText(FText::FromString(Result));
	}
}

// aggiorna la vita di tutte le pedine nel pannello laterale
void UMatchHUD::UpdateGlobalStats(int32 HumanSniperHP, int32 HumanBrawlerHP, int32 AISniperHP, int32 AIBrawlerHP)
{
	if (TXT_VitaMioSniper) TXT_VitaMioSniper->SetText(FText::FromString(FString::Printf(TEXT("[TU] Sniper: %d/20 HP"), HumanSniperHP)));
	if (TXT_VitaMioBrawler) TXT_VitaMioBrawler->SetText(FText::FromString(FString::Printf(TEXT("[TU] Brawler: %d/40 HP"), HumanBrawlerHP)));
	if (TXT_VitaAISniper) TXT_VitaAISniper->SetText(FText::FromString(FString::Printf(TEXT("[AI] Sniper: %d/20 HP"), AISniperHP)));
	if (TXT_VitaAIBrawler) TXT_VitaAIBrawler->SetText(FText::FromString(FString::Printf(TEXT("[AI] Brawler: %d/40 HP"), AIBrawlerHP)));
}

// aggiunge una nuova riga colorata allo storico delle mosse e scala sotto le altre
void UMatchHUD::AddMoveToHistory(FString NewMove)
{
	if (STORICO_MOSSE)
	{
		FString FormattedMove = NewMove;
		if (NewMove.Contains(TEXT("[TU]")))
		{
			FormattedMove = FString::Printf(TEXT("<Tu>%s</>"), *NewMove);
		}
		else if (NewMove.Contains(TEXT("[AI]")))
		{
			FormattedMove = FString::Printf(TEXT("<Ai>%s</>"), *NewMove);
		}

		FString CurrentText = STORICO_MOSSE->GetText().ToString();
		FString NewText = CurrentText.IsEmpty() ? FormattedMove : FormattedMove + TEXT("\n") + CurrentText;

		STORICO_MOSSE->SetText(FText::FromString(NewText));
	}
}

// trasforma il turno in countdown e mostra l'esito a fine partita
void UMatchHUD::ShowFinalMessage(FString Message, int32 SecondsRemaining)
{
	if (TURNO)
	{
		FString CountdownText = FString::Printf(TEXT("Reset in %d..."), SecondsRemaining);
		TURNO->SetText(FText::FromString(CountdownText));
	}

	if (TXT_MessaggioCentrale)
	{
		TXT_MessaggioCentrale->SetText(FText::FromString(Message));
		TXT_MessaggioCentrale->SetVisibility(ESlateVisibility::Visible);

		FLinearColor TextColor = Message.Contains(TEXT("VITTORIA")) ? FLinearColor(1.f, 0.8f, 0.1f) : FLinearColor(0.5f, 0.f, 0.f);
		TXT_MessaggioCentrale->SetColorAndOpacity(FSlateColor(TextColor));
	}
}