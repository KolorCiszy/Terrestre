
#include "BlockPalette.h"
#include "Containers/BitArray.h"
#include "Containers/Set.h"
#include "Containers/SortedMap.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkConstants.h"

FBlockPalette::FBlockPalette(TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>>& rawData)
{
    TSet<FBlockState> entries;
    for (auto& block : rawData)
    {
        entries.FindOrAdd(block);
    }
	
    int16 index{};
    paletteEntries.Reset();
    bHomogenous = false;
    bEmpty = false;
    for (const auto& blockState : entries)
    {
        paletteEntries.Add(index++, FPaletteEntry(blockState, 0));
    }
    bitsPerBlock = CalculateBitsPerBlock();
    data.PadToNum(FChunkConstants::Volume * uint32(bitsPerBlock),0);
    int32 dataIndex{};
    for (const auto& BlockState : rawData)
    {
        index = FindBlockPaletteIndex(BlockState);
        paletteEntries[index].refCount++;

        for (uint8 currentBit{}; currentBit < bitsPerBlock; currentBit++)
        {

            data[dataIndex + currentBit] = static_cast<bool>(index % 2);
            index >>= 1;

        }
        dataIndex += bitsPerBlock;
    }
    if (paletteEntries.Num() == 1 && paletteEntries[0].refCount == FChunkConstants::Volume)
    {
        bHomogenous = true;
		if (paletteEntries[0].block.IsAirBlock())
		{
            bEmpty = true;
		}
    }
}

FBlockState FBlockPalette::GetBlockAtIndex(const int16 index) const
{
    if (bHomogenous)
        return paletteEntries[0].block;
    int32 dataIndex = bitsPerBlock * index;
    int16 paletteIndex{};
    for (uint8 i{}; i < bitsPerBlock; i++)
    {
        if (data[dataIndex + i])
            paletteIndex += pow(2, i);
    }
    return paletteEntries.Contains(paletteIndex) ? paletteEntries[paletteIndex].block : FBlockState{};
}
FBlockState FBlockPalette::GetBlockAtLocalPos(FIntVector localPos) const
{
    FIntVector PosAfterModulo = UChunkUtilityLib::VecModulo(localPos, FChunkConstants::Size);
    return GetBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex(PosAfterModulo));
}

int16 FBlockPalette::FindBlockPaletteIndex(const FBlockState& block) const
{
    auto predicate = [&](const TPair<int16, FPaletteEntry>& predEntry) {return predEntry.Value.block == block; };
    auto entry = Algo::FindByPredicate(paletteEntries, predicate);
    if (entry)
        return entry->Key;
    else
        return -1;      
}
void FBlockPalette::ModifyBlockAtIndex(int16 index, const FBlockState& newBlock)
{
    // * Get block state that is currently placed at this index
    FBlockState currentBlock = GetBlockAtIndex(index);
    
    int16 currentIndex = FindBlockPaletteIndex(currentBlock);
    if(currentBlock == newBlock)
    {
        // * New block is the same as the current one, so we do nothing
        return;
    }
    else
    {
        // * New block is different than the current one so, try to find the new block in the palette
        int16 paletteIndex = FindBlockPaletteIndex(newBlock);
        // * Also remove the old block from the palette, if present, as it will soon get replaced
        if(paletteEntries.Contains(currentIndex))
        {
            paletteEntries[currentIndex].refCount--;
            if (paletteEntries[currentIndex].refCount == 0)
            {
                paletteEntries.Remove(currentIndex);
                paletteEntries.Shrink();
            }
        }
        int32 dataIndex = index * bitsPerBlock;
        if (paletteIndex != -1)
        {
            // * New block is already present in the palette, so just increase its ref count
            paletteEntries[paletteIndex].refCount++;
            // * Then encode respective data bits
            for (uint8 i{}; i < bitsPerBlock; i++)
            {
                data[dataIndex + i] = static_cast<bool>(paletteIndex % 2);
                paletteIndex >>= 1;
            }
        }
        else
        {
            // * This block is not yet present in the palette so we will need to add it
            // * But before that, we have to check if the new palette index will fit into current bits per block
            if (bitsPerBlock == FMath::CeilLogTwo(paletteEntries.Num()+1))
            {
                // * Yay, we don't have to change bits per block so just add block to the palette
                int16 newBlockIndex = paletteEntries.Num();
                paletteEntries.Add(newBlockIndex, FPaletteEntry(newBlock, 1));
                // * And encode the new data
                for (uint8 i{}; i < bitsPerBlock; i++)
                {
                    data[dataIndex + i] = static_cast<bool>(newBlockIndex % 2);
                    newBlockIndex >>= 1;
                }
            }
            else
            {
                // * If the next index will be impossible to encode with current bits per block

                TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>> rawData{};
                BulkUnpack(rawData);
                // * The data is now in its raw form - just an array of block states, so we simply swap the index with the new block
                rawData[index] = newBlock;
                // * And re-encode the new palette
                FBlockPalette newPalette(rawData);
                *this = MoveTemp(newPalette);


            }
        }
        // * If the ref count of the old block is 0, we can remove it from the palette
        // * Last check - lowers bits per block, if possible
        if (bitsPerBlock != CalculateBitsPerBlock())
        {
            TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>> rawData{};
            BulkUnpack(rawData);
            FBlockPalette newPalette(rawData);
            *this = MoveTemp(newPalette);
        }
    }
}
void FBlockPalette::SetFill(FBlockState blockFill)
{

    paletteEntries.Empty();
    paletteEntries.Shrink();
    paletteEntries.Add(0, FPaletteEntry(blockFill, FChunkConstants::Volume));
    bHomogenous = true;
}
void FBlockPalette::BulkUnpack(TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>>& outputDestination) const
{
    if(bHomogenous)
    {
        outputDestination.Init(paletteEntries[0].block, FChunkConstants::Volume);
        return;
    }
    outputDestination.SetNumUninitialized(FChunkConstants::Volume);
    int32 dataIndex{};
    int32 dataItr{};
    for(int32 outputItr{0}; outputItr < outputDestination.Num(); outputItr++)
    {
        int16 blockIndex{};
        for (uint8 i{}; i < bitsPerBlock; i++)
        {
            if (data[dataItr + i])
            {
                blockIndex += pow(2, i);
            }
        }
        dataItr += bitsPerBlock;
        if(paletteEntries.Contains(blockIndex))
        {
            outputDestination[outputItr] = paletteEntries[blockIndex].block;
        }
            
    }
}
uint8 FBlockPalette::CalculateBitsPerBlock() const
{
    uint32 debug = FMath::CeilLogTwo(paletteEntries.Num());
    return debug;
}


