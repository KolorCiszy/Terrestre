#pragma once

#include "CoreMinimal.h"
#include "BlockState.h"
#include "Terrestre/Core/Chunk/ChunkConstants.h"

class BitArray;
class SortedMap;

struct FPaletteEntry
{
	FPaletteEntry() : block{ 0 }, refCount{ FChunkConstants::Volume } {};
	FPaletteEntry(FBlockState inBlock, uint32 inRefCount) : block{ inBlock }, refCount{ inRefCount } {};
	FBlockState block;
	uint32 refCount;
};
FORCEINLINE FArchive& operator<<(FArchive& ar, FPaletteEntry& pe)
{
	ar << pe.block;
	ar << pe.refCount;
	return ar;
}

class FBlockPalette
{
public:
	FBlockPalette() : data{}, paletteEntries{}, bitsPerBlock{}, bHomogenous{}, bEmpty{}
	{	};
	FBlockPalette(TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>>& rawData);

	FBlockPalette(const FBlockPalette&) = default;
	
	FBlockPalette(FBlockPalette&& move)
	{
		data = MoveTemp(move.data);
		paletteEntries = MoveTemp(move.paletteEntries);
		bHomogenous = move.bHomogenous;
		bEmpty = move.bEmpty;
		bitsPerBlock = move.bitsPerBlock;
		
	}
	FBlockPalette& operator=(const FBlockPalette&) = default;

	FBlockPalette& operator=(FBlockPalette&& move)
	{
		data = MoveTemp(move.data);
		paletteEntries = MoveTemp(move.paletteEntries);
		bHomogenous = move.bHomogenous;
		bitsPerBlock = move.bitsPerBlock;
		bEmpty = move.bEmpty;
		return *this;
	}

	FBlockState GetBlockAtLocalPos(FIntVector localPos) const;
	// * Gets the block state at given index (chunk index)
	FBlockState GetBlockAtIndex(const int16 index) const;
	// * Decodes all data into an array
	void BulkUnpack(TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>>& outputDestination) const;

	bool FORCEINLINE IsEmpty() const
	{
		return bEmpty;
	}
	
	/* blockFill - this type of block fills whole section */
	inline bool IsHomogenous(FBlockState& blockFill) const
	{
		blockFill = paletteEntries[0].block;
		return bHomogenous;
	}

	/* blockFill - fill whole section with specified block type */
	void SetFill(FBlockState blockFill);

	void ModifyBlockAtIndex(int16 index, const FBlockState& newBlock);

	friend FArchive& operator<<(FArchive& ar, FBlockPalette& palette);
	
private:
	int16 FindBlockPaletteIndex(const FBlockState& block) const;

	uint8 FORCEINLINE CalculateBitsPerBlock() const; 
	
	TBitArray<FDefaultAllocator> data;
	TSortedMap<int16, FPaletteEntry> paletteEntries; // <index, paletteEntry>
	
	int8 bitsPerBlock;
	bool bHomogenous : 1;
	bool bEmpty : 1;
	
};
FORCEINLINE FArchive& operator<<(FArchive& ar, FBlockPalette& palette)
{
	ar << palette.bHomogenous;
	ar << palette.bitsPerBlock;
	ar << palette.paletteEntries;
	ar << palette.data;
	return ar;
}
