#pragma once

#include "CoreMinimal.h"
#include "FastNoiseSettings.h"
#include "DataRegistry.h"
#include "TerrainShaper.generated.h"

class UFastNoiseWrapper;
class USplineComponent;
struct FBlockState;


UE_ENABLE_OPTIMIZATION


UCLASS()
class UTerrainShaper : public UObject
{
	GENERATED_BODY()
public:
	
	UTerrainShaper();
	virtual void BeginDestroy() override;
	static inline float SEA_LEVEL = 100.0f;
	
	static inline float const MAX_TERRAIN_HEIGHT = 100.f;
	
	static inline float const SQUEEZE_FACTOR = 0.012f;
	/* Generates Terrain Shape for given chunk location, outputing a temp heightmap for further generator use */
	static void GenerateTerrainShape(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& chunkBlocks,
									TArray<FFluidState, TInlineAllocator<AChunk::Volume>>& fluidStates, 
									FVector chunkLocation, bool& bDensitySignChange, TArray<int16>& heightMap);
	
	void Initialize();

	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline float GetContinentalnessAtXY_BP(double X, double Y);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline int32 GetTerrainHeightAtXY_BP(double X, double Y);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline int32 GetSurfaceHeightAtXY_BP(double X, double Y);
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static inline float GetDensityAtXYZ_BP(double worldXPos, double worldYPos, double worldZPos);


	FORCEINLINE 
	float GetDensityAtXYZ_Native(double worldXPos, double worldYPos, double worldZPos);
	FORCEINLINE
	float GetContinentalnessAtXY_Native(double X, double Y);
	
	int32 GetTerrainHeightAtXY_Native(double X, double Y);
	UPROPERTY()
	UFastNoiseWrapper* DensityNoise;
	UPROPERTY()
	UFastNoiseWrapper* ContinentalnessNoise;

	static inline FDataRegistryId ContinentalnessCurveID = FDataRegistryId(L"TerrainGenerationCurves", L"Continentalness");
};