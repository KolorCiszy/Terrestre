

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "TerrestreGameModeInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTerrestreGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TERRESTRE_API ITerrestreGameModeInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	/*  Called when a player controller requests to spawn a pawn. 
	* @param PlayerController The player controller requesting the pawn spawn.
	* @return true if the pawn was spawned successfully, false otherwise.
	*/
	virtual bool RequestPlayerPawnSpawn(APlayerController* PlayerController);
};
