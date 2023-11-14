#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagsManager.h"
#include "BaseCharacter.generated.h"


struct FBlockState;


UCLASS(Abstract)
class ABaseCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	FGameplayTagContainer& GetCharacterGameplayTags();

	ABaseCharacter();

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer CharacterGameplayTags;

	UFUNCTION(BlueprintImplementableEvent)
	void OnStartSwimming();
	virtual void OnStartSwimming_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void OnStopSwimming();
	virtual void OnStopSwimming_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHeadSubmerged();
	virtual void OnHeadSubmerged_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHeadAboveLiquid();
	virtual void OnHeadAboveLiquid_Implementation();
protected:

	/* true is character is swimming on water surface */
	bool bSwimmingOnWaterSurface;

	/* true is character is swimming on water surface */
	bool bHeadUnderWater;
private:


	/* Get the block state character is standing on */
	UFUNCTION(BlueprintCallable, Category = "Base Character")
	virtual FBlockState GetBlockStateCharStandingOn();
	/* Get the fluid state character has its head submerged in */
	UFUNCTION(BlueprintCallable, Category = "Base Character")
	virtual FFluidState GetHeadFluidState();

	UFUNCTION()
	void OnCharacterMovementUpdatedNative(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);
};