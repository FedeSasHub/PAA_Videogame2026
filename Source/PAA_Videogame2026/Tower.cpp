#include "Tower.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"


ATower::ATower()
{
	PrimaryActorTick.bCanEverTick = false;

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	RootComponent = TowerMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderAsset.Succeeded())
	{
		TowerMesh->SetStaticMesh(CylinderAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (DefaultMat.Succeeded())
	{
		BaseTowerMaterial = DefaultMat.Object;
	}

	TowerMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 2.0f));

	CurrentState = ETowerState::Neutral;
	CurrentOwner = ETowerOwner::None;
}

// salva le coordinate della torre all'interno della griglia di gioco
void ATower::SetupTower(int32 X, int32 Y)
{
	GridPosition = FVector2D(X, Y);
}

// aggiorna dinamicamente il colore della torre in base alla fazione che la controlla
void ATower::UpdateVisuals(FLinearColor Color)
{
	if (!TowerMesh) return;

	if (!DynamicTowerMat)
	{
		UMaterialInterface* MatToUse = BaseTowerMaterial ? BaseTowerMaterial : TowerMesh->GetMaterial(0);
		if (MatToUse)
		{
			DynamicTowerMat = TowerMesh->CreateDynamicMaterialInstance(0, MatToUse);
		}
	}

	if (DynamicTowerMat)
	{
		DynamicTowerMat->SetVectorParameterValue(FName("BodyColor"), Color);
	}
}