#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "BaseCharacter.generated.h"

struct FBlockState;


UCLASS(Abstract)
class ABaseCharacter : public ACharacter
{
	GENERATED_BODY()
public:

	FGameplayTagContainer& GetCharacterGameplayTags();
private:

	UFUNCTION(BlueprintCallable, Category = "Base Character")
	virtual FBlockState  GetBlockStateCharStandingOn();

	FGameplayTagContainer CharacterGameplayTags;
};