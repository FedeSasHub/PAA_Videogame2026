#pragma once

#include "CoreMinimal.h"
#include "UnitBase.h"
#include "Brawler.generated.h"

UCLASS()
class PAA_VIDEOGAME2026_API ABrawler : public AUnitBase
{
	GENERATED_BODY()

public:
	ABrawler();

	virtual void PrintUnitStatus() override;
};