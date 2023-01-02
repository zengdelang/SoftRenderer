#include "UISequenceCompiler.h"
#include "MovieSceneTimeHelpers.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "UISequenceComponent.h"
#include "Sections/MovieSceneBoolSection.h"
#include "Sections/MovieSceneColorSection.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Sections/MovieSceneIntegerSection.h"
#include "Sections/MovieSceneVectorSection.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Sections/MovieSceneAudioSection.h"
#include "Sections/MovieSceneByteSection.h"
#include "Sequence/Sections/MovieSceneRotatorSection.h"

void FUISequenceCompiler::CompileUISequenceData(const UBlueprint* Blueprint, FCompilerResultsLog* MessageLog)
{
	if (!Blueprint)
		return;
	
	TArray<USCS_Node*> RootNodes = Blueprint->SimpleConstructionScript->GetRootNodes();
	for (const auto& RootNode : RootNodes)
	{
		UUISequenceComponent* UISequenceComp = Cast<UUISequenceComponent>(RootNode->ComponentTemplate);
		if (!IsValid(UISequenceComp) || !UISequenceComp->Sequence || !UISequenceComp->Sequence->bChanged)
			continue;
		
		ExportUISequenceData(Blueprint, UISequenceComp, MessageLog);
	}
}

void FUISequenceCompiler::ExportUISequenceData(const UBlueprint* Blueprint, UUISequenceComponent* UISequenceComp, FCompilerResultsLog* MessageLog)
{
	UISequenceComp->Sequence->bChanged = false;
	UISequenceComp->ClearData();

	const EMovieSceneEvaluationType EvaluationType = UISequenceComp->Sequence->GetMovieScene()->GetEvaluationType();
	const FFrameRate                TickResolution = UISequenceComp->Sequence->GetMovieScene()->GetTickResolution();
	const FFrameRate                DisplayRate    = UISequenceComp->Sequence->GetMovieScene()->GetDisplayRate();
	
	FMovieScenePlaybackPosition PlayPosition;
	PlayPosition.SetTimeBase(DisplayRate, TickResolution, EvaluationType);
	
	// Set up the default frame range from the sequence's play range
	const TRange<FFrameNumber> PlaybackRange   = UISequenceComp->Sequence->GetMovieScene()->GetPlaybackRange();

	const FFrameNumber SrcStartFrame = UE::MovieScene::DiscreteInclusiveLower(PlaybackRange);
	const FFrameNumber SrcEndFrame   = UE::MovieScene::DiscreteExclusiveUpper(PlaybackRange);

	const FFrameTime StartTime = ConvertFrameTime(SrcStartFrame, TickResolution, DisplayRate);
	const FFrameTime EndingTime = ConvertFrameTime(SrcEndFrame, TickResolution, DisplayRate);

	const FFrameNumber StartingFrame = ConvertFrameTime(SrcStartFrame, TickResolution, DisplayRate).FloorToFrame();
	const FFrameNumber EndingFrame   = EndingTime.FloorToFrame();
	
	const int32 DurationFrames = (EndingFrame - StartingFrame).Value;
	const float DurationSubFrames = EndingTime.GetSubFrame();

	UISequenceComp->InputRate = PlayPosition.GetInputRate();
	UISequenceComp->TickResolution = TickResolution;
	UISequenceComp->DisplayRate = DisplayRate;
	UISequenceComp->StartTimeOffset = FQualifiedFrameTime(StartTime, PlayPosition.GetInputRate()).AsSeconds();
	UISequenceComp->Duration = FQualifiedFrameTime(FFrameTime(DurationFrames, DurationSubFrames), PlayPosition.GetInputRate()).AsSeconds();
 
	const TArray<FMovieSceneBinding>& Bindings = UISequenceComp->Sequence->GetMovieScene()->GetBindings();
	for (const auto& Binding : Bindings)
	{
		if (Binding.GetTracks().Num() == 0)
			continue;

		FUISequenceBinding SequenceBinding;
		SequenceBinding.BindingName = FName(Binding.GetName());
		
		if (const FUISequenceObjectReference* ObjectReference = UISequenceComp->Sequence->ObjectReferences.FindObjectReference(Binding.GetObjectGuid()))
		{
			if (ObjectReference->GetType() == EUISequenceObjectReferenceType::ContextActor)
			{
				SequenceBinding.BindingName = TEXT("Owner");
			}
		}
		else
		{
			if (MessageLog)
			{
				MessageLog->Note(*FString::Format(TEXT("UI sequence complie : export fails for non-existent property, Property:{0}, Component:{1}"),
	 {SequenceBinding.BindingName.ToString(), UISequenceComp->GetFullName()}));
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("%s"), *FString::Format(TEXT("UI sequence complie : export fails for non-existent property, Property:{0}, Component:{1}"),
						{SequenceBinding.BindingName.ToString(), UISequenceComp->GetFullName()}));
			}
			continue;
		}
		
		for (const auto& Track : Binding.GetTracks())
		{
			if (Track->IsEvalDisabled())
			{
				continue;
			}
			
			CreateUISequenceTrack(SrcStartFrame, SrcEndFrame, SequenceBinding, Track, UISequenceComp, PlayPosition, TickResolution, DisplayRate, MessageLog, false);
		}

		if (SequenceBinding.Tracks.Num() > 0)
		{
			UISequenceComp->Bindings.Emplace(SequenceBinding);
		}
	}

	const TArray<UMovieSceneTrack*>& MaterTracks = UISequenceComp->Sequence->GetMovieScene()->GetMasterTracks();
	for (const auto& MaterTrack : MaterTracks)
	{
		FUISequenceBinding SequenceBinding;
		SequenceBinding.bMaterTrack = true;
		SequenceBinding.BindingName = FName("MasterBinding");
		
		if (MaterTrack->IsEvalDisabled())
		{
			continue;
		}
			
		CreateUISequenceTrack(SrcStartFrame, SrcEndFrame, SequenceBinding, MaterTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate, MessageLog,true);

		if (SequenceBinding.Tracks.Num() > 0)
		{
			UISequenceComp->Bindings.Emplace(SequenceBinding);
		}
	}

	TArray<UObject*> ArchetypeInstances;
	UISequenceComp->GetArchetypeInstances(ArchetypeInstances);

	for (const auto ArchetypeInstance : ArchetypeInstances)
	{
		UUISequenceComponent* ArchetypeInstanceComp = Cast<UUISequenceComponent>(ArchetypeInstance);
		if (IsValid(ArchetypeInstanceComp) && ArchetypeInstanceComp != UISequenceComp)
		{
			ArchetypeInstanceComp->Modify();
						
			ArchetypeInstanceComp->StartTimeOffset = UISequenceComp->StartTimeOffset;
			ArchetypeInstanceComp->Duration = UISequenceComp->Duration;
			ArchetypeInstanceComp->DisplayRate = UISequenceComp->DisplayRate;
			ArchetypeInstanceComp->TickResolution = UISequenceComp->TickResolution;
			ArchetypeInstanceComp->InputRate = UISequenceComp->InputRate;
			ArchetypeInstanceComp->Bindings = UISequenceComp->Bindings;
			ArchetypeInstanceComp->BoolCurves = UISequenceComp->BoolCurves;
			ArchetypeInstanceComp->FloatCurves = UISequenceComp->FloatCurves;
			ArchetypeInstanceComp->IntCurves = UISequenceComp->IntCurves;
			ArchetypeInstanceComp->VectorCurves = UISequenceComp->VectorCurves;
			ArchetypeInstanceComp->AudioCurves = UISequenceComp->AudioCurves;
			
			ArchetypeInstanceComp->MarkPackageDirty();    
		}
	}
}

