#pragma once

#include "GeneratorConfig.h"
#include "Image/Bitmap.h"
#include "Shape/Projection.h"
#include "Shape/Shape.h"

namespace SDFGenerator
{
	/**
	 * Generates a conventional single-channel signed distance field.
	 */
	void GenerateSDF(const FBitmapRef<float, 1>& Output, const FShape& Shape, const FProjection& Projection, double Range, const FGeneratorConfig& Config = FGeneratorConfig());
}
