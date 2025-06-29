#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Terrestre/Core/Gamemode/PlayerGameMode.h"
#include "Terrestre/Core/Interfaces/PlayerInteraction.h"
#include "TerrestrePlayerState.generated.h"

class UInventoryComponent;
struct FPlayerData;
UCLASS(Config=Game)
class TERRESTRE_API ATerrestrePlayerState : public APlayerState, public IPlayerInteraction
{
	GENERATED_BODY()

	ATerrestrePlayerState();
public:
	void ReadLoadedPlayerData(TSharedPtr<FPlayerData> data);

	void SetSpawnLocation(FVector location);

	FTransform GetSpawnTransform() const;



	UFUNCTION(BlueprintPure, Category = "Player State")
	EPlayerGameMode GetPlayerGameMode()
	{
		return CurrentPlayerGM;
	}
	UFUNCTION(BlueprintCallable, Category = "Player State")
	void SetPlayerGameMode(EPlayerGameMode newGM)
	{
		CurrentPlayerGM = newGM; // TODO: add multicast delegate to notify change
	}

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Player State")
	float GetAttackCooldown()
	{
		return CurrentAttackCooldown;
	}
	UFUNCTION(BlueprintPure, Category = "Player State")
	float GetBlockPlaceCooldown()
	{
		return CurrentBlockPlaceCooldown;
	}
	UFUNCTION(BlueprintCallable, Category = "Player State")
	void ResetBlockPlaceCooldown()
	{
		CurrentBlockPlaceCooldown = 0.0f;
	}
	UFUNCTION(BlueprintCallable, Category = "Player State")
	void ResetAttackCooldown()
	{
		CurrentAttackCooldown = 0.0f;
	}

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void TriggerBlockPlaceCooldown()
	{
		CurrentBlockPlaceCooldown = BlockPlaceCooldown;
	}
	UFUNCTION(BlueprintCallable, Category = "Player State")
	void TriggerAttackCooldown()
	{
		CurrentAttackCooldown = AttackCooldown;
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BlockPlaceCooldown;

	UPROPERTY(BlueprintReadOnly)
	float CurrentBlockPlaceCooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackCooldown;

	UPROPERTY(BlueprintReadOnly)
	float CurrentAttackCooldown;

	UPROPERTY(EditDefaultsOnly)
	FTransform SpawnTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> MainInventory;

	UPROPERTY(BlueprintReadOnly)
	EPlayerGameMode CurrentPlayerGM;


private:
};

