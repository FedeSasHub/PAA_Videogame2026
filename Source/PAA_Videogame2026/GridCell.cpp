#include "GridCell.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AGridCell::AGridCell()
{
	PrimaryActorTick.bCanEverTick = false;
	CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
	RootComponent = CellMesh;

	// Portiamo la scala a 0.9 per un gap molto più netto
	CellMesh->SetRelativeScale3D(FVector(0.95f, 0.95f, 1.0f));

	CellMesh->OnClicked.AddDynamic(this, &AGridCell::OnCellClicked);
}

void AGridCell::SetupCell(int32 X, int32 Y, int32 Elevation)
{
	GridPosition = FVector2D(X, Y);
	ElevationLevel = Elevation;

	// Se abbiamo inserito i materiali e l'indice è valido, coloriamo il cubo!
	if (ElevationMaterials.IsValidIndex(ElevationLevel) && CellMesh)
	{
		CellMesh->SetMaterial(0, ElevationMaterials[ElevationLevel]);
	}

}
void AGridCell::OnCellClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Warning, TEXT("Hai cliccato la cella: X:%d Y:%d (Elevazione: %d)"), (int32)GridPosition.X, (int32)GridPosition.Y, ElevationLevel);
}
void AGridCell::HighlightCell(bool bEnable) // GIALLO (Movimento)
{
	if (!CellMesh) return;
	if (bEnable) {
		if (!OriginalMaterial) OriginalMaterial = CellMesh->GetMaterial(0);
		if (MoveHighlightMaterial) CellMesh->SetMaterial(0, MoveHighlightMaterial);
	}
	else {
		if (OriginalMaterial) CellMesh->SetMaterial(0, OriginalMaterial);
	}
}

void AGridCell::HighlightAttackCell(bool bHighlight) // ROSSO (Attacco)
{
	if (!CellMesh) return;
	if (bHighlight) {
		if (!OriginalMaterial) OriginalMaterial = CellMesh->GetMaterial(0);
		if (AttackHighlightMaterial) CellMesh->SetMaterial(0, AttackHighlightMaterial);
	}
	else {
		if (OriginalMaterial) CellMesh->SetMaterial(0, OriginalMaterial);
	}
}

void AGridCell::HighlightHybridCell(bool bHighlight) // ARANCIONE (Ibrido)
{
	if (!CellMesh) return;
	if (bHighlight) {
		if (!OriginalMaterial) OriginalMaterial = CellMesh->GetMaterial(0);
		if (HybridHighlightMaterial) CellMesh->SetMaterial(0, HybridHighlightMaterial);
	}
	else {
		if (OriginalMaterial) CellMesh->SetMaterial(0, OriginalMaterial);
	}
}