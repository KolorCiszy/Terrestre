#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FastNoiseSettings.h"
#include "TerrainSurfaceDecoratorSettings.generated.h"


UCLASS(Config = Game)
class TERRESTRE_API UTerrainSurfaceDecoratorSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Surface Decorator Settings")
    uint8 DirtBlocksBelowGrass = 3;

    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Surface Decorator Settings")
    uint8 SandBlocksBelowSeaLevel = 5;
};