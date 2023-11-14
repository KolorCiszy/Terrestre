#include "TerrainShaper.h"
#include "FastNoiseWrapper.h"
#include "Terrestre/Core/Chunk/Storage/ChunkData.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"


UTerrainShaper::UTerrainShaper()
{
}

UTerrainShaper* UTerrainShaper::Get()
{
	return UChunkUtilityLib::GetChunkManager()->TerrainShaper;
}

void UTerrainShaper::Initialize()
{
	DensityNoise0 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	DensityNoise1 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	DensityNoise2 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	ContinentalnessNoise0 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	ContinentalnessNoise1 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	ErosionNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	WeirdnessNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	PVNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());


	DensityNoise0->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.type,
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.seed, 
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.frequency, 
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.interpolation,
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.fractalType, 
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.octaves, 
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.lacunarity,
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.gain, 
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.cellularJitter, 
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.celDistFunc, 
			UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity0.celReturnType);

	DensityNoise1->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.type,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.seed,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.frequency,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.interpolation,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.fractalType,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.octaves,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.lacunarity,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.gain,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity1.celReturnType);

	DensityNoise2->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.type,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.seed,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.frequency,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.interpolation,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.fractalType,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.octaves,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.lacunarity,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.gain,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Denstity2.celReturnType);

	ContinentalnessNoise0->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.type,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.seed,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.frequency,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.interpolation,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.fractalType,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.octaves,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.lacunarity,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.gain,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness0.celReturnType);


	ContinentalnessNoise1->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.type,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.seed,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.frequency,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.interpolation,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.fractalType,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.octaves,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.lacunarity,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.gain,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Continentalness1.celReturnType);

	ErosionNoise->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.type,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.seed,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.frequency,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.interpolation,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.fractalType,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.octaves,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.lacunarity,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.gain,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Erosion.celReturnType);
	
	WeirdnessNoise->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.type,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.seed,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.frequency,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.interpolation,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.fractalType,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.octaves,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.lacunarity,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.gain,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->Weirdness.celReturnType);

	PVNoise->SetupFastNoise(UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.type,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.seed,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.frequency,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.interpolation,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.fractalType,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.octaves,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.lacunarity,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.gain,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.cellularJitter,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.celDistFunc,
		UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PV.celReturnType);

	float temp{};
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ContTHCurve, UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->ContTHCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ErosTHCurve, UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->ErosTHCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, PVTHCurve, UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PVTHCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ContSFCurve, UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->ContSFCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ErosSFCurve, UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->ErosSFCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, PVSFCurve, UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->PVSFCurveID, 0);

}

float UTerrainShaper::GetContinentalnessAtXY_BP(double X, double Y)
{
	return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetContinentalnessAtXY_Native(X, Y);
}

EContinentsIndex UTerrainShaper::GetContinentsIndex_BP(float continentalness)
{
	return UTerrainShaper::GetContinentsIndex(continentalness);
}

float UTerrainShaper::GetErosionAtXY_BP(double X, double Y)
{
	return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetErosionAtXY_Native(X, Y);
}

EErosionIndex UTerrainShaper::GetErosionIndex_BP(float erosion)
{
	return UTerrainShaper::GetErosionIndex(erosion);
}

int32 UTerrainShaper::GetTerrainHeightAtXY_BP(double X, double Y)
{
	float continentalness = UChunkUtilityLib::GetTerrainShaper()->GetContinentalnessAtXY_Native(X, Y);
	float erosion = UChunkUtilityLib::GetTerrainShaper()->GetErosionAtXY_Native(X, Y);
	float PV = UChunkUtilityLib::GetTerrainShaper()->GetPVAtXY_Native(X, Y);
	return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetTerrainHeightAtXY_Native(X, Y, continentalness, erosion, PV);
}
float UTerrainShaper::GetSFAtXY_BP(double X, double Y)
{
	float continentalness = UChunkUtilityLib::GetTerrainShaper()->GetContinentalnessAtXY_Native(X, Y);
	float erosion = UChunkUtilityLib::GetTerrainShaper()->GetErosionAtXY_Native(X, Y);
	float PV = UChunkUtilityLib::GetTerrainShaper()->GetPVAtXY_Native(X, Y);
	return UChunkUtilityLib::GetTerrainShaper()->GetSqueezeFactorAtXY_Native(X, Y, continentalness, erosion, PV);
}
float UTerrainShaper::GetInitialDensityAtXYZ_BP(double worldXPos, double worldYPos, double worldZPos)
{
	return UChunkUtilityLib::GetChunkManager()->TerrainShaper->GetInitialDensityAtXYZ_Native(worldXPos, worldYPos, worldZPos);
}
float UTerrainShaper::GetFinalDensityAtXYZ_BP(float initialDensity, double worldZPos, int32 terrainHeight, float SF)
{
	return UChunkUtilityLib::GetTerrainShaper()->GetFinalDensity_Native(initialDensity, worldZPos, terrainHeight, SF);
}