void FUISequenceCompiler::CreateUISequenceTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	UMovieSceneTrack* Track, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate, FCompilerResultsLog* MessageLog, const bool bMasterTrack)
{
	if (const UMovieSceneBoolTrack* BoolTrack = Cast<UMovieSceneBoolTrack>(Track))
	{
		CreateUISequenceBoolTrack(SrcStartFrame, SrcEndFrame, Binding, BoolTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneFloatTrack* FloatTrack = Cast<UMovieSceneFloatTrack>(Track))
	{
		CreateUISequenceFloatTrack(SrcStartFrame, SrcEndFrame, Binding, FloatTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneVectorTrack* VectorTrack = Cast<UMovieSceneVectorTrack>(Track))
	{
		CreateUISequenceVectorTrack(SrcStartFrame, SrcEndFrame, Binding, VectorTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneIntegerTrack* IntTrack = Cast<UMovieSceneIntegerTrack>(Track))
	{
		CreateUISequenceIntTrack(SrcStartFrame, SrcEndFrame, Binding, IntTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneByteTrack* ByteTrack = Cast<UMovieSceneByteTrack>(Track))
	{
		CreateUISequenceByteTrack(SrcStartFrame, SrcEndFrame, Binding, ByteTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneColorTrack* ColorTrack = Cast<UMovieSceneColorTrack>(Track))
	{
		CreateUISequenceColorTrack(SrcStartFrame, SrcEndFrame, Binding, ColorTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneRotatorTrack* RotatorTrack = Cast<UMovieSceneRotatorTrack>(Track))
	{
		CreateUISequenceRotatorTrack(SrcStartFrame, SrcEndFrame, Binding, RotatorTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneTransformTrack* TransformTrack = Cast<UMovieSceneTransformTrack>(Track))
	{
		CreateUISequenceTransformTrack(SrcStartFrame, SrcEndFrame, Binding, TransformTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieScene3DTransformTrack* Transform3DTrack = Cast<UMovieScene3DTransformTrack>(Track))
	{
		CreateUISequence3DTransformTrack(SrcStartFrame, SrcEndFrame, Binding, Transform3DTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneUIComponentMaterialTrack* UIMaterialTrack = Cast<UMovieSceneUIComponentMaterialTrack>(Track))
	{
		CreateUISequenceUIMaterialTrack(SrcStartFrame, SrcEndFrame, Binding, UIMaterialTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneComponentMaterialTrack* ComponentMaterialTrack = Cast<UMovieSceneComponentMaterialTrack>(Track))
	{
		CreateUISequenceComponentMaterialTrack(SrcStartFrame, SrcEndFrame, Binding, ComponentMaterialTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate);
	}
	else if (const UMovieSceneAudioTrack* AudioTrack = Cast<UMovieSceneAudioTrack>(Track))
	{
		CreateUISequenceComponentAudioTrack(SrcStartFrame, SrcEndFrame, Binding, AudioTrack, UISequenceComp, PlayPosition, TickResolution, DisplayRate, bMasterTrack);
	}
	else
	{
		if (Track)
		{
			if (MessageLog)
			{
				MessageLog->Warning(*FString::Format(TEXT("UI sequence complie : export fails for unsupported track types,Binding:{0},Track:{1},Track type:{2}"),
	{Binding.BindingName.ToString(),Track->GetTrackName().ToString(), Track->GetClass()->GetName()}));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s"), *FString::Format(TEXT("UI sequence complie : export fails for unsupported track types,Binding:{0},Track:{1},Track type:{2}"),
						{Binding.BindingName.ToString(),Track->GetTrackName().ToString(), Track->GetClass()->GetName()}));
			}
		}
	}
}

void FUISequenceCompiler::CreateUISequenceBoolTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneBoolTrack* BoolTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = BoolTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;
	
	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = BoolTrack->GetTrackName();
			
	if (UMovieSceneBoolSection* BoolSection = Cast<UMovieSceneBoolSection>(Sections[0]))
	{
		const FMovieSceneBoolChannel& BoolChannel= BoolSection->GetChannel();
		
		FUISequenceBoolCurve BoolCurve;
		
		for (const auto& TimeElem : BoolChannel.GetTimes())
		{
			const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
			BoolCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
		}

		for (const auto& ValueElem : BoolChannel.GetValues())
		{
			BoolCurve.Values.Add(ValueElem);
		}

		const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneBoolChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
		BoolCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&BoolChannel);

		const auto DefaultValueProperty = CastField<FBoolProperty>(FMovieSceneBoolChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
		BoolCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&BoolChannel);;

		SequenceTrack.TrackType = EUISequenceTrackType::Bool;
		SequenceTrack.SectionIndex = UISequenceComp->BoolCurves.Emplace(BoolCurve);
		check(SequenceTrack.SectionIndex < UINT16_MAX);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceFloatTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneFloatTrack* FloatTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = FloatTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = FloatTrack->GetTrackName();

	if (const UMovieSceneFloatSection* FloatSection = Cast<UMovieSceneFloatSection>(Sections[0]))
	{
		const FMovieSceneFloatChannel& FloatChannel = FloatSection->GetChannel();
		
		FUISequenceFloatCurve FloatCurve;

		for (const auto& TimeElem : FloatChannel.GetTimes())
		{
			const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
			FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
		}

		for (const auto& ValueElem : FloatChannel.GetValues())
		{
			FUISequenceFloatValue Value;
			Value.Tangent = ValueElem.Tangent;
			Value.Value = ValueElem.Value;

			if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
			{
				Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
			}
			else
			{
				Value.InterpMode = ValueElem.InterpMode;
			}
			
			Value.TangentMode = ValueElem.TangentMode;
			FloatCurve.Values.Add(Value);
		}

		const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
		FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

		const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
		FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

		FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
		FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

		SequenceTrack.TrackType = EUISequenceTrackType::Float;
		SequenceTrack.SectionIndex = UISequenceComp->FloatCurves.Emplace(FloatCurve);
		check(SequenceTrack.SectionIndex < UINT16_MAX);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceVectorTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneVectorTrack* VectorTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = VectorTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = VectorTrack->GetTrackName();
	
	if (const UMovieSceneVectorSection* VectorSection = Cast<UMovieSceneVectorSection>(Sections[0]))
	{
		FUISequenceVectorCurve VectorCurve;
		
		const int32 ChannelsUsed = VectorSection->GetChannelsUsed();
		for (int32 Index = 0; Index < ChannelsUsed; ++Index)
		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = VectorSection->GetChannel(Index);

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;

				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			if (Index == 0)
			{
				VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
				check(VectorCurve.X < UINT16_MAX);
			}
			else if (Index == 1)
			{
				VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
				check(VectorCurve.Y < UINT16_MAX);
			}
			else if (Index == 2)
			{
				VectorCurve.Z = UISequenceComp->FloatCurves.Emplace(FloatCurve);
				check(VectorCurve.Z < UINT16_MAX);
			}
			else if (Index == 3)
			{
				VectorCurve.W = UISequenceComp->FloatCurves.Emplace(FloatCurve);
				check(VectorCurve.W < UINT16_MAX);
			}
		}

		if (ChannelsUsed == 2)
		{
			VectorCurve.SectionType = EUISequenceVectorSectionType::Vector2;
		}
		else if (ChannelsUsed == 3)
		{
			VectorCurve.SectionType = EUISequenceVectorSectionType::Vector3;
		}
		else if (ChannelsUsed == 4)
		{
			VectorCurve.SectionType = EUISequenceVectorSectionType::Vector4;
		}
		
		SequenceTrack.TrackType = EUISequenceTrackType::Vector;
		SequenceTrack.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
		check(SequenceTrack.SectionIndex < UINT16_MAX);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceIntTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneIntegerTrack* IntTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = IntTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;
	
	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = IntTrack->GetTrackName();
			
	if (const UMovieSceneIntegerSection* IntSection = Cast<UMovieSceneIntegerSection>(Sections[0]))
	{
		const FMovieSceneIntegerChannel& IntChannel= IntSection->GetChannel();
		
		FUISequenceIntCurve IntCurve;
		
		for (const auto& TimeElem : IntChannel.GetTimes())
		{
			const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
			IntCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
		}

		for (const auto& ValueElem : IntChannel.GetValues())
		{
			IntCurve.Values.Add(ValueElem);
		}

		const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneIntegerChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
		IntCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&IntChannel);

		const auto DefaultValueProperty = CastField<FIntProperty>(FMovieSceneIntegerChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
		IntCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&IntChannel);;

		SequenceTrack.TrackType = EUISequenceTrackType::Int;
		SequenceTrack.SectionIndex = UISequenceComp->IntCurves.Emplace(IntCurve);
		check(SequenceTrack.SectionIndex < UINT16_MAX);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceByteTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneByteTrack* ByteTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = ByteTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;
	
	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = ByteTrack->GetTrackName();
			
	if (const UMovieSceneByteSection* ByteSection = Cast<UMovieSceneByteSection>(Sections[0]))
	{
		const FMovieSceneByteChannel& ByteChannel= ByteSection->ByteCurve;
		
		FUISequenceIntCurve IntCurve;
		
		for (const auto& TimeElem : ByteChannel.GetTimes())
		{
			const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
			IntCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
		}

		for (const auto& ValueElem : ByteChannel.GetValues())
		{
			IntCurve.Values.Add(ValueElem);
		}

		const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneByteChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
		IntCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&ByteChannel);

		const auto DefaultValueProperty = CastField<FByteProperty>(FMovieSceneByteChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
		IntCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&ByteChannel);;

		SequenceTrack.TrackType = EUISequenceTrackType::Byte;
		SequenceTrack.SectionIndex = UISequenceComp->IntCurves.Emplace(IntCurve);
		check(SequenceTrack.SectionIndex < UINT16_MAX);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceColorTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
                                                     const UMovieSceneColorTrack* ColorTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = ColorTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = ColorTrack->GetTrackName();
	
	if (const UMovieSceneColorSection* ColorSection = Cast<UMovieSceneColorSection>(Sections[0]))
	{
		FUISequenceVectorCurve VectorCurve;

		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = ColorSection->GetRedChannel();

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;

				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
			check(VectorCurve.X < UINT16_MAX);
		}

		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = ColorSection->GetGreenChannel();

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;

				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
			check(VectorCurve.Y < UINT16_MAX);
		}

		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = ColorSection->GetBlueChannel();

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;

				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			VectorCurve.Z = UISequenceComp->FloatCurves.Emplace(FloatCurve);
			check(VectorCurve.Z < UINT16_MAX);
		}

		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = ColorSection->GetAlphaChannel();

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;
				
				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			VectorCurve.W = UISequenceComp->FloatCurves.Emplace(FloatCurve);
			check(VectorCurve.W < UINT16_MAX);
		}

		VectorCurve.SectionType = EUISequenceVectorSectionType::LinearColor;
		
		SequenceTrack.TrackType = EUISequenceTrackType::Vector;
		SequenceTrack.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
		check(SequenceTrack.SectionIndex < UINT16_MAX);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceRotatorTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneRotatorTrack* RotatorTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = RotatorTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = RotatorTrack->GetTrackName();
	
	if (const UMovieSceneRotatorSection* RotatorSection = Cast<UMovieSceneRotatorSection>(Sections[0]))
	{
		FUISequenceVectorCurve VectorCurve;

		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = RotatorSection->GetRollChannel();

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;

				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
			check(VectorCurve.X < UINT16_MAX);
		}

		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = RotatorSection->GetPitchChannel();

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;

				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
			check(VectorCurve.Y < UINT16_MAX);
		}

		{
			FUISequenceFloatCurve FloatCurve;

			const FMovieSceneFloatChannel& FloatChannel = RotatorSection->GetYawChannel();

			for (const auto& TimeElem : FloatChannel.GetTimes())
			{
				const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
				FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
			}

			for (const auto& ValueElem : FloatChannel.GetValues())
			{
				FUISequenceFloatValue Value;
				Value.Tangent = ValueElem.Tangent;
				Value.Value = ValueElem.Value;

				if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
				{
					Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
				}
				else
				{
					Value.InterpMode = ValueElem.InterpMode;
				}
			
				Value.TangentMode = ValueElem.TangentMode;
				FloatCurve.Values.Add(Value);
			}

			const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
			FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
			FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

			FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
			FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

			VectorCurve.Z = UISequenceComp->FloatCurves.Emplace(FloatCurve);
			check(VectorCurve.Z < UINT16_MAX);
		}
		
		VectorCurve.SectionType = EUISequenceVectorSectionType::Rotator;
		
		SequenceTrack.TrackType = EUISequenceTrackType::Vector;
		SequenceTrack.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
		check(SequenceTrack.SectionIndex < UINT16_MAX);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceTransformTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneTransformTrack* TransformTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = TransformTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = TransformTrack->GetTrackName();
	
	if (const UMovieScene3DTransformSection* TransformSection = Cast<UMovieScene3DTransformSection>(Sections[0]))
	{
		FUISequenceTransformCurve TransformCurve;
		
		EMovieSceneTransformChannel MaskChannels = TransformSection->GetMask().GetChannels();
		
		{
			const auto TranslationProperty =  CastField<FStructProperty>(UMovieScene3DTransformSection::StaticClass()->FindPropertyByName(TEXT("Translation")));
			for (int32 Index = 0; Index < 3; ++Index)
			{
				FMovieSceneFloatChannel* TranslationValue = (FMovieSceneFloatChannel*)TranslationProperty->ContainerPtrToValuePtr<uint8>(TransformSection, Index);

				FUISequenceFloatCurve FloatCurve;

				FMovieSceneFloatChannel& FloatChannel = *TranslationValue;

				if (Index == 0 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::TranslationX))
				{
					continue;
				}
				else if (Index == 1 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::TranslationY))
				{
					continue;
				}
				else if (Index == 2 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::TranslationZ))
				{
					continue;
				}

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

				if (Index == 0)
				{
					TransformCurve.LocationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.LocationX < UINT16_MAX);
				}
				else if (Index == 1)
				{
					TransformCurve.LocationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.LocationY < UINT16_MAX);
				}
				else if (Index == 2)
				{
					TransformCurve.LocationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.LocationZ < UINT16_MAX);
				}
			}
		}

		{
			const auto RotationProperty =  CastField<FStructProperty>(UMovieScene3DTransformSection::StaticClass()->FindPropertyByName(TEXT("Rotation")));
			for (int32 Index = 0; Index < 3; ++Index)
			{
				FMovieSceneFloatChannel* RotationValue = (FMovieSceneFloatChannel*)RotationProperty->ContainerPtrToValuePtr<uint8>(TransformSection, Index);

				FUISequenceFloatCurve FloatCurve;

				FMovieSceneFloatChannel& FloatChannel = *RotationValue;

				if (Index == 0 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::RotationX))
				{
					continue;
				}
				else if (Index == 1 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::RotationY))
				{
					continue;
				}
				else if (Index == 2 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::RotationZ))
				{
					continue;
				}

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

				if (Index == 0)
				{
					TransformCurve.RotationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.RotationX < UINT16_MAX);
				}
				else if (Index == 1)
				{
					TransformCurve.RotationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.RotationY < UINT16_MAX);
				}
				else if (Index == 2)
				{
					TransformCurve.RotationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.RotationZ < UINT16_MAX);
				}
			}
		}

		{
			const auto ScaleProperty =  CastField<FStructProperty>(UMovieScene3DTransformSection::StaticClass()->FindPropertyByName(TEXT("Scale")));
			for (int32 Index = 0; Index < 3; ++Index)
			{
				FMovieSceneFloatChannel* ScaleValue = (FMovieSceneFloatChannel*)ScaleProperty->ContainerPtrToValuePtr<uint8>(TransformSection, Index);

				FUISequenceFloatCurve FloatCurve;

				FMovieSceneFloatChannel& FloatChannel = *ScaleValue;

				if (Index == 0 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::ScaleX))
				{
					continue;
				}
				else if (Index == 1 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::ScaleY))
				{
					continue;
				}
				else if (Index == 2 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::ScaleZ))
				{
					continue;
				}

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

				if (Index == 0)
				{
					TransformCurve.ScaleX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.ScaleX < UINT16_MAX);
				}
				else if (Index == 1)
				{
					TransformCurve.ScaleY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.ScaleY < UINT16_MAX);
				}
				else if (Index == 2)
				{
					TransformCurve.ScaleZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.ScaleZ < UINT16_MAX);
				}
			}
		}
		
		SequenceTrack.TrackType = EUISequenceTrackType::Transform;
		SequenceTrack.SectionIndex = UISequenceComp->TransformCurves.Emplace(TransformCurve);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequence3DTransformTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieScene3DTransformTrack* TransformTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = TransformTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = TransformTrack->GetTrackName();
	
	if (const UMovieScene3DTransformSection* TransformSection = Cast<UMovieScene3DTransformSection>(Sections[0]))
	{
		FUISequenceTransformCurve TransformCurve;
		
		EMovieSceneTransformChannel MaskChannels = TransformSection->GetMask().GetChannels();
		
		{
			const auto TranslationProperty =  CastField<FStructProperty>(UMovieScene3DTransformSection::StaticClass()->FindPropertyByName(TEXT("Translation")));
			for (int32 Index = 0; Index < 3; ++Index)
			{
				FMovieSceneFloatChannel* TranslationValue = (FMovieSceneFloatChannel*)TranslationProperty->ContainerPtrToValuePtr<uint8>(TransformSection, Index);

				FUISequenceFloatCurve FloatCurve;

				FMovieSceneFloatChannel& FloatChannel = *TranslationValue;

				if (Index == 0 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::TranslationX))
				{
					continue;
				}
				else if (Index == 1 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::TranslationY))
				{
					continue;
				}
				else if (Index == 2 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::TranslationZ))
				{
					continue;
				}

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

				if (Index == 0)
				{
					TransformCurve.LocationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.LocationX < UINT16_MAX);
				}
				else if (Index == 1)
				{
					TransformCurve.LocationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.LocationY < UINT16_MAX);
				}
				else if (Index == 2)
				{
					TransformCurve.LocationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.LocationZ < UINT16_MAX);
				}
			}
		}

		{
			const auto RotationProperty =  CastField<FStructProperty>(UMovieScene3DTransformSection::StaticClass()->FindPropertyByName(TEXT("Rotation")));
			for (int32 Index = 0; Index < 3; ++Index)
			{
				FMovieSceneFloatChannel* RotationValue = (FMovieSceneFloatChannel*)RotationProperty->ContainerPtrToValuePtr<uint8>(TransformSection, Index);

				FUISequenceFloatCurve FloatCurve;

				FMovieSceneFloatChannel& FloatChannel = *RotationValue;

				if (Index == 0 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::RotationX))
				{
					continue;
				}
				else if (Index == 1 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::RotationY))
				{
					continue;
				}
				else if (Index == 2 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::RotationZ))
				{
					continue;
				}

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

				if (Index == 0)
				{
					TransformCurve.RotationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.RotationX < UINT16_MAX);
				}
				else if (Index == 1)
				{
					TransformCurve.RotationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.RotationY < UINT16_MAX);
				}
				else if (Index == 2)
				{
					TransformCurve.RotationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.RotationZ < UINT16_MAX);
				}
			}
		}

		{
			const auto ScaleProperty =  CastField<FStructProperty>(UMovieScene3DTransformSection::StaticClass()->FindPropertyByName(TEXT("Scale")));
			for (int32 Index = 0; Index < 3; ++Index)
			{
				FMovieSceneFloatChannel* ScaleValue = (FMovieSceneFloatChannel*)ScaleProperty->ContainerPtrToValuePtr<uint8>(TransformSection, Index);

				FUISequenceFloatCurve FloatCurve;

				FMovieSceneFloatChannel& FloatChannel = *ScaleValue;

				if (Index == 0 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::ScaleX))
				{
					continue;
				}
				else if (Index == 1 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::ScaleY))
				{
					continue;
				}
				else if (Index == 2 && !EnumHasAllFlags(MaskChannels, EMovieSceneTransformChannel::ScaleZ))
				{
					continue;
				}

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

				if (Index == 0)
				{
					TransformCurve.ScaleX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.ScaleX < UINT16_MAX);
				}
				else if (Index == 1)
				{
					TransformCurve.ScaleY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.ScaleY < UINT16_MAX);
				}
				else if (Index == 2)
				{
					TransformCurve.ScaleZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(TransformCurve.ScaleZ < UINT16_MAX);
				}
			}
		}
		
		SequenceTrack.TrackType = EUISequenceTrackType::Transform;
		SequenceTrack.SectionIndex = UISequenceComp->TransformCurves.Emplace(TransformCurve);
	}

	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceUIMaterialTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneUIComponentMaterialTrack* UIMaterialTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = UIMaterialTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = UIMaterialTrack->GetTrackName();
	
	if (const UMovieSceneParameterSection* ParameterSection = Cast<UMovieSceneParameterSection>(Sections[0]))
	{
		FUIMaterialParameterCurve MaterialParameterCurve;
		
		MaterialParameterCurve.MaterialIndex = UIMaterialTrack->GetMaterialIndex();
		
		// bool
		{
			for (const auto& BoolParameters : ParameterSection->GetBoolParameterNamesAndCurves())
			{
				const FMovieSceneBoolChannel& BoolChannel= BoolParameters.ParameterCurve;
		
				FUISequenceBoolCurve BoolCurve;
		
				for (const auto& TimeElem : BoolChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					BoolCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : BoolChannel.GetValues())
				{
					BoolCurve.Values.Add(ValueElem);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneBoolChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				BoolCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&BoolChannel);

				const auto DefaultValueProperty = CastField<FBoolProperty>(FMovieSceneBoolChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				BoolCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&BoolChannel);;
				
				FUIMaterialParameterValue BoolParameter;
				BoolParameter.ParameterName = BoolParameters.ParameterName;
				BoolParameter.SectionIndex = UISequenceComp->BoolCurves.Emplace(BoolCurve);
				check(BoolParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Bools.Emplace(BoolParameter);
			}
		}

		// Scalar
		{
			for (const auto& ScalarParameters : ParameterSection->GetScalarParameterNamesAndCurves())
			{
				const FMovieSceneFloatChannel& FloatChannel = ScalarParameters.ParameterCurve;
		
				FUISequenceFloatCurve FloatCurve;

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;
				
				FUIMaterialParameterValue ScalarParameter;
				ScalarParameter.ParameterName = ScalarParameters.ParameterName;
				ScalarParameter.SectionIndex = UISequenceComp->FloatCurves.Emplace(FloatCurve);
				check(ScalarParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Scalars.Emplace(ScalarParameter);
			}
		}

		// Vector2D
		{
			for (const auto& Vector2DParameters : ParameterSection->GetVector2DParameterNamesAndCurves())
			{
				FUISequenceVectorCurve VectorCurve;
				
				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = Vector2DParameters.XCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.X < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = Vector2DParameters.YCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Y < UINT16_MAX);
				}

				VectorCurve.SectionType = EUISequenceVectorSectionType::Vector2;

				FUIMaterialParameterValue Vector2DParameter;
				Vector2DParameter.ParameterName = Vector2DParameters.ParameterName;
				Vector2DParameter.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
				check(Vector2DParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Vector2Ds.Emplace(Vector2DParameter);
			}
		}

		// Vector
		{
			for (const auto& VectorParameters : ParameterSection->GetVectorParameterNamesAndCurves())
			{
				FUISequenceVectorCurve VectorCurve;
				
				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = VectorParameters.XCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.X < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = VectorParameters.YCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Y < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = VectorParameters.ZCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Z = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Z < UINT16_MAX);
				}

				VectorCurve.SectionType = EUISequenceVectorSectionType::Vector3;

				FUIMaterialParameterValue VectorParameter;
				VectorParameter.ParameterName = VectorParameters.ParameterName;
				VectorParameter.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
				check(VectorParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Vectors.Emplace(VectorParameter);
			}
		}

		// Color
		{
			for (const auto& ColorParameters : ParameterSection->GetColorParameterNamesAndCurves())
			{
				FUISequenceVectorCurve VectorCurve;

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.RedCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.X < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.GreenCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Y < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.BlueCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Z = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Z < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.AlphaCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;
						
						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.W = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.W < UINT16_MAX);
				}

				VectorCurve.SectionType = EUISequenceVectorSectionType::LinearColor;
				
				FUIMaterialParameterValue ColorParameter;
				ColorParameter.ParameterName = ColorParameters.ParameterName;
				ColorParameter.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
				check(ColorParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Colors.Emplace(ColorParameter);
			}
		}

		// Transform
		{
			for (const auto& TransformParameters : ParameterSection->GetTransformParameterNamesAndCurves())
			{
				FUISequenceTransformCurve TransformCurve;
				
				{
					for (int32 Index = 0; Index < 3; ++Index)
					{
						const FMovieSceneFloatChannel& FloatChannel = TransformParameters.Translation[Index];

						FUISequenceFloatCurve FloatCurve;

						for (const auto& TimeElem : FloatChannel.GetTimes())
						{
							const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
							FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
						}

						for (const auto& ValueElem : FloatChannel.GetValues())
						{
							FUISequenceFloatValue Value;
							Value.Tangent = ValueElem.Tangent;
							Value.Value = ValueElem.Value;

							if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
							{
								Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
							}
							else
							{
								Value.InterpMode = ValueElem.InterpMode;
							}
					
							Value.TangentMode = ValueElem.TangentMode;
							FloatCurve.Values.Add(Value);
						}

						const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
						FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
						FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
						FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

						if (Index == 0)
						{
							TransformCurve.LocationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.LocationX < UINT16_MAX);
						}
						else if (Index == 1)
						{
							TransformCurve.LocationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.LocationY < UINT16_MAX);
						}
						else if (Index == 2)
						{
							TransformCurve.LocationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.LocationZ < UINT16_MAX);
						}
					}
				}

				{
					for (int32 Index = 0; Index < 3; ++Index)
					{
						const FMovieSceneFloatChannel& FloatChannel = TransformParameters.Rotation[Index];

						FUISequenceFloatCurve FloatCurve;
						
						for (const auto& TimeElem : FloatChannel.GetTimes())
						{
							const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
							FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
						}

						for (const auto& ValueElem : FloatChannel.GetValues())
						{
							FUISequenceFloatValue Value;
							Value.Tangent = ValueElem.Tangent;
							Value.Value = ValueElem.Value;

							if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
							{
								Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
							}
							else
							{
								Value.InterpMode = ValueElem.InterpMode;
							}
					
							Value.TangentMode = ValueElem.TangentMode;
							FloatCurve.Values.Add(Value);
						}

						const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
						FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
						FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
						FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

						if (Index == 0)
						{
							TransformCurve.RotationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.RotationX < UINT16_MAX);
						}
						else if (Index == 1)
						{
							TransformCurve.RotationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.RotationY < UINT16_MAX);
						}
						else if (Index == 2)
						{
							TransformCurve.RotationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.RotationZ < UINT16_MAX);
						}
					}
				}

				{
					for (int32 Index = 0; Index < 3; ++Index)
					{
						const FMovieSceneFloatChannel& FloatChannel = TransformParameters.Scale[Index];

						FUISequenceFloatCurve FloatCurve;
						
						for (const auto& TimeElem : FloatChannel.GetTimes())
						{
							const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
							FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
						}

						for (const auto& ValueElem : FloatChannel.GetValues())
						{
							FUISequenceFloatValue Value;
							Value.Tangent = ValueElem.Tangent;
							Value.Value = ValueElem.Value;

							if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
							{
								Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
							}
							else
							{
								Value.InterpMode = ValueElem.InterpMode;
							}
					
							Value.TangentMode = ValueElem.TangentMode;
							FloatCurve.Values.Add(Value);
						}

						const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
						FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
						FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
						FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

						if (Index == 0)
						{
							TransformCurve.ScaleX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.ScaleX < UINT16_MAX);
						}
						else if (Index == 1)
						{
							TransformCurve.ScaleY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.ScaleY < UINT16_MAX);
						}
						else if (Index == 2)
						{
							TransformCurve.ScaleZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.ScaleZ < UINT16_MAX);
						}
					}
				}

				FUIMaterialParameterValue TransformParameter;
				TransformParameter.ParameterName = TransformParameters.ParameterName;
				TransformParameter.SectionIndex = UISequenceComp->TransformCurves.Emplace(TransformCurve);
				check(TransformParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Transforms.Emplace(TransformParameter);
			}
		}
		
		SequenceTrack.TrackType = EUISequenceTrackType::MaterialParameter;
		SequenceTrack.SectionIndex = UISequenceComp->ParameterCurves.Emplace(MaterialParameterCurve);
	}
	
	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceComponentMaterialTrack(const FFrameNumber& SrcStartFrame, const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding,
	const UMovieSceneComponentMaterialTrack* ComponentMaterialTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate)
{
	const auto& Sections = ComponentMaterialTrack->GetAllSections();
	if (Sections.Num() <= 0)
		return;

	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = ComponentMaterialTrack->GetTrackName();
	
	if (const UMovieSceneParameterSection* ParameterSection = Cast<UMovieSceneParameterSection>(Sections[0]))
	{
		FUIMaterialParameterCurve MaterialParameterCurve;
		
		MaterialParameterCurve.MaterialIndex = ComponentMaterialTrack->GetMaterialIndex();
		
		// bool
		{
			for (const auto& BoolParameters : ParameterSection->GetBoolParameterNamesAndCurves())
			{
				const FMovieSceneBoolChannel& BoolChannel= BoolParameters.ParameterCurve;
		
				FUISequenceBoolCurve BoolCurve;
		
				for (const auto& TimeElem : BoolChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					BoolCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : BoolChannel.GetValues())
				{
					BoolCurve.Values.Add(ValueElem);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneBoolChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				BoolCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&BoolChannel);

				const auto DefaultValueProperty = CastField<FBoolProperty>(FMovieSceneBoolChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				BoolCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&BoolChannel);;
				
				FUIMaterialParameterValue BoolParameter;
				BoolParameter.ParameterName = BoolParameters.ParameterName;
				BoolParameter.SectionIndex = UISequenceComp->BoolCurves.Emplace(BoolCurve);
				check(BoolParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Bools.Emplace(BoolParameter);
			}
		}

		// Scalar
		{
			for (const auto& ScalarParameters : ParameterSection->GetScalarParameterNamesAndCurves())
			{
				const FMovieSceneFloatChannel& FloatChannel = ScalarParameters.ParameterCurve;
		
				FUISequenceFloatCurve FloatCurve;

				for (const auto& TimeElem : FloatChannel.GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for (const auto& ValueElem : FloatChannel.GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
			
					Value.TangentMode = ValueElem.TangentMode;
					FloatCurve.Values.Add(Value);
				}

				const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
				FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
				FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

				FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
				FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;
				
				FUIMaterialParameterValue ScalarParameter;
				ScalarParameter.ParameterName = ScalarParameters.ParameterName;
				ScalarParameter.SectionIndex = UISequenceComp->FloatCurves.Emplace(FloatCurve);
				check(ScalarParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Scalars.Emplace(ScalarParameter);
			}
		}

		// Vector2D
		{
			for (const auto& Vector2DParameters : ParameterSection->GetVector2DParameterNamesAndCurves())
			{
				FUISequenceVectorCurve VectorCurve;
				
				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = Vector2DParameters.XCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.X < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = Vector2DParameters.YCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Y < UINT16_MAX);
				}

				VectorCurve.SectionType = EUISequenceVectorSectionType::Vector2;

				FUIMaterialParameterValue Vector2DParameter;
				Vector2DParameter.ParameterName = Vector2DParameters.ParameterName;
				Vector2DParameter.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
				check(Vector2DParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Vector2Ds.Emplace(Vector2DParameter);
			}
		}

		// Vector
		{
			for (const auto& VectorParameters : ParameterSection->GetVectorParameterNamesAndCurves())
			{
				FUISequenceVectorCurve VectorCurve;
				
				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = VectorParameters.XCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.X < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = VectorParameters.YCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Y < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = VectorParameters.ZCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
				
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Z = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Z < UINT16_MAX);
				}

				VectorCurve.SectionType = EUISequenceVectorSectionType::Vector3;

				FUIMaterialParameterValue VectorParameter;
				VectorParameter.ParameterName = VectorParameters.ParameterName;
				VectorParameter.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
				check(VectorParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Vectors.Emplace(VectorParameter);
			}
		}

		// Color
		{
			for (const auto& ColorParameters : ParameterSection->GetColorParameterNamesAndCurves())
			{
				FUISequenceVectorCurve VectorCurve;

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.RedCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.X = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.X < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.GreenCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Y = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Y < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.BlueCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;

						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.Z = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.Z < UINT16_MAX);
				}

				{
					FUISequenceFloatCurve FloatCurve;

					const FMovieSceneFloatChannel& FloatChannel = ColorParameters.AlphaCurve;

					for (const auto& TimeElem : FloatChannel.GetTimes())
					{
						const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
						FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
					}

					for (const auto& ValueElem : FloatChannel.GetValues())
					{
						FUISequenceFloatValue Value;
						Value.Tangent = ValueElem.Tangent;
						Value.Value = ValueElem.Value;
						
						if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
						{
							Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
						}
						else
						{
							Value.InterpMode = ValueElem.InterpMode;
						}
					
						Value.TangentMode = ValueElem.TangentMode;
						FloatCurve.Values.Add(Value);
					}

					const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
					FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
					FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

					FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
					FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

					VectorCurve.W = UISequenceComp->FloatCurves.Emplace(FloatCurve);
					check(VectorCurve.W < UINT16_MAX);
				}

				VectorCurve.SectionType = EUISequenceVectorSectionType::LinearColor;
				
				FUIMaterialParameterValue ColorParameter;
				ColorParameter.ParameterName = ColorParameters.ParameterName;
				ColorParameter.SectionIndex = UISequenceComp->VectorCurves.Emplace(VectorCurve);
				check(ColorParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Colors.Emplace(ColorParameter);
			}
		}

		// Transform
		{
			for (const auto& TransformParameters : ParameterSection->GetTransformParameterNamesAndCurves())
			{
				FUISequenceTransformCurve TransformCurve;
				
				{
					for (int32 Index = 0; Index < 3; ++Index)
					{
						const FMovieSceneFloatChannel& FloatChannel = TransformParameters.Translation[Index];

						FUISequenceFloatCurve FloatCurve;

						for (const auto& TimeElem : FloatChannel.GetTimes())
						{
							const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
							FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
						}

						for (const auto& ValueElem : FloatChannel.GetValues())
						{
							FUISequenceFloatValue Value;
							Value.Tangent = ValueElem.Tangent;
							Value.Value = ValueElem.Value;

							if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
							{
								Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
							}
							else
							{
								Value.InterpMode = ValueElem.InterpMode;
							}
					
							Value.TangentMode = ValueElem.TangentMode;
							FloatCurve.Values.Add(Value);
						}

						const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
						FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
						FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
						FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

						if (Index == 0)
						{
							TransformCurve.LocationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.LocationX < UINT16_MAX);
						}
						else if (Index == 1)
						{
							TransformCurve.LocationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.LocationY < UINT16_MAX);
						}
						else if (Index == 2)
						{
							TransformCurve.LocationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.LocationZ < UINT16_MAX);
						}
					}
				}

				{
					for (int32 Index = 0; Index < 3; ++Index)
					{
						const FMovieSceneFloatChannel& FloatChannel = TransformParameters.Rotation[Index];

						FUISequenceFloatCurve FloatCurve;
						
						for (const auto& TimeElem : FloatChannel.GetTimes())
						{
							const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
							FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
						}

						for (const auto& ValueElem : FloatChannel.GetValues())
						{
							FUISequenceFloatValue Value;
							Value.Tangent = ValueElem.Tangent;
							Value.Value = ValueElem.Value;

							if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
							{
								Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
							}
							else
							{
								Value.InterpMode = ValueElem.InterpMode;
							}
					
							Value.TangentMode = ValueElem.TangentMode;
							FloatCurve.Values.Add(Value);
						}

						const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
						FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
						FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
						FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

						if (Index == 0)
						{
							TransformCurve.RotationX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.RotationX < UINT16_MAX);
						}
						else if (Index == 1)
						{
							TransformCurve.RotationY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.RotationY < UINT16_MAX);
						}
						else if (Index == 2)
						{
							TransformCurve.RotationZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.RotationZ < UINT16_MAX);
						}
					}
				}

				{
					for (int32 Index = 0; Index < 3; ++Index)
					{
						const FMovieSceneFloatChannel& FloatChannel = TransformParameters.Scale[Index];

						FUISequenceFloatCurve FloatCurve;
						
						for (const auto& TimeElem : FloatChannel.GetTimes())
						{
							const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
							FloatCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
						}

						for (const auto& ValueElem : FloatChannel.GetValues())
						{
							FUISequenceFloatValue Value;
							Value.Tangent = ValueElem.Tangent;
							Value.Value = ValueElem.Value;

							if (UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
							{
								Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
							}
							else
							{
								Value.InterpMode = ValueElem.InterpMode;
							}
					
							Value.TangentMode = ValueElem.TangentMode;
							FloatCurve.Values.Add(Value);
						}

						const auto bHasDefaultValueProperty = CastField<FBoolProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("bHasDefaultValue")));
						FloatCurve.bHasDefaultValue = bHasDefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						const auto DefaultValueProperty = CastField<FFloatProperty>(FMovieSceneFloatChannel::StaticStruct()->FindPropertyByName(TEXT("DefaultValue")));
						FloatCurve.DefaultValue = DefaultValueProperty->GetPropertyValue_InContainer(&FloatChannel);

						FloatCurve.PreInfinityExtrap = FloatChannel.PreInfinityExtrap;
						FloatCurve.PostInfinityExtrap = FloatChannel.PostInfinityExtrap;

						if (Index == 0)
						{
							TransformCurve.ScaleX = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.ScaleX < UINT16_MAX);
						}
						else if (Index == 1)
						{
							TransformCurve.ScaleY = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.ScaleY < UINT16_MAX);
						}
						else if (Index == 2)
						{
							TransformCurve.ScaleZ = UISequenceComp->FloatCurves.Emplace(FloatCurve);
							check(TransformCurve.ScaleZ < UINT16_MAX);
						}
					}
				}

				FUIMaterialParameterValue TransformParameter;
				TransformParameter.ParameterName = TransformParameters.ParameterName;
				TransformParameter.SectionIndex = UISequenceComp->TransformCurves.Emplace(TransformCurve);
				check(TransformParameter.SectionIndex < UINT16_MAX);
				MaterialParameterCurve.Transforms.Emplace(TransformParameter);
			}
		}
		
		SequenceTrack.TrackType = EUISequenceTrackType::MaterialParameter;
		SequenceTrack.SectionIndex = UISequenceComp->ParameterCurves.Emplace(MaterialParameterCurve);
	}
	
	Binding.Tracks.Emplace(SequenceTrack);
}

