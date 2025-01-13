#pragma once

#include <functional>
#include <utility>

struct IntVec2
{
public:
	int				x = 0;
	int				y = 0;

	static IntVec2 const ZERO;
	static IntVec2 const ONE;

	static IntVec2 const NORTH;
	static IntVec2 const SOUTH;
	static IntVec2 const EAST;
	static IntVec2 const WEST;
	static IntVec2 const NORTH_EAST;
	static IntVec2 const NORTH_WEST;
	static IntVec2 const SOUTH_EAST;
	static IntVec2 const SOUTH_WEST;

public:
					IntVec2() {}
					IntVec2(IntVec2 const& copyFrom);
	explicit		IntVec2(int initialX, int initialY);
					~IntVec2() {}

	float			GetLength() const;
	int				GetTaxicabLength() const;
	int				GetLengthSquared() const;
	float			GetOrientationRadians() const;
	float			GetOrientationDegrees() const;
	IntVec2 const	GetRotated90Degrees() const;
	IntVec2 const	GetRotatedMinus90Degrees() const;

	void			Rotate90Degrees();
	void			RotateMinus90Degrees();

	IntVec2			SetFromText(char const* text);

	IntVec2	const	operator+(IntVec2 const& copyFrom);
	IntVec2 const	operator-(IntVec2 const& copyFrom);
	bool			operator==( IntVec2 const& compare ) const;	// vec2 == vec2
	bool			operator!=(const IntVec2& compare) const;
	void			operator=( IntVec2 const& copyFrom );				// vec2 = vec2

	friend bool operator<(IntVec2 const& a, IntVec2 const& b);
};

namespace std
{
	template <>
	struct hash<IntVec2>
	{
		size_t operator()(const IntVec2& vec) const
		{
			return (hash<int>()(vec.x) ^ (hash<int>()(vec.y) << 1));
		}
	};
}

struct IntVec2Hash
{
	std::size_t operator()(const IntVec2& v) const noexcept; // Perform compile-time check for exceptions
};

struct IntVec2PairHash
{
	std::size_t operator()(const std::pair<IntVec2, IntVec2>& pair) const noexcept;
};

struct IntVec2PairEqual
{
	bool operator()(const std::pair<IntVec2, IntVec2>& leftHash, const std::pair<IntVec2, IntVec2>& rightHash) const noexcept;
};