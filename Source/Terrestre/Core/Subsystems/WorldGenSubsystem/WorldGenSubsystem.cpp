#include "WorldGenSubsystem.h"
#include "Terrestre/Core/Gamemode/TerrestreGameInstance.h"
#include "Terrestre/Core/Subsystems/WorldGenSubsystem/Data Generators/TerrainShaper.h"
#include "Terrestre/Core/Subsystems/WorldGenSubsystem/Data Generators/TerrainSurfaceDecorator.h"

void UWorldGenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UWorldGenSubsystem::ReceivePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UWorldGenSubsystem::ReceivePostLoadMapWithWorld);
	TerrainShaper = NewObject<UTerrainShaper>(this);

	TerrainSurfaceDecorator = NewObject<UTerrainSurfaceDecorator>(this);

	CreateWorldGenThreadPool();

}
void UWorldGenSubsystem::Deinitialize()
{
	Super::Deinitialize();
	WorldGenTP->Destroy();
	delete WorldGenTP;
}
void UWorldGenSubsystem::ReceivePreLoadMap(const FString& InMapName)
{
	UTerrestreGameInstance* GameInstance = CastChecked<UTerrestreGameInstance>(GetGameInstance());
	if (FName(InMapName) == GameInstance->GetPrimaryDimensionName())
	{
		TerrainShaper->Initialize();
		TerrainSurfaceDecorator->Initialize();
	}
}
void UWorldGenSubsystem::CreateWorldGenThreadPool()
{
	UTerrestreGameInstance* GameInstance = CastChecked<UTerrestreGameInstance>(GetGameInstance());

	int32 threadsToCreate = GameInstance->GetWorldGenThreadCount();
	ensureMsgf(threadsToCreate < FPlatformMisc::NumberOfCores() &&
		threadsToCreate > 0,
		TEXT("That's an invalid amount of threads to create"));
	WorldGenTP = FQueuedThreadPool::Allocate();
	WorldGenTP->Create(threadsToCreate, 128 * 1024, TPri_AboveNormal, TEXT("World Gen Thread Pool"));
	
	
}
FQueuedThreadPool* UWorldGenSubsystem::GetRegionGenPool()
{
	return WorldGenTP;
}
TFuture<FChunkData> UWorldGenSubsystem::ProvideChunkData(AChunk* Chunk)
{
	switch (Chunk->GetGenStage()) 
	{
	case EChunkGenStage::Empty: return GenerateChunkTerrainShape(Chunk);
		break;
	case EChunkGenStage::TerrainShape: return GenerateChunkSurfaceLayer(Chunk);
		break;
	case EChunkGenStage::SurfaceLayer: return AsyncPool(*WorldGenTP, [Chunk]() // temp lambda that will only update gen stage to full
		{
			FChunkData FinalData = Chunk->GetChunkData();
			FinalData.GenStage = EChunkGenStage::Full;
			return FinalData;
		}, [Chunk]() {Chunk->ChunkDataReady.ExecuteIfBound(); });
									 break;
	default: UE_LOG(LogTemp, Warning, TEXT("Something went really wrong."));
		break;
	}
	return TFuture<FChunkData>();
}
TFuture<FChunkData> UWorldGenSubsystem::GenerateChunkTerrainShape(AChunk* Chunk)
{	
	
	TFuture<FChunkData> Result = AsyncPool(*WorldGenTP, [this, Chunk]
		{

			FProtoChunkData ProtoData{};

			FChunkGenerationData GenerationData{};

			TerrainShaper->GenerateChunkInitialData(GenerationData, Chunk->GetActorLocation());

			TerrainShaper->GenerateTerrainShape(ProtoData, GenerationData, Chunk->GetActorLocation());

			FChunkData FinalData{ ProtoData };

			FinalData.GenStage = EChunkGenStage::TerrainShape;

			return FinalData;
		}, [Chunk]() {Chunk->ChunkDataReady.ExecuteIfBound(); });
	
	return Result;
	
}

TFuture<FChunkData> UWorldGenSubsystem::GenerateChunkSurfaceLayer(AChunk* Chunk)
{
	TFuture<FChunkData> Result = AsyncPool(*WorldGenTP, [this, Chunk]
		{
			FChunkData FinalData = Chunk->GetChunkData();

			bool bSuccess = TerrainSurfaceDecorator->GenerateSurfaceLayer(Chunk, FinalData);

			FinalData.GenStage = bSuccess ? EChunkGenStage::SurfaceLayer : EChunkGenStage::TerrainShape;
			return FinalData;
		}, [Chunk]() {Chunk->ChunkDataReady.ExecuteIfBound(); });
	return Result;
}