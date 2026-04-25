#include "Tower.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ATower::ATower()
{
	PrimaryActorTick.bCanEverTick = false;

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	RootComponent = TowerMesh;
}

void ATower::SetupTower(int32 X, int32 Y)
{
	GridPosition = FVector2D(X, Y);
	UE_LOG(LogTemp, Warning, TEXT("Torre piazzata in X:%d Y:%d"), X, Y);
}

void ATower::UpdateVisuals(FLinearColor Color)
{
	if (!TowerMesh) return;

	// Cerchiamo di usare il materiale base se assegnato, altrimenti prendiamo quello di default
	UMaterialInterface* MatToUse = BaseTowerMaterial ? BaseTowerMaterial : TowerMesh->GetMaterial(0);

	if (MatToUse)
	{
		UMaterialInstanceDynamic* DynamicMat = TowerMesh->CreateDynamicMaterialInstance(0, MatToUse);
		if (DynamicMat)
		{
			// NOTA: Nel Materiale della tua torre in Unreal, 
			// ricordati di chiamare il nodo del colore "BodyColor" come per i soldati!
			DynamicMat->SetVectorParameterValue(FName("BodyColor"), Color);
			TowerMesh->SetMaterial(0, DynamicMat);
		}
	}
}