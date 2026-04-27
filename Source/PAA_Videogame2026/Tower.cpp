#include "Tower.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// inizializza il componente visivo della torre 
ATower::ATower()
{
	PrimaryActorTick.bCanEverTick = false;

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	RootComponent = TowerMesh;
}

// salva le coordinate della griglia all'interno della torre
void ATower::SetupTower(int32 X, int32 Y)
{
	GridPosition = FVector2D(X, Y);
	UE_LOG(LogTemp, Warning, TEXT("Tower placed at X:%d Y:%d"), X, Y);
}

// crea un'istanza dinamica del materiale per colorare la torre in base al proprietario (blu per il giocatore umano, rosso per l'IA, grigio quando è neutrale)
void ATower::UpdateVisuals(FLinearColor Color)
{
	if (!TowerMesh) return;

	UMaterialInterface* MatToUse = BaseTowerMaterial ? BaseTowerMaterial : TowerMesh->GetMaterial(0);

	if (MatToUse)
	{
		UMaterialInstanceDynamic* DynamicMat = TowerMesh->CreateDynamicMaterialInstance(0, MatToUse);
		if (DynamicMat)
		{
			DynamicMat->SetVectorParameterValue(FName("BodyColor"), Color);
			TowerMesh->SetMaterial(0, DynamicMat);
		}
	}
}