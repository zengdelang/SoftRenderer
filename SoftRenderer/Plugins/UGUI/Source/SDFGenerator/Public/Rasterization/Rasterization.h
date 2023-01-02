#pragma once

#include "Image/Bitmap.h"
#include "Shape/Projection.h"
#include "Shape/Shape.h"

namespace SDFGenerator
{
   /**
    * Rasterizes the shape into a monochrome bitmap.
    */
   void Rasterize(const FBitmapRef<float, 1>& Output, const FShape& Shape, const FProjection& Projection, EFillRule FillRule = EFillRule::FILL_NONZERO);
   
   /**
    * Fixes the sign of the input signed distance field, so that it matches the shape's rasterized fill.
    */
   void DistanceSignCorrection(const FBitmapRef<float, 1>& Sdf, const FShape& Shape, const FProjection& Projection, EFillRule FillRule = EFillRule::FILL_NONZERO);
   void DistanceSignCorrection(const FBitmapRef<float, 3>& Sdf, const FShape& Shape, const FProjection& projection, EFillRule FillRule = EFillRule::FILL_NONZERO);
   void DistanceSignCorrection(const FBitmapRef<float, 4>& Sdf, const FShape& Shape, const FProjection& projection, EFillRule FillRule = EFillRule::FILL_NONZERO);
   
}
