#include "UnitBase.h"

// inizializza la classe
AUnitBase::AUnitBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

// esegue le operazioni di base
void AUnitBase::BeginPlay()
{
	Super::BeginPlay();
	PrintUnitStatus();
}

// stampa in console i dati generici dell'unita 
void AUnitBase::PrintUnitStatus()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawnata unita base generica."));
}