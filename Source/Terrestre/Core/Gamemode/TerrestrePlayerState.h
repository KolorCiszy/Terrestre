#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TerrestrePlayerState.generated.h"

struct FPlayerData;
UCLASS(Config=Game)
class ATerrestrePlayerState : public APlayerState
{
	GENERATED_BODY()

	ATerrestrePlayerState();
public:
	void ReadLoadedPlayerData(TSharedPtr<FPlayerData> data);

	FVector GetSpawnLocation() const;

	void SetSpawnLocation(FVector location);

	FTransform GetSpawnTransform() const;
private:

	FTransform SpawnTransform;
	
};

