#pragma once

#include "ISequencerSection.h"

/**
 * A movie scene section for material parameters.
 */
class FUIParameterSection
	: public FSequencerSection
{
public:
	FUIParameterSection(UMovieSceneSection& InSectionObject)
		: FSequencerSection(InSectionObject)
	{ }

public:
	//~ ISequencerSection interface
	virtual FReply OnKeyDoubleClicked(const TArray<FKeyHandle>& KeyHandles) override;
	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override;
	virtual bool RequestDeleteCategory(const TArray<FName>& CategoryNamePath) override;
	virtual bool RequestDeleteKeyArea(const TArray<FName>& KeyAreaNamePath) override;
	
};
