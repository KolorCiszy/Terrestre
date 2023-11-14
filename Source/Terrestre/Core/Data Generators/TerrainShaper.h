#pragma once

#include "CoreMinimal.h"
#include "FastNoiseSettings.h"
#include "DataRegistry.h"
#include "DataRegistry/Public/DataRegistrySubsystem.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "TerrainShaperSettings.h"
#include "RegionGenerationData.h"
#include "TerrainShaper.generated.h"

class UFastNoiseWrapper;
struct FBlockState;


UENUM()
enum class EContinentsIndex : uint8
{
	DEEP_OCEAN, // -1.0f ~ -0.45f
	OCEAN, // -0.45f ~ -0.2f
	COAST, // -0.2f ~ -0.1f
	NEAR_INLAND, // -0.1f ~ 0.05f
	MID_INLAND, // 0.05f ~ 0.3f
	FAR_INLAND, // 0.3f ~ 0.7f
	EXTREMLY_INLAND, // 0.7f ~ 1.0f
	COUNT
};
UENUM()
enum class EErosionIndex : uint8
{
	EXTREME_MOUNTAINS, // -1.0f ~ -0.8f
	MOUNTAINS, // -0.8f ~ -0.375f
	HILLS, // -0.375f ~ -0.25f
	PLAINS, // -0.25f ~ 0.3f
	LOWLANDS, // 0.3f ~ 0.7f
	VALLEY, // 0.7f ~ 0.8f
	CANYON, // 0.8f - 0.9f
	EXTREME_FLATLANDS, // 0.9 ~ 1.0f
	COUNT
};


UCLASS()
class UTerrainShaper : public UObject
{
	GENERATED_BODY()
public:
	
	UTerrainShaper();
	
	void Initialize();

	static UTerrainShaper* Get();

	/* Generates Terrain Shape for given chunk location, outputing a temp heightmap for further generator use */
	void GenerateTerrainShape(FChunkProtoRegion& protoRegion,
								FRegionGenerationData& regionGenData,
								FIntVector regionID);
	
	void GenerateRegionInitialData(FRegionGenerationData& regionData, FIntVector regionID);
	
	UE_NODISCARD int16 GetTerrainHeightAtXY_Native(double worldX, double worldY, float C, float E, float PV, float W = 0.f);

	UE_NODISCARD float GetSqueezeFactorAtXY_Native(double worldX, double worldY, float C, float E, float PV, float W = 0.f);

	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline float GetContinentalnessAtXY_BP(double X, double Y);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline float GetErosionAtXY_BP(double X, double Y);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline int32 GetTerrainHeightAtXY_BP(double X, double Y);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline float GetSFAtXY_BP(double X, double Y);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline float GetInitialDensityAtXYZ_BP(double worldXPos, double worldYPos, double worldZPos);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline float GetFinalDensityAtXYZ_BP(float initialDensity, double worldZPos, int32 terrainHeight, float SF);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static EErosionIndex GetErosionIndex_BP(float erosion);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static EContinentsIndex GetContinentsIndex_BP(float continentalness);

