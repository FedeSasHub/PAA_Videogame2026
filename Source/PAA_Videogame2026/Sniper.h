#pragma once

#include "CoreMinimal.h"
#include "UnitBase.h"
#include "Sniper.generated.h"

UCLASS()
class PAA_VIDEOGAME2026_API ASniper : public AUnitBase
{
	GENERATED_BODY()

public:
	ASniper();

	// Polimorfismo: modifichiamo il comportamento della funzione base
	virtual void PrintUnitStatus() override;
};