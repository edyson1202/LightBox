#pragma once

#include "Utils.h"

namespace LightBox
{
	struct Interval
	{
	public:
		// Default interval is empty
		Interval()
			: min(infinity), max(-infinity) {}
		Interval(float min, float max)
			: min(min), max(max) {}
		Interval(const Interval& a, const Interval& b)
			: min(fmin(a.min, b.min)), max(fmax(a.max, b.max)) {}

		bool Contains(float x) const { return min <= x && x <= max; }
		bool Surrounds(float x) const { return min < x && x < max; }

		static const Interval empty, universe;

	public:
		float min, max;
	};

	const static Interval empty(+infinity, -infinity);
	const static Interval universe(-infinity, +infinity);
}