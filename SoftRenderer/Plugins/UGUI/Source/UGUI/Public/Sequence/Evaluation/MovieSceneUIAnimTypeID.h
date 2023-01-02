#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeCounter.h"

/** Anim type ID container that uniquely identifies types of animated data based on a predicate data structure */
template<typename DataType>
struct TMovieSceneUIAnimTypeIDContainer
{
	/** Get the type unique animation type identifier for the specified predicate */
	FMovieSceneAnimTypeID GetAnimTypeID(const DataType& InPredicate)
	{
		// Crude spinlock
		while (Lock.Set(1));

		int32 Index = Data.IndexOfByKey(InPredicate);
		if (Index != INDEX_NONE)
		{
			FMovieSceneAnimTypeID Value = TypeIDs[Index];
			Lock.Set(0);
			return Value;
		}

		Data.Add(InPredicate);

		FTypeID NewID(this, TypeIDs.Num());
		TypeIDs.Add(NewID);

		Lock.Set(0);
		return NewID;
	}

private:

	struct FTypeID : FMovieSceneAnimTypeID
	{
		FTypeID(void* Base, uint32 InIndex)
		{
			ID = GenerateHash(Base, InIndex);
		}
	};

	FThreadSafeCounter Lock;

	/** Array of existing type identifiers */
	TArray<FMovieSceneAnimTypeID> TypeIDs;
	
	/** Array of data type predicates whose indices map to TypeIDs */
	TArray<DataType> Data;
};