void FUISequenceCompiler::CreateUISequenceComponentAudioTrack(const FFrameNumber& SrcStartFrame,const FFrameNumber& SrcEndFrame, FUISequenceBinding& Binding, const UMovieSceneAudioTrack* AudioTrack, UUISequenceComponent* UISequenceComp, const FMovieScenePlaybackPosition PlayPosition, const FFrameRate TickResolution, const FFrameRate DisplayRate, bool bMasterTrack)
{
	const auto& Sections = AudioTrack->GetAllSections();
	if(Sections.Num() <= 0 )
		return;
	
	FUISequenceTrack SequenceTrack;
	SequenceTrack.TrackName = AudioTrack->GetTrackName();
	SequenceTrack.TrackType = EUISequenceTrackType::AudioEvent;
	
	FUISequenceAudioCurve AudioCurve;
	for(const auto& Section:Sections)
	{
		if(const auto AudioSection = Cast<UMovieSceneAudioSection>(Section))
		{
			FUISequenceAudioSection AudioCurveSection;
			AudioCurveSection.AttenuationSettings = AudioSection->GetAttenuationSettings();
			AudioCurveSection.Sound = AudioSection->GetSound();
			AudioCurveSection.StartFrameOffset = static_cast<float>(TickResolution.AsSeconds(AudioSection->GetStartOffset()));
			AudioCurveSection.ClipStartTime = static_cast<float>(TickResolution.AsSeconds(AudioSection->GetRange().GetLowerBoundValue()));
			AudioCurveSection.ClipEndTime = static_cast<float>(TickResolution.AsSeconds(AudioSection->GetRange().GetUpperBoundValue()));
			AudioCurveSection.bLooping = AudioSection->GetLooping();
			AudioCurveSection.SectionIndex = 	AudioCurve.AudioSections.Num();

			// Create Volume Curve
			{
				FUISequenceFloatCurve SoundVolumeCurve;
				for(const auto& TimeElem : AudioSection->GetSoundVolumeChannel().GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					SoundVolumeCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for(const auto& ValueElem : AudioSection->GetSoundVolumeChannel().GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if(UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
				
					Value.TangentMode = ValueElem.TangentMode;
					SoundVolumeCurve.Values.Add(Value);
				}

				AudioCurveSection.VolumeCurveIndex = UISequenceComp->FloatCurves.Emplace(SoundVolumeCurve);
			}

			// Create MultiPicker Curve
			{
				FUISequenceFloatCurve MultiPickerCurve;
				for(const auto& TimeElem : AudioSection->GetPitchMultiplierChannel().GetTimes())
				{
					const FFrameTime Time = ConvertFrameTime(TimeElem, TickResolution, DisplayRate);
					MultiPickerCurve.Times.Add(FQualifiedFrameTime(Time, PlayPosition.GetInputRate()).AsSeconds());
				}

				for(const auto& ValueElem : AudioSection->GetPitchMultiplierChannel().GetValues())
				{
					FUISequenceFloatValue Value;
					Value.Tangent = ValueElem.Tangent;
					Value.Value = ValueElem.Value;

					if(UISequenceComp->bConvertCubicToLinear && ValueElem.InterpMode == ERichCurveInterpMode::RCIM_Cubic)
					{
						Value.InterpMode = ERichCurveInterpMode::RCIM_Linear;
					}
					else
					{
						Value.InterpMode = ValueElem.InterpMode;
					}
				
					Value.TangentMode = ValueElem.TangentMode;
					MultiPickerCurve.Values.Add(Value);
				}

				AudioCurveSection.PitchMultiplierCurveIndex = UISequenceComp->FloatCurves.Emplace(MultiPickerCurve);
			}
			
			AudioCurve.AudioSections.Emplace(AudioCurveSection);
		}
	}
	
	AudioCurve.bMasterTrack = bMasterTrack;
	SequenceTrack.SectionIndex = UISequenceComp->AudioCurves.Emplace(AudioCurve);
	Binding.bMaterTrack = bMasterTrack;
	Binding.Tracks.Emplace(SequenceTrack);
}
