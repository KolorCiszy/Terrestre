// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ChunkAccess.generated.h"

struct FBlockState;
struct FFluidState;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UChunkAccess : public UInterface
{
	GENERATED_BODY()
};

// Fix for UHT001: Unexpected '}'. Did you miss a semi-colon?  
// Ensure all methods have proper syntax and end with a semi-colon.  

class TERRESTRE_API IChunkAccess  
{  
    GENERATED_BODY()  

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.  
public:  
    virtual FBlockState GetBlockAtWorldPosition(const FVector& worldPosition) = 0;

    virtual FFluidState GetFluidAtWorldPosition(const FVector& worldPosition) = 0;
};
