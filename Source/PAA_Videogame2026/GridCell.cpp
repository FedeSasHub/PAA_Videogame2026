#include "GridCell.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

// imposta la cella e la rimpicciolisce leggermente per creare spazio tra i blocchi
AGridCell::AGridCell()
{
	PrimaryActorTick.bCanEverTick = false;
	CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
	RootComponent = CellMesh;

	CellMesh->SetRelativeScale3D(FVector(0.95f, 0.95f, 1.0f));
	CellMesh->OnClicked.AddDynamic(this, &AGridCell::OnCellClicked);
}

// assegna le coordinate, l'altitudine e il materiale visivo corrispondente alla cella
void AGridCell::SetupCell(int32 X, int32 Y, int32 Elevation)
{
	GridPosition = FVector2D(X, Y);
	ElevationLevel = Elevation;

	if (ElevationMaterials.IsValidIndex(ElevationLevel) && CellMesh)
	{
		CellMesh->SetMaterial(0, ElevationMaterials[ElevationLevel]);
	}
}

// stampa un log in console quando il giocatore clicca fisicamente sul blocco 3d
void AGridCell::OnCellClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Warning, TEXT("Clicked cell: X:%d Y:%d (Elevation: %d)"), (int32)GridPosition.X, (int32)GridPosition.Y, ElevationLevel);
}

// applica o rimuove il materiale giallo che indica le celle calpestabili
void AGridCell::HighlightCell(bool bEnable)
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

// applica o rimuove il materiale rosso che indica le celle attaccabili
void AGridCell::HighlightAttackCell(bool bHighlight)
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

// applica o rimuove il materiale arancione per le celle sia calpestabili che attaccabili
void AGridCell::HighlightHybridCell(bool bHighlight)
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