int16 UTerrainShaper::GetTerrainHeightAtXY_Native(double worldX, double worldY, float C, float E, float PV, float W)
{
	int16 output{};
	output += FMath::Floor(ContTHCurve->Eval(C));
	output += FMath::Floor(ErosTHCurve->Eval(E));
	output += FMath::Floor(PVTHCurve->Eval(C));
	return output;
}

float UTerrainShaper::GetSqueezeFactorAtXY_Native(double worldX, double worldY, float C, float E, float PV, float W)
{

	float continentalnessSFCurveValue = ContSFCurve->Eval(C);
	float erosionSFCurveValue = ErosSFCurve->Eval(C);
	float PVSFCurveValue = PVSFCurve->Eval(C);

	float ModifiedSF = UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->BaseSqueezeFactor
		* (continentalnessSFCurveValue + erosionSFCurveValue + PVSFCurveValue);
	return ModifiedSF;
}




void UTerrainShaper::GenerateRegionInitialData(FRegionGenerationData& regionData, FIntVector regionID)
{
	FIntVector RegionOrigin = UChunkUtilityLib::GetRegionOrigin(regionID);
	

	for (int32 x = 0; x < FChunkRegion::RegionSizeInBlocks; x++)
	{
		for (int32 y = 0; y < FChunkRegion::RegionSizeInBlocks; y++)
		{
			int32 index = (x + y * FChunkRegion::RegionSizeInBlocks);
			int32 xPosOffset = x + RegionOrigin.X;
			int32 yPosOffset = y + RegionOrigin.Y;

			*(regionData.ContinentalnessValues.GetData() + index) =
				GetContinentalnessAtXY_Native(xPosOffset, yPosOffset);

			*(regionData.ErosionValues.GetData() + index) =
				GetErosionAtXY_Native(xPosOffset, yPosOffset);

			*(regionData.PVValues.GetData() + index) =
				GetPVAtXY_Native(xPosOffset, yPosOffset);

			*(regionData.TerrainHeightValues.GetData() + index) =
				GetTerrainHeightAtXY_Native(xPosOffset, yPosOffset,
					*(regionData.ContinentalnessValues.GetData() + index),
					*(regionData.ErosionValues.GetData() + index),
					*(regionData.PVValues.GetData() + index));

			*(regionData.SFValues.GetData() + index) =
				GetSqueezeFactorAtXY_Native(xPosOffset, yPosOffset,
					*(regionData.ContinentalnessValues.GetData() + index),
					*(regionData.ErosionValues.GetData() + index),
					*(regionData.PVValues.GetData() + index));
		}
	}
	
}


void UTerrainShaper::GenerateTerrainShape(FChunkProtoRegion& protoRegion,
									      FRegionGenerationData& regionGenData,
										  FIntVector regionID)
{
	
	FIntVector regionOriginB = UChunkUtilityLib::GetRegionOrigin(regionID);
	FVector chunkLocation{};
	FProtoChunkData* chunkData = nullptr;

	
	for (int32 x = 0; x < FChunkRegion::RegionSizeInBlocks; x++)
	{
		int32 worldXPos = regionOriginB.X + x;
		int32 chunkX = FMath::Floor(double(worldXPos) / double(AChunk::Size)) * AChunk::SizeScaled.X;
		chunkLocation.X = chunkX;
		
		for (int32 y = 0; y < FChunkRegion::RegionSizeInBlocks; y++)
		{
			
			int32 worldYPos = regionOriginB.Y + y;
			
			int32 genDataIndex = (x + y * FChunkRegion::RegionSizeInBlocks);

			float SF = *(regionGenData.SFValues.GetData() + genDataIndex);
			int16 TH = *(regionGenData.TerrainHeightValues.GetData() + genDataIndex);
			
			int32 chunkY = FMath::Floor(double(worldYPos) / double(AChunk::Size)) * AChunk::SizeScaled.X;
			chunkLocation.Y = chunkY;
			

			
			for (int32 z = 0; z < FChunkRegion::RegionSizeInBlocks; z++)
			{
				int32 worldZPos = regionOriginB.Z + z;
				
				int32 chunkZ = FMath::Floor(double(worldZPos) / double(AChunk::Size)) * AChunk::SizeScaled.X;
				chunkLocation.Z = chunkZ;
				chunkData = &protoRegion.ProtoChunkData.FindOrAdd(chunkLocation);

				float density = GetInitialDensityAtXYZ_Native(worldXPos, worldYPos, worldZPos);

				density = GetFinalDensity_Native(density, worldZPos, TH, SF);

				int32 chunkIndex = UChunkUtilityLib::LocalBlockPosToIndex(UChunkUtilityLib::BlockPosToLocalBlockPos({ x,y,z }));
				
				if(density < 0)
				{
					(chunkData->BlockStates.GetData() + chunkIndex)->blockID = 0;
					if(worldZPos <= UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->SeaLevel)
					{
						*(chunkData->FluidStates.GetData() + chunkIndex) = FFluidState(1,100);
					}
				}
				else
				{
					(chunkData->BlockStates.GetData() + chunkIndex)->blockID = 1;
					*(chunkData->FluidStates.GetData() + chunkIndex) = FFluidState(UINT8_MAX, 0);
				}
			}
		}
		
	}
}