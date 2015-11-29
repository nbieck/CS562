///////////////////////////////////////////////////////////////////////////////////////
// Hi-Z ray tracing methods
///////////////////////////////////////////////////////////////////////////////////////

static const float2 hiZSize = cb_screenSize; // not sure if correct - this is mip level 0 size

float3 intersectDepthPlane(float3 o, float3 d, float t)
{
	return o + d * t;
}

float2 getCell(float2 ray, float2 cellCount)
{
	// does this need to be floor, or does it need fractional part - i think cells are meant to be whole pixel values (integer values) but not sure
	return floor(ray * cellCount);
}

float3 intersectCellBoundary(float3 o, float3 d, float2 cellIndex, float2 cellCount, float2 crossStep, float2 crossOffset)
{
	float2 index = cellIndex + crossStep;
	index /= cellCount;
	index += crossOffset;
	float2 delta = index - o.xy;
	delta /= d.xy;
	float t = min(delta.x, delta.y);
	return intersectDepthPlane(o, d, t);
}

float getMinimumDepthPlane(float2 ray, float level, float rootLevel)
{
	// not sure why we need rootLevel for this
	return hiZBuffer.SampleLevel(sampPointClamp, ray.xy, level).r;
}

float2 getCellCount(float level, float rootLevel)
{
	// not sure why we need rootLevel for this
	float2 div = level == 0.0f ? 1.0f : exp2(level);
	return cb_screenSize / div;
}

bool crossedCellBoundary(float2 cellIdxOne, float2 cellIdxTwo)
{
	return cellIdxOne.x != cellIdxTwo.x || cellIdxOne.y != cellIdxTwo.y;
}

float3 hiZTrace(float3 p, float3 v)
{
	const float rootLevel = float(cb_mipCount) - 1.0f; // convert to 0-based indexing
	
	float level = HIZ_START_LEVEL;

	uint iterations = 0u;

	// get the cell cross direction and a small offset to enter the next cell when doing cell crossing
	float2 crossStep = float2(v.x >= 0.0f ? 1.0f : -1.0f, v.y >= 0.0f ? 1.0f : -1.0f);
	float2 crossOffset = float2(crossStep.xy * HIZ_CROSS_EPSILON.xy);
	crossStep.xy = saturate(crossStep.xy);

	// set current ray to original screen coordinate and depth
	float3 ray = p.xyz;

	// scale vector such that z is 1.0f (maximum depth)
	float3 d = v.xyz / v.z;

	// set starting point to the point where z equals 0.0f (minimum depth)
	float3 o = intersectDepthPlane(p, d, -p.z);

	// cross to next cell to avoid immediate self-intersection
	float2 rayCell = getCell(ray.xy, hiZSize.xy);
	ray = intersectCellBoundary(o, d, rayCell.xy, hiZSize.xy, crossStep.xy, crossOffset.xy);

	while(level >= HIZ_STOP_LEVEL && iterations < MAX_ITERATIONS)
	{
		// get the minimum depth plane in which the current ray resides
		float minZ = getMinimumDepthPlane(ray.xy, level, rootLevel);
		
		// get the cell number of the current ray
		const float2 cellCount = getCellCount(level, rootLevel);
		const float2 oldCellIdx = getCell(ray.xy, cellCount);

		// intersect only if ray depth is below the minimum depth plane
		float3 tmpRay = intersectDepthPlane(o, d, max(ray.z, minZ));

		// get the new cell number as well
		const float2 newCellIdx = getCell(tmpRay.xy, cellCount);

		// if the new cell number is different from the old cell number, a cell was crossed
		if(crossedCellBoundary(oldCellIdx, newCellIdx))
		{
			// intersect the boundary of that cell instead, and go up a level for taking a larger step next iteration
			tmpRay = intersectCellBoundary(o, d, oldCellIdx, cellCount.xy, crossStep.xy, crossOffset.xy); //// NOTE added .xy to o and d arguments
			level = min(HIZ_MAX_LEVEL, level + 2.0f);
		}

		ray.xyz = tmpRay.xyz;

		// go down a level in the hi-z buffer
		--level;

		++iterations;
	}

	return ray;
}
