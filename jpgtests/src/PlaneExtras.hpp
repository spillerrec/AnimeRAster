/*
	This file is part of AnimeRaster.

	AnimeRaster is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AnimeRaster is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AnimeRaster.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLANE_EXTRAS_HPP
#define PLANE_EXTRAS_HPP

#include "planes/PlaneBase.hpp"

#include <limits>

class QImage;

namespace color{
	inline double asDouble( uint8_t val ){ return val / 255.0; }
	inline uint8_t fromDouble( double val ){
		auto truncated = val>1.0 ? 1.0 : ( val<0.0 ? 0.0 : val );
		return std::round( truncated * 255.0 );
	}
}

namespace AnimeRaster{

using Plane      = Overmix::PlaneBase<uint8_t>;
using CoeffPlane = Overmix::PlaneBase<int16_t>;

template<typename T>
T maxPlaneValue( const Overmix::PlaneBase<T>& plane ){
	T max_value = std::numeric_limits<T>::min();
	for( auto line : plane )
		for( auto value : line )
			max_value = std::max( max_value, value );
	return max_value;
}

template<typename T>
T minPlaneValue( const Overmix::PlaneBase<T>& plane ){
	T min_value = std::numeric_limits<T>::max();
	for( auto line : plane )
		for( auto value : line )
			min_value = std::min( min_value, value );
	return min_value;
}

Plane normalized( const CoeffPlane& plane );
QImage planeToQImage( const Plane& p );

}

#endif
