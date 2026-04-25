#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PAAPlayerController.generated.h"

UCLASS()
class PAA_VIDEOGAME2026_API APAAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APAAPlayerController();

protected:
	// Questa funzione serve a "leggere" i tasti premuti (SetupInput)
	virtual void SetupInputComponent() override;

	// Questa è la funzione che scatta quando premi il tasto sinistro
	void OnLeftClick();
	void OnRightClick();
	void OnWaitPressed();
	virtual void BeginPlay() override;
};