	/* will have to link is somehow to terrain gen curves */ 
	static constexpr EErosionIndex GetErosionIndex(float erosion)
	{
		if(FMath::IsWithinInclusive(erosion, -1.0f, 1.0f))
		{
			if (erosion <= 0.f)
			{
				if (FMath::IsWithinInclusive(erosion, -0.25f, 0.3f))
				{
					return EErosionIndex::PLAINS;
				}
				else if (FMath::IsWithinInclusive(erosion, -0.8f, -0.375f))
				{
					return EErosionIndex::MOUNTAINS;
				}
				else if (FMath::IsWithinInclusive(erosion, -1.0f, -0.8f))
				{
					return EErosionIndex::EXTREME_MOUNTAINS;
				}
				else
				{
					return EErosionIndex::HILLS;
				}
			}
			else 
			{
				if(FMath::IsWithinInclusive(erosion, -0.25f, 0.3f))
				{
					return EErosionIndex::PLAINS;
				}
				else if(FMath::IsWithinInclusive(erosion, 0.3f, 0.7f))
				{
					return EErosionIndex::LOWLANDS;
				}
				else if(FMath::IsWithinInclusive(erosion, 0.7f, 0.8f))
				{
					return EErosionIndex::VALLEY;
				}
				else if(FMath::IsWithinInclusive(erosion, 0.8f, 0.9f))
				{
					return EErosionIndex::CANYON;
				}
				else
				{
					return EErosionIndex::EXTREME_FLATLANDS;
				}
			}

		}
		else
		{
			if(erosion < 0)
			{
				return EErosionIndex::EXTREME_MOUNTAINS;
			}
			else
			{
				return EErosionIndex::EXTREME_FLATLANDS;
			}
		}
	}
	static constexpr EContinentsIndex GetContinentsIndex(float continentalness)
	{
		if(FMath::IsWithinInclusive(continentalness, -1.0f, 1.0f))
		{
			if(continentalness < 0.0f)
			{
				if(FMath::IsWithinInclusive(continentalness , -1.0f, -0.45f))
				{
					return EContinentsIndex::DEEP_OCEAN;
				}
				else if(FMath::IsWithinInclusive(continentalness, -0.45f, -0.2f))
				{
					return EContinentsIndex::OCEAN;
				}
				else if(FMath::IsWithinInclusive(continentalness, -0.2f, -0.1f))
				{
					return EContinentsIndex::COAST;
				}
				else
				{
					return EContinentsIndex::NEAR_INLAND;
				}
			}
			else
			{
				if(FMath::IsWithinInclusive(continentalness, -0.1f, 0.05f))
				{
					return EContinentsIndex::NEAR_INLAND;
				}
				else if(FMath::IsWithinInclusive(continentalness, 0.05f, 0.3f))
				{
					return EContinentsIndex::MID_INLAND;
				}
				else if(FMath::IsWithinInclusive(continentalness, 0.3f, 0.7f))
				{
					return EContinentsIndex::FAR_INLAND;
				}
				else
				{
					return EContinentsIndex::EXTREMLY_INLAND;
				}
			}
		}
		else
		{
			if(continentalness < 0.0f)
			{
				return EContinentsIndex::DEEP_OCEAN;
			}
			else
			{
				return EContinentsIndex::EXTREMLY_INLAND;
			}
		}
	}
	
	 
	UE_NODISCARD float FORCEINLINE GetInitialDensityAtXYZ_Native(double worldXPos, double worldYPos, double worldZPos)
	{
		float densityOffset = DensityNoise1->GetNoise2D(worldXPos, worldYPos);
		float initialDensity = DensityNoise0->GetNoise3D(worldXPos, worldYPos, worldZPos) * 1.65224234f + densityOffset;
		initialDensity < 0 ? initialDensity *= 0.00946 : initialDensity *=  0.0066f;
		return initialDensity;
	}
	UE_NODISCARD float FORCEINLINE GetFinalDensity_Native(float initialDensity, double worldZPos, int16 TH, float SF)
	{
		int32 heightDiff = TH - worldZPos;
		initialDensity = FMath::Floor(initialDensity * 100.0f) * 0.01f;
		initialDensity = initialDensity * 23.1166f - 2.1f;
		if (heightDiff <= 4)
		{
			initialDensity += (4 * SF * heightDiff);
		}
		else
		{
			initialDensity += (SF * heightDiff);
		}
		return initialDensity;
	}

	
	/* Controls how far inland the terrain is, primary driving the terrain height */
	UE_NODISCARD float FORCEINLINE GetContinentalnessAtXY_Native(double X, double Y)
	{
		float weirdness = GetWeirdnessAtXY_Native(X, Y);
		float output = ContinentalnessNoise0->GetNoise2D(X,Y) * 3.0f + weirdness;
		output = FMath::Floor(output * 100.0f) * 0.001f;
		return output;
	}
	/* Controls how flat the terrain is in general */
	UE_NODISCARD float FORCEINLINE GetErosionAtXY_Native(double X, double Y)
	{
		float output = ErosionNoise->GetNoise2D(X, Y) * 3.0f; 
		output = FMath::Floor(output * 100.0f) * 0.001f;
		return output;
	}

	/* Adds random peak and valleys when output value is high or low enough */
	UE_NODISCARD float FORCEINLINE GetPVAtXY_Native(double X, double Y)
	{
		float output = PVNoise->GetNoise2D(X, Y) * -1.5f;
		output = FMath::Floor(output * 100.0f) * 0.001f;
		return output;
	}
	UE_NODISCARD float FORCEINLINE GetWeirdnessAtXY_Native(double X, double Y)
	{
		float output = WeirdnessNoise->GetNoise2D(X, Y);
		output = FMath::Floor(output * 100.0f) * 0.001f;
		return output;
	}
	

private:
	UPROPERTY()
	UFastNoiseWrapper* DensityNoise0;
	UPROPERTY()
	UFastNoiseWrapper* DensityNoise1;
	UPROPERTY()
	UFastNoiseWrapper* DensityNoise2;
	UPROPERTY()
	UFastNoiseWrapper* ContinentalnessNoise0;
	UPROPERTY()
	UFastNoiseWrapper* ContinentalnessNoise1;
	UPROPERTY()
	UFastNoiseWrapper* ErosionNoise;
	UPROPERTY()
	UFastNoiseWrapper* WeirdnessNoise;
	UPROPERTY()
	UFastNoiseWrapper* PVNoise;

	static inline const FRealCurve* ContTHCurve;
	static inline const FRealCurve* ErosTHCurve;
	static inline const FRealCurve* PVTHCurve;
	static inline const FRealCurve* ContSFCurve;
	static inline const FRealCurve* ErosSFCurve;
	static inline const FRealCurve* PVSFCurve;

};
