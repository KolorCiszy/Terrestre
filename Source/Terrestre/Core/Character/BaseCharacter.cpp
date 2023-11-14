
#include "BaseCharacter.h"

#include "Terrestre/Core/Chunk/Storage/BlockState.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Components/CapsuleComponent.h"


ABaseCharacter::ABaseCharacter()
{
	OnCharacterMovementUpdated.AddDynamic(this, &ABaseCharacter::OnCharacterMovementUpdatedNative);
}

FGameplayTagContainer& ABaseCharacter::GetCharacterGameplayTags()
{
	return CharacterGameplayTags;
}
FBlockState ABaseCharacter::GetBlockStateCharStandingOn()
{
	FVector feetLocation = GetActorLocation();
	feetLocation.Z  -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 15.0);
	return UChunkUtilityLib::GetChunkManager()->GetBlockAtWorldLocation(feetLocation);
}
FFluidState ABaseCharacter::GetHeadFluidState()
{
	FVector headLocation = GetActorLocation();
	headLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.7;
	return UChunkUtilityLib::GetChunkManager()->GetFluidAtWorldLocation(headLocation);
}
void ABaseCharacter::OnStartSwimming_Implementation()
{
	GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Swimming);
	OnStartSwimming();
}
void ABaseCharacter::OnStopSwimming_Implementation()
{
	GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = false;
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	OnStopSwimming();
}
void ABaseCharacter::OnHeadSubmerged_Implementation()
{
	OnHeadSubmerged();
}
void ABaseCharacter::OnHeadAboveLiquid_Implementation()
{
	OnHeadAboveLiquid();
}
void ABaseCharacter::OnCharacterMovementUpdatedNative(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	auto fluid = GetHeadFluidState();
	auto block = GetBlockStateCharStandingOn();
	if (fluid.IsWater())
	{
		if (!bHeadUnderWater)
		{
			OnHeadSubmerged_Implementation();
			bHeadUnderWater = true;
		}
		
		if (block.CanCharacterStandOn())
		{
			if (GetCharacterMovement()->IsSwimming())
			{
				OnStopSwimming_Implementation();
			}
		}
		else
		{
			if (!GetCharacterMovement()->IsSwimming() && !GetCharacterMovement()->IsFlying())
			{
				OnStartSwimming_Implementation();
			}
			bSwimmingOnWaterSurface = false;
		}
	}
	else
	{
		if (GetCharacterMovement()->IsSwimming())
		{
			if (block.CanCharacterStandOn())
			{
				bSwimmingOnWaterSurface = false;
				OnStopSwimming_Implementation();
			}
			else
			{
				bSwimmingOnWaterSurface = true;
			}
		}
		if(bHeadUnderWater)
		{
			OnHeadAboveLiquid_Implementation();
			bHeadUnderWater = false;
		}
		
	}
	if (GetCharacterMovement()->Velocity.Z > 0 && bSwimmingOnWaterSurface)
	{
		GetCharacterMovement()->Velocity.Z = 0;
	}
}