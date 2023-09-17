#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TerrestrePlayerState.generated.h"

struct FPlayerData;
UCLASS()
class ATerrestrePlayerState : public APlayerState
{
	GENERATED_BODY()

	ATerrestrePlayerState();
public:
	void ReadLoadedPlayerData(TSharedPtr<FPlayerData> data);

	inline FVector GetSpawnLocation() const;

	inline void SetSpawnLocation(FVector location);

	inline FTransform GetSpawnTransform() const;
private:

	FTransform SpawnTransform;
	
};

