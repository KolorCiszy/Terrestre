#include "TerrainShaper.h"
#include "FastNoiseWrapper.h"
#include "Terrestre/Core/Chunk/Storage/ChunkData.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Gamemode/TerrestreGameModeBase.h"
#include "Terrestre/Core/Gamemode/TerrestreGameInstance.h"

UTerrainShaper::UTerrainShaper()
{
}

void UTerrainShaper::SetupNoiseFunctions()
{
	DensityNoise0 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	DensityNoise1 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	DensityNoise2 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());

	ContinentalnessNoise0 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());
	ContinentalnessNoise1 = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());

	ErosionNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());

	WeirdnessNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());

	PVNoise = NewObject<UFastNoiseWrapper>(UFastNoiseWrapper::StaticClass());

	int64 WorldSeed = GetWorld()->GetGameInstanceChecked<UTerrestreGameInstance>()->GetOpenWorldData()->WorldSeed;

	int32 DensitySeed = WorldSeed & 0x00'00'00'00'ff'ff'ff'ff;
	
	int32 ContinentalnessSeed = WorldSeed & 0x0f'0f'0f'0f'0f'0f'0f'0f;

	int32 ErosionSeed = WorldSeed & 0xff'00'ff'00'ff'00'ff'00;
	DensityNoise0->SetupFastNoise(
		Settings->Denstity0.type,
		DensitySeed,
		Settings->Denstity0.frequency,
		Settings->Denstity0.interpolation,
		Settings->Denstity0.fractalType,
		Settings->Denstity0.octaves,
		Settings->Denstity0.lacunarity,
		Settings->Denstity0.gain,
		Settings->Denstity0.cellularJitter,
		Settings->Denstity0.celDistFunc,
		Settings->Denstity0.celReturnType);

	DensityNoise1->SetupFastNoise(
		Settings->Denstity1.type,
		DensitySeed,
		Settings->Denstity1.frequency,
		Settings->Denstity1.interpolation,
		Settings->Denstity1.fractalType,
		Settings->Denstity1.octaves,
		Settings->Denstity1.lacunarity,
		Settings->Denstity1.gain,
		Settings->Denstity1.cellularJitter,
		Settings->Denstity1.celDistFunc,
		Settings->Denstity1.celReturnType);

	DensityNoise2->SetupFastNoise(
		Settings->Denstity2.type,
		DensitySeed,
		Settings->Denstity2.frequency,
		Settings->Denstity2.interpolation,
		Settings->Denstity2.fractalType,
		Settings->Denstity2.octaves,
		Settings->Denstity2.lacunarity,
		Settings->Denstity2.gain,
		Settings->Denstity2.cellularJitter,
		Settings->Denstity2.celDistFunc,
		Settings->Denstity2.celReturnType);

	ContinentalnessNoise0->SetupFastNoise(
		Settings->Continentalness0.type,
		ContinentalnessSeed,
		Settings->Continentalness0.frequency,
		Settings->Continentalness0.interpolation,
		Settings->Continentalness0.fractalType,
		Settings->Continentalness0.octaves,
		Settings->Continentalness0.lacunarity,
		Settings->Continentalness0.gain,
		Settings->Continentalness0.cellularJitter,
		Settings->Continentalness0.celDistFunc,
		Settings->Continentalness0.celReturnType);


	ContinentalnessNoise1->SetupFastNoise(
		Settings->Continentalness1.type,
		ContinentalnessSeed,
		Settings->Continentalness1.frequency,
		Settings->Continentalness1.interpolation,
		Settings->Continentalness1.fractalType,
		Settings->Continentalness1.octaves,
		Settings->Continentalness1.lacunarity,
		Settings->Continentalness1.gain,
		Settings->Continentalness1.cellularJitter,
		Settings->Continentalness1.celDistFunc,
		Settings->Continentalness1.celReturnType);

	ErosionNoise->SetupFastNoise(
		Settings->Erosion.type,
		Settings->Erosion.seed,
		Settings->Erosion.frequency,
		Settings->Erosion.interpolation,
		Settings->Erosion.fractalType,
		Settings->Erosion.octaves,
		Settings->Erosion.lacunarity,
		Settings->Erosion.gain,
		Settings->Erosion.cellularJitter,
		Settings->Erosion.celDistFunc,
		Settings->Erosion.celReturnType);

	WeirdnessNoise->SetupFastNoise(
		Settings->Weirdness.type,
		Settings->Weirdness.seed,
		Settings->Weirdness.frequency,
		Settings->Weirdness.interpolation,
		Settings->Weirdness.fractalType,
		Settings->Weirdness.octaves,
		Settings->Weirdness.lacunarity,
		Settings->Weirdness.gain,
		Settings->Weirdness.cellularJitter,
		Settings->Weirdness.celDistFunc,
		Settings->Weirdness.celReturnType);

	PVNoise->SetupFastNoise(
		Settings->PV.type,
		Settings->PV.seed,
		Settings->PV.frequency,
		Settings->PV.interpolation,
		Settings->PV.fractalType,
		Settings->PV.octaves,
		Settings->PV.lacunarity,
		Settings->PV.gain,
		Settings->PV.cellularJitter,
		Settings->PV.celDistFunc,
		Settings->PV.celReturnType);
}

