#include "Brawler.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// imposta le statistiche base e la mesh 3d a forma di cubo per il brawler
ABrawler::ABrawler()
{
	MovementRange = 6;
	AttackRange = 1;
	HealthPoints = 40;
	MinDamage = 1;
	MaxDamage = 6;

	UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeAsset.Succeeded())
	{
		Mesh->SetStaticMesh(CubeAsset.Object);
	}
	Mesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
}

// stampa in console le statistiche specifiche di questa unita 
void ABrawler::PrintUnitStatus()
{
	UE_LOG(LogTemp, Warning, TEXT("Brawler schierato! HP: %d, Danno: %d-%d, Movimento: %d"), HealthPoints, MinDamage, MaxDamage, MovementRange);
}