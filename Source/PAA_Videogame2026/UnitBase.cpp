#include "UnitBase.h"

AUnitBase::AUnitBase()
{
	// Disabilitiamo il Tick perche le unita in uno strategico a turni 
	// non hanno bisogno di aggiornarsi ogni singolo frame. Ottimizza il gioco.
	PrimaryActorTick.bCanEverTick = false;
}

void AUnitBase::BeginPlay()
{
	Super::BeginPlay();
	PrintUnitStatus(); // Appena l'unita appare nel mondo, stampa le sue info
}

void AUnitBase::PrintUnitStatus()
{
	// Log di test per vedere se funziona
	UE_LOG(LogTemp, Warning, TEXT("Spawnata unita base generica."));
}