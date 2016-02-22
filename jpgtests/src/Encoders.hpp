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

#ifndef ENCODERS_HPP
#define ENCODERS_HPP

#include <vector>
#include <stdint.h>

namespace AnimeRaster{
	
class JpegImage;

std::vector<uint8_t> planarJpegEncode( const JpegImage& );
std::vector<uint8_t> interleavedJpegEncode( const JpegImage& );
std::vector<uint8_t> blockJpegEncode( const JpegImage& );
std::vector<uint8_t> multiJpegEncode( const std::vector<JpegImage>& );

}

#endif