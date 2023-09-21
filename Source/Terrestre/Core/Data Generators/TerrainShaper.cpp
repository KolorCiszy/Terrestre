#include "TerrainShaper.h"
#include "FastNoiseWrapper.h"
#include "Terrestre/Core/Chunk/Storage/ChunkData.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "DataRegistry/Public/DataRegistrySubsystem.h"

#pragma optimize("t", on)

UTerrainShaper::UTerrainShaper()
{
}
void UTerrainShaper::BeginDestroy()
{
	Super::BeginDestroy();
	//DensityNoise->ConditionalBeginDestroy();
	//ContinentalnessNoise->ConditionalBeginDestroy();
}

void UTerrainShaper::Initialize()
{
	DensityNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass()); //RF_MarkAsRootSet);
	ContinentalnessNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass()); //RF_MarkAsRootSet);
	DensityNoise->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.type,
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.seed, 
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.frequency, 
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.interpolation,
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.fractalType, 
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.octaves, 
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.lacunarity,
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.gain, 
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.cellularJitter, 
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.celDistFunc, 
			UChunkUtilityLib::GetChunkManager()->terrainShaperDensityNoise.celReturnType);

	ContinentalnessNoise->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.type,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.seed,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.frequency,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.interpolation,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.fractalType,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.octaves,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.lacunarity,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.gain,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->terrainShaperContinentalnessNoise.celReturnType);
	/*
	DensityNoise->SetupFastNoise(EFastNoise_NoiseType::Perlin, 6969, 0.01f, 
								EFastNoise_Interp::Quintic, EFastNoise_FractalType::FBM, 3, 0.4f, 2.0f, .35f, 
								EFastNoise_CellularDistanceFunction::Natural);
	ContinentalnessNoise->SetupFastNoise(EFastNoise_NoiseType::Perlin, 6969, 0.002f,
										EFastNoise_Interp::Quintic, EFastNoise_FractalType::FBM, 3, 0.6f, 0.4f, .45f, 
										EFastNoise_CellularDistanceFunction::Natural);
										*/

}
float UTerrainShaper::GetContinentalnessAtXY_Native(double X, double Y)
{
	
    float output = ContinentalnessNoise->GetNoise2D(X, Y);
	return UChunkUtilityLib::SetFloatPrecision(output * 2, 3) / 2;

}
float UTerrainShaper::GetContinentalnessAtXY_BP(double X, double Y)
{
	return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetContinentalnessAtXY_Native(X, Y);
}

int32 UTerrainShaper::GetTerrainHeightAtXY_Native(double X, double Y)
{
	float continentalness = GetContinentalnessAtXY_Native(X, Y);

	float continentalnessCurveValue{};
	EDataRegistrySubsystemGetItemResult result{};
	UDataRegistrySubsystem::EvaluateDataRegistryCurve(ContinentalnessCurveID, continentalness, .0f, result, continentalnessCurveValue);
	return continentalnessCurveValue + SEA_LEVEL;
}
int32 UTerrainShaper::GetTerrainHeightAtXY_BP(double X, double Y)
{

	return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetTerrainHeightAtXY_Native(X, Y);
}

int32 UTerrainShaper::GetSurfaceHeightAtXY_BP(double X, double Y)
{
	
	int32 terrainHeight = UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetTerrainHeightAtXY_Native(X, Y);
	float currentDensity = UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetDensityAtXYZ_Native(X, Y, terrainHeight);

	float previousDensity = currentDensity;
	
	if(FMath::IsNearlyEqual(currentDensity, 0, 0.0001))
	{
		return terrainHeight;
	}

	bool bFound{};

	int32 loopCount{};
	while (!bFound)
	{
		if (currentDensity > 0)
		{
			currentDensity = UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetDensityAtXYZ_Native(X, Y, ++terrainHeight);
		}
		else
		{
			currentDensity = UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetDensityAtXYZ_Native(X, Y, --terrainHeight);
		}
		if (FMath::Sign(currentDensity) != FMath::Sign(previousDensity))
		{
			bFound = true;
			break;
		}
		if (loopCount++ > 400)
		{
			return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetTerrainHeightAtXY_Native(X, Y);
		}
		previousDensity = currentDensity;

	}
	return terrainHeight;
}
float UTerrainShaper::GetDensityAtXYZ_Native(double worldXPos, double worldYPos, double worldZPos)
{
	int32 terrainHeight = GetTerrainHeightAtXY_Native(worldXPos, worldYPos);
	float density = UChunkUtilityLib::GetChunkManager()->TerrainShaper->DensityNoise->GetNoise3D(worldXPos, worldYPos, worldZPos);
	int32 heightDifference = terrainHeight - worldZPos;
	density += (SQUEEZE_FACTOR * heightDifference);
	return density;
}
float UTerrainShaper::GetDensityAtXYZ_BP(double worldXPos, double worldYPos, double worldZPos)
{
	return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetDensityAtXYZ_Native(worldXPos, worldYPos, worldZPos);
}

