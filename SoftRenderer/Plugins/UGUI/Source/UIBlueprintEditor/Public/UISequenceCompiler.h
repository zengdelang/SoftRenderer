#pragma once

#include "CoreMinimal.h"
#include "UISequenceComponent.h"
#include "Sequence/Tracks/MovieSceneRotatorTrack.h"
#include "Sequence/Tracks/MovieSceneUIMaterialTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneBoolTrack.h"
#include "Tracks/MovieSceneByteTrack.h"
#include "Tracks/MovieSceneColorTrack.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneIntegerTrack.h"
#include "Tracks/MovieSceneMaterialTrack.h"
#include "Tracks/MovieSceneTransformTrack.h"
#include "Tracks/MovieSceneVectorTrack.h"

class UIBLUEPRINTEDITOR_API FUISequenceCompiler
{
public:
	static void CompileUISequenceData(const UBlueprint* Blueprint, FCompilerResultsLog* MessageLog);

protected:
	static void ExportUISequenceData(const UBlueprint* Blueprint, UUISequenceComponent* UISequenceComp, FCompilerResultsLog* MessageLog);
	
	static void CreateUISequenceTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		UMovieSceneTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate, FCompilerResultsLog* MessageLog, const bool bMasterTrack = false);

	static void CreateUISequenceBoolTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneBoolTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);
	
	static void CreateUISequenceFloatTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneFloatTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceVectorTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneVectorTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceIntTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneIntegerTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceByteTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneByteTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceColorTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneColorTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceRotatorTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneRotatorTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceTransformTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneTransformTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequence3DTransformTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieScene3DTransformTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);
	
	static void CreateUISequenceUIMaterialTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneUIComponentMaterialTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceComponentMaterialTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneComponentMaterialTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate);

	static void CreateUISequenceComponentAudioTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
		const UMovieSceneAudioTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate, const bool bMasterTrack = false);
};
