#pragma once

#include "CoreMinimal.h"
#include "BlockState.h"
#include "Terrestre/Core/Chunk/Chunk.h"


class BitArray;
class SortedMap;



struct FPaletteEntry
{
	FPaletteEntry(FBlockState inBlock, uint32 inRefCount) : block{ inBlock }, refCount{ inRefCount } {};
	FBlockState block;
	uint32 refCount;
};
class FBlockPalette
{
public:
	FBlockPalette() : data{}, paletteEntries{}, bHomogenous{}, bitsPerBlock{} 
	{	};
	FBlockPalette(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& rawData);

	FBlockPalette(const FBlockPalette&) = delete;
	
	FBlockPalette(FBlockPalette&& move)
	{
		data = MoveTemp(move.data);
		paletteEntries = MoveTemp(move.paletteEntries);
		bHomogenous = move.bHomogenous;
		bitsPerBlock = move.bitsPerBlock;
	}
	FBlockPalette& operator=(const FBlockPalette&) = delete;

	FBlockPalette& operator=(FBlockPalette&& move)
	{
		data = MoveTemp(move.data);
		paletteEntries = MoveTemp(move.paletteEntries);
		bHomogenous = move.bHomogenous;
		bitsPerBlock = move.bitsPerBlock;
		return *this;
	}

	// * Gets the block state at given index (chunk index)
	FBlockState GetBlockAtIndex(const int16 index) const;
	// * Decodes all data into an array
	void BulkUnpack(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& outputDestination);

	FORCEINLINE
	bool IsEmpty() const;
	

	/* blockFill - this type of block fills whole section */
	inline bool IsHomogenous(FBlockState& blockFill) const
	{
		blockFill = paletteEntries[0].block;
		return bHomogenous;
	}

	/* blockFill - fill whole section with specified block type */
	void SetFill(FBlockState blockFill);

	void ModifyBlockAtIndex(int16 index, const FBlockState& newBlock);
private:
	int16 FindBlockPaletteIndex(const FBlockState& block) const;

	uint8 CalculateBitsPerBlock() const; 
	
	TBitArray<FDefaultAllocator> data;
	TSortedMap<int16, FPaletteEntry> paletteEntries; // <index, paletteEntry>
	bool bHomogenous;
	int8 bitsPerBlock;
	
};

