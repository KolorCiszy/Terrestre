#pragma once

#include "FastNoiseWrapper.h"
#include "FastNoiseSettings.generated.h"

USTRUCT(BlueprintType)
struct FFastNoiseSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		EFastNoise_NoiseType type = EFastNoise_NoiseType::Perlin;
	UPROPERTY(EditAnywhere)
		int32 seed = 6969;
	UPROPERTY(EditAnywhere)
		float frequency = 0.01f;
	UPROPERTY(EditAnywhere)
		EFastNoise_Interp interpolation = EFastNoise_Interp::Quintic;
	UPROPERTY(EditAnywhere)
		EFastNoise_FractalType fractalType = EFastNoise_FractalType::FBM;
	UPROPERTY(EditAnywhere)
		int32 octaves = 3;
	UPROPERTY(EditAnywhere)
		float lacunarity = 2.0f;
	UPROPERTY(EditAnywhere)
		float gain = 0.5f;
	UPROPERTY(EditAnywhere)
		float cellularJitter = 0.45f;
	UPROPERTY(EditAnywhere)
		EFastNoise_CellularDistanceFunction celDistFunc = EFastNoise_CellularDistanceFunction::Euclidean;
	UPROPERTY(EditAnywhere)
		EFastNoise_CellularReturnType celReturnType = EFastNoise_CellularReturnType::CellValue;

	FFastNoiseSettings() : type{ EFastNoise_NoiseType::Perlin }, seed{ 6969 }, frequency{ 0.01f },
		interpolation{ EFastNoise_Interp::Quintic }, fractalType{ EFastNoise_FractalType::FBM }, octaves{ 3 },
		lacunarity{ 2.0f }, gain{ 0.5f }, cellularJitter{ 0.45f },
		celDistFunc{ EFastNoise_CellularDistanceFunction::Euclidean }, celReturnType( EFastNoise_CellularReturnType::CellValue) {}

};
