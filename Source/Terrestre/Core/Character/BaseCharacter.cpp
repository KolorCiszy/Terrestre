
#include "BaseCharacter.h"

#include "Terrestre/Core/Chunk/Storage/BlockState.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Components/CapsuleComponent.h"


FGameplayTagContainer& ABaseCharacter::GetCharacterGameplayTags()
{
	return CharacterGameplayTags;
}
FBlockState ABaseCharacter::GetBlockStateCharStandingOn()
{
	FVector feetLocation = GetActorLocation();
	feetLocation.Z  += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return UChunkUtilityLib::GetChunkManager()->GetBlockAtWorldLocation(feetLocation);
}