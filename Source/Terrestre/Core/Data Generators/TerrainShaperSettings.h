#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FastNoiseSettings.h"
#include "TerrainShaperSettings.generated.h"


UCLASS(Config=Game)
class TERRESTRE_API UTerrainShaperSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings Denstity0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings Denstity1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings Denstity2;

    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings Continentalness0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings Continentalness1;

    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings Erosion;

    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings PV;

    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Noise settings")
    FFastNoiseSettings Weirdness;

    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Terrain Settings")
    float BaseSqueezeFactor;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Terrain Settings")
    float MaxSqueezeFactor;

    UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Terrain Settings")
    float SeaLevel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Sources")
    FDataRegistryId ContTHCurveID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Sources")
    FDataRegistryId ErosTHCurveID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Sources")
    FDataRegistryId PVTHCurveID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Sources")
    FDataRegistryId ContSFCurveID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Sources")
    FDataRegistryId ErosSFCurveID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Sources")
    FDataRegistryId PVSFCurveID;

};