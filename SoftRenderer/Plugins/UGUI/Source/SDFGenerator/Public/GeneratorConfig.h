#pragma once

namespace SDFGenerator
{
	/**
	 * The configuration of the distance field generator algorithm.
	 */
	struct FGeneratorConfig
	{
		/// Specifies whether to use the version of the algorithm that supports overlapping contours with the same winding. May be set to false to improve performance when no such contours are present.
		bool bOverlapSupport;

		explicit FGeneratorConfig(bool bInOverlapSupport = true) : bOverlapSupport(bInOverlapSupport)
		{
			
		}
	};
}
