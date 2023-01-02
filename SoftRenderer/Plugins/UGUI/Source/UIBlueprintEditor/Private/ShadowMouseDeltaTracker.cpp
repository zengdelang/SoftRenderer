#include "ShadowMouseDeltaTracker.h"

#define LOCTEXT_NAMESPACE "MouseDeltaTracker"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FShadowMouseDeltaTracker
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FShadowMouseDeltaTracker::FShadowMouseDeltaTracker()
	: Start( FVector::ZeroVector )
	, StartSnapped( FVector::ZeroVector )
	, StartScreen( FVector::ZeroVector )
	, End( FVector::ZeroVector )
	, EndSnapped( FVector::ZeroVector )
	, EndScreen( FVector::ZeroVector )
	, RawDelta( FVector::ZeroVector )
	, ReductionAmount( FVector::ZeroVector )
	, DragTool( NULL )
	, bHasAttemptedDragTool(false)
	, bUsedDragModifier(false)
	, bIsDeletingDragTool(false)
{
}

FShadowMouseDeltaTracker::~FShadowMouseDeltaTracker()
{
}

#undef LOCTEXT_NAMESPACE
