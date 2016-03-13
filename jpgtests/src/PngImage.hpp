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

#ifndef PNG_IMAGE_HPP
#define PNG_IMAGE_HPP

#include "planes/PlaneBase.hpp"

class QImage;
using QRgb=unsigned;

namespace AnimeRaster{
	
struct Color{
	double x, y, z;
	Color() : x(0.0), y(0.0), z(0.0) { }
	Color( double x, double y, double z ) : x(x), y(y), z(z) { }
	Color( QRgb rgb );
	Color rgbToYcbcr() const;
	
};

class PngImage {
	public:
		Overmix::PlaneBase<Color> ycbcr;
	
	public:
		PngImage( QImage img );
};

}

#endif
