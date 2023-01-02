#include "SDFGenerator.h"
#include "Shape/ContourCombiners.h"
#include "SignedDistance/ShapeDistanceFinder.h"

namespace SDFGenerator
{
	template <typename DistanceType>
	class FDistancePixelConversion;

	template <>
	class FDistancePixelConversion<double>
	{
		double InvRange;
		
	public:
		typedef FBitmapRef<float, 1> BitmapRefType;

		explicit FDistancePixelConversion(double Range) : InvRange(1 / Range)
		{
		}

		void operator()(float* Pixels, double Distance) const
		{
			*Pixels = static_cast<float>(InvRange * Distance + .5);
		}
	};
	
	template <class ContourCombiner>
	void GenerateDistanceField(
		const typename FDistancePixelConversion<typename ContourCombiner::DistanceType>::BitmapRefType& Output,
		const FShape& Shape, const FProjection& Projection, double Range)
	{
		FDistancePixelConversion<typename ContourCombiner::DistanceType> DistancePixelConversion(Range);
		
#ifdef SDF_GENERATOR_USE_OPENMP
		#pragma omp parallel
#endif
		
		{
			FShapeDistanceFinder<ContourCombiner> DistanceFinder(Shape);
			bool bRightToLeft = false;
			
#ifdef SDF_GENERATOR_USE_OPENMP
			#pragma omp for
#endif
			
			for (int32 Y = 0; Y < Output.Height; ++Y)
			{
				int32 Row = Shape.bInverseYAxis ? Output.Height - Y - 1 : Y;
				for (int32 Col = 0; Col < Output.Width; ++Col)
				{
					int32 X = bRightToLeft ? Output.Width - Col - 1 : Col;
					FPoint2D P = Projection.Unproject(FPoint2D(X + .5, Y + .5));
					typename ContourCombiner::DistanceType Distance = DistanceFinder.Distance(P);
					DistancePixelConversion(Output(X, Row), Distance);
				}
				bRightToLeft = !bRightToLeft;
			}
		}
	}

	void GenerateSDF(const FBitmapRef<float, 1>& Output, const FShape& Shape, const FProjection& Projection, double Range, const FGeneratorConfig& Config)
	{
		if (Config.bOverlapSupport)
		{
			GenerateDistanceField<FOverlappingContourCombiner<FTrueDistanceSelector>>(Output, Shape, Projection, Range);
		}
		else
		{
			GenerateDistanceField<FSimpleContourCombiner<FTrueDistanceSelector>>(Output, Shape, Projection, Range);
		}
	}
}
