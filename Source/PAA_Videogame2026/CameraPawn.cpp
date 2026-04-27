#include "CameraPawn.h"

// imposta la telecamera dall'alto per inquadrare tutta la mappa
ACameraPawn::ACameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	RootComponent = TopDownCamera;

	TopDownCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	TopDownCamera->OrthoWidth = 3000.0f;
	TopDownCamera->SetWorldRotation(FRotator(-90.0f, 90.0f, 0.0f));
}