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

#include "Encoders.hpp"
#include "../../lib/src/transform.hpp"

#include "JpegImage.hpp"
#include "Converters.hpp"
#include "Converters.hpp"

#include <iostream>

using namespace AnimeRaster;

void combine( std::vector<uint8_t>& output, const std::vector<uint8_t>& add ){
	for( auto val : add )
		output.push_back( val );
}

int trimZeroes( std::vector<uint8_t>& arr ){
	int end = arr.size() - 1;
	for( ; end>0 && arr[end]==0; end-- )
		;
	//std::cout << "Reduced: " << arr.size()-1 << " to " << end << std::endl;
	arr.resize( end + 1 );
	return arr.size();
}

std::vector<uint8_t> simplePlaneEncode( const JpegPlane& p ){
	std::vector<uint8_t> sizes;
	std::vector<uint8_t> output;
	
	for( auto pos : getZigZagPattern() ){
		auto data = packTo16bit( linearizePlane( coeffsFromOffset( p, pos ) ) );
		combine( sizes, packTo16bit( { trimZeroes( data ) } ) );
		combine( output, data );
	}
	
	combine( sizes, output );
	return sizes;
}

std::vector<uint8_t> AnimeRaster::simpleJpegEncode( const JpegImage& img ){
	std::vector<uint8_t> output;
	
	for( auto& plane : img.planes )
		combine( output, simplePlaneEncode( plane ) );
	
	return lzmaCompress( output );
}