void UTerrainShaper::GenerateTerrainShape(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& chunkBlocks, 
											TArray<FFluidState, TInlineAllocator<AChunk::Volume>>& fluidStates,
											FVector chunkLocation, bool& bDensitySignChange, TArray<int16>& heightMap)
{
	
	auto terrainShaper = UChunkUtilityLib::GetChunkManager()->TerrainShaper;
	
	chunkLocation /= AChunk::VoxelSize;
	auto densityNoise = UChunkUtilityLib::GetChunkManager()->TerrainShaper->DensityNoise;
	float previousDensity = terrainShaper->GetDensityAtXYZ_Native(chunkLocation.X, chunkLocation.Y, chunkLocation.Z);
	
	for (uint8 x = 0; x < AChunk::Size; x++)
	{
		int32 worldXPos = chunkLocation.X + x;
		for (uint8 y = 0; y < AChunk::Size; y++)
		{
			
			int32 worldYPos = chunkLocation.Y + y;
			int32 terrainHeight = terrainShaper->GetTerrainHeightAtXY_Native(worldXPos, worldYPos);
			for (uint8 z = 0; z < AChunk::Size; z++)
			{
				int32 index = UChunkUtilityLib::LocalBlockPosToIndex({ x,y,z });
				int32 worldZPos = chunkLocation.Z + z;
				if (z == 0)
				{
					previousDensity = terrainShaper->GetDensityAtXYZ_Native(worldXPos, worldYPos, worldZPos - 1);
				}
				float density = densityNoise->GetNoise3D(worldXPos, worldYPos, worldZPos);
				
				int32 heightDifference = terrainHeight - worldZPos;
				density += (SQUEEZE_FACTOR * heightDifference);

				if(density <= 0)
				{
					*(chunkBlocks.GetData() + index) = FBlockState(0);
					if (density * previousDensity < 0.0f)
					{
						*(chunkBlocks.GetData() + index) = FBlockState(2);
						heightMap[index - z * AChunk::SizeSquared] = worldZPos;
						bDensitySignChange = true;
					}
					if (worldZPos <= SEA_LEVEL)
					{
						*(fluidStates.GetData() + index) = FFluidState(1 , 100);

					}
				}
				else
				{
					float densityHigher = densityNoise->GetNoise3D(worldXPos, worldYPos, worldZPos + 1);
					
					float densityHigher2 = densityNoise->GetNoise3D(worldXPos, worldYPos, worldZPos + 2);
					float densityHigher3 = densityNoise->GetNoise3D(worldXPos, worldYPos, worldZPos + 3);
					densityHigher += (SQUEEZE_FACTOR * (heightDifference - 1));
					densityHigher2 += (SQUEEZE_FACTOR * (heightDifference - 2));
					densityHigher3 += (SQUEEZE_FACTOR * (heightDifference - 3));
					if (densityHigher < 0 || densityHigher2 < 0 || densityHigher3 < 0)
					{
						*(chunkBlocks.GetData() + index) = FBlockState(3);
					}
					else
					{
						*(chunkBlocks.GetData() + index) = FBlockState(1);
					}
					*(fluidStates.GetData() + index) = FFluidState(UINT8_MAX, 100);
					
				}
				
				previousDensity = density;
			}
			
		}
	}
}