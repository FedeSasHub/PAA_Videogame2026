#include "Sniper.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ASniper::ASniper()
{
    // Statistiche PDF
    MovementRange = 4;
    AttackRange = 10;
    HealthPoints = 20;
    MinDamage = 4;
    MaxDamage = 8;

    // Parte Visiva
    UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    RootComponent = Mesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylinderAsset.Succeeded())
    {
        Mesh->SetStaticMesh(CylinderAsset.Object);
    }
    Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f));
}

void ASniper::PrintUnitStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Sniper schierato! HP: %d, Danno: %d-%d, Movimento: %d"), HealthPoints, MinDamage, MaxDamage, MovementRange);
}