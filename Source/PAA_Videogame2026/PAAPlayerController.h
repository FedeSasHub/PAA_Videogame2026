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
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;

	void OnLeftClick();
	void OnRightClick();

	UFUNCTION(BlueprintCallable, Category = "Player Actions")
	void OnWaitPressed();
};