#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "CameraPawn.generated.h"

UCLASS()
class PAA_VIDEOGAME2026_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACameraPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* TopDownCamera;
};