void UTerrainShaper::Initialize()
{
	if (const UWorld* World = GetWorld())
	{
		const UTerrestreGameInstance* GameInstance = World->GetGameInstanceChecked<UTerrestreGameInstance>();
		if (GameInstance)
		{
			Settings = GameInstance->TerrainShaperSettings;
		}
		else
		{
			checkf(0, TEXT("wtf?"));
		}
	}
	
	SetupNoiseFunctions();

	
	float temp{};
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ContTHCurve, Settings->ContTHCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ErosTHCurve, Settings->ErosTHCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, PVTHCurve, Settings->PVTHCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ContSFCurve, Settings->ContSFCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, ErosSFCurve, Settings->ErosSFCurveID, 0);
	UDataRegistrySubsystem::Get()->EvaluateCachedCurve(temp, PVSFCurve, Settings->PVSFCurveID, 0);

	bInitialized = true;
}
void UTerrainShaper::DeleteNoiseFunctions()
{ //maybe not needed
	DensityNoise0->ConditionalBeginDestroy();
	DensityNoise0 = nullptr;
	DensityNoise1->ConditionalBeginDestroy();
	DensityNoise1 = nullptr;
	DensityNoise2->ConditionalBeginDestroy();
	DensityNoise2 = nullptr;
	//itd...
}

float UTerrainShaper::GetContinentalnessAtXY_BP(double X, double Y)
{
	return GetContinentalnessAtXY_Native(X, Y);
}

EContinentsIndex UTerrainShaper::GetContinentsIndex_BP(float continentalness)
{
	return UTerrainShaper::GetContinentsIndex(continentalness);
}

float UTerrainShaper::GetErosionAtXY_BP(double X, double Y)
{
	return GetErosionAtXY_Native(X, Y);
}

EErosionIndex UTerrainShaper::GetErosionIndex_BP(float erosion)
{
	return UTerrainShaper::GetErosionIndex(erosion);
}

int32 UTerrainShaper::GetTerrainHeightAtXY_BP(double X, double Y)
{
	float continentalness = GetContinentalnessAtXY_Native(X, Y);
	float erosion = GetErosionAtXY_Native(X, Y);
	float PV = GetPVAtXY_Native(X, Y);
	return GetTerrainHeightAtXY_Native(X, Y, continentalness, erosion, PV);
}
float UTerrainShaper::GetSFAtXY_BP(double X, double Y)
{
	float continentalness = GetContinentalnessAtXY_Native(X, Y);
	float erosion = GetErosionAtXY_Native(X, Y);
	float PV = GetPVAtXY_Native(X, Y);
	return GetSqueezeFactorAtXY_Native(X, Y, continentalness, erosion, PV);
}
float UTerrainShaper::GetInitialDensityAtXYZ_BP(double worldXPos, double worldYPos, double worldZPos)
{
	return GetInitialDensityAtXYZ_Native(worldXPos, worldYPos, worldZPos);
}
float UTerrainShaper::GetFinalDensityAtXYZ_BP(float initialDensity, double worldZPos, int32 terrainHeight, float SF)
{
	return GetFinalDensity_Native(initialDensity, worldZPos, terrainHeight, SF);
}

int16 UTerrainShaper::GetTerrainHeightAtXY_Native(double worldX, double worldY, float C, float E, float PV, float W)
{
	int16 output{};
	output += FMath::Floor(ContTHCurve->Eval(C));
	output += FMath::Floor(ErosTHCurve->Eval(E));
	output += FMath::Floor(PVTHCurve->Eval(PV));
	return output;
}

