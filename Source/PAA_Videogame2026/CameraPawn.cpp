#include "CameraPawn.h"

ACameraPawn::ACameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// Creiamo la telecamera e la impostiamo come radice dell'attore
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	RootComponent = TopDownCamera;

	// Impostiamo la proiezione in Ortografica (2D puro) 
	TopDownCamera->ProjectionMode = ECameraProjectionMode::Orthographic;

	// Impostiamo la larghezza della visuale per farci stare tutta la griglia 25x25 
	// 25 celle * 100 cm = 2500. Mettiamo 3500 per avere un po' di margine ai bordi.
	TopDownCamera->OrthoWidth = 3500.0f;

	// Ruotiamo la telecamera di -90 gradi sull'asse Y (Pitch) per farla guardare dritta in basso
	TopDownCamera->SetWorldRotation(FRotator(-90.0f, 90.0f, 0.0f));
}