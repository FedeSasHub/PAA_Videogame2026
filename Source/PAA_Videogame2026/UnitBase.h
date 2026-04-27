#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitBase.generated.h"

UCLASS()
class PAA_VIDEOGAME2026_API AUnitBase : public AActor
{
	GENERATED_BODY()

public:
	AUnitBase();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 MovementRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 HealthPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Stats")
	int32 MaxHealthPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 MinDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
	int32 MaxDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Position")
	int32 GridX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Position")
	int32 GridY;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Position")
	int32 OriginalGridX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Position")
	int32 OriginalGridY;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turn State")
	bool bHasMovedThisTurn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turn State")
	bool bHasAttackedThisTurn = false;

	virtual void PrintUnitStatus();
};