float UTerrainShaper::GetSqueezeFactorAtXY_Native(double worldX, double worldY, float C, float E, float PV, float W)
{

	float continentalnessSFCurveValue = ContSFCurve->Eval(C);
	float erosionSFCurveValue = ErosSFCurve->Eval(C);
	float PVSFCurveValue = PVSFCurve->Eval(C);

	float ModifiedSF = Settings->BaseSqueezeFactor
		* (continentalnessSFCurveValue + erosionSFCurveValue + PVSFCurveValue);
	return ModifiedSF;
}




void UTerrainShaper::GenerateChunkInitialData(FChunkGenerationData& outChunkGenData, FVector chunkLocation)
{
	FIntVector ChunkBlockLocation = UChunkUtilityLib::WorldLocationToBlockPos(chunkLocation);
	FChunkGenerationData& GenData = outChunkGenData;
	for (int32 x = 0; x < FChunkConstants::Size; x++)
	{
		for (int32 y = 0; y < FChunkConstants::Size; y++)
		{
			int32 index = (x + y * FChunkConstants::Size);
			int32 xWorldPosOffset = x + ChunkBlockLocation.X;
			int32 yWorldPosOffset = y + ChunkBlockLocation.Y;

			*(GenData.ContinentalnessValues.GetData() + index) =
				GetContinentalnessAtXY_Native(xWorldPosOffset, yWorldPosOffset);

			*(GenData.ErosionValues.GetData() + index) =
				GetErosionAtXY_Native(xWorldPosOffset, yWorldPosOffset);

			*(GenData.PVValues.GetData() + index) =
				GetPVAtXY_Native(xWorldPosOffset, yWorldPosOffset);

			*(GenData.TerrainHeightValues.GetData() + index) =
				GetTerrainHeightAtXY_Native(xWorldPosOffset, yWorldPosOffset,
					*(GenData.ContinentalnessValues.GetData() + index),
					*(GenData.ErosionValues.GetData() + index),
					*(GenData.PVValues.GetData() + index));

			*(GenData.SFValues.GetData() + index) =
				GetSqueezeFactorAtXY_Native(xWorldPosOffset, yWorldPosOffset,
					*(GenData.ContinentalnessValues.GetData() + index),
					*(GenData.ErosionValues.GetData() + index),
					*(GenData.PVValues.GetData() + index));
		}
	}
	
}


void UTerrainShaper::GenerateTerrainShape(FProtoChunkData& outProtoData,
											FChunkGenerationData& inChunkGenData,
											FVector chunkLocation)
{
	FIntVector ChunkBlockLocation = UChunkUtilityLib::WorldLocationToBlockPos(chunkLocation);
	for (int32 x = 0; x < FChunkConstants::Size; x++)
	{
		int32 worldXPos = ChunkBlockLocation.X + x;
		
		for (int32 y = 0; y < FChunkConstants::Size; y++)
		{
			int32 genDataIndex = (x + y * FChunkConstants::Size);

			float SF = *(inChunkGenData.SFValues.GetData() + genDataIndex);
			int16 TH = *(inChunkGenData.TerrainHeightValues.GetData() + genDataIndex);
			
			int32 worldYPos = ChunkBlockLocation.Y + y;

			for (int32 z = 0; z < FChunkConstants::Size; z++)
			{
				int32 worldZPos = ChunkBlockLocation.Z + z;
				
				float density = GetInitialDensityAtXYZ_Native(worldXPos, worldYPos, worldZPos);

				density = GetFinalDensity_Native(density, worldZPos, TH, SF);

				int32 chunkIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,z });
				
				if(density < 0)
				{
					(outProtoData.BlockStates.GetData() + chunkIndex)->blockID = 0;
					if(worldZPos <= Settings->SeaLevel)
					{
						*(outProtoData.FluidStates.GetData() + chunkIndex) = FFluidState(1,100);
					}
				}
				else
				{
					(outProtoData.BlockStates.GetData() + chunkIndex)->blockID = 1;
					*(outProtoData.FluidStates.GetData() + chunkIndex) = FFluidState(UINT8_MAX, 0);
				}
			}
		}
	}
}