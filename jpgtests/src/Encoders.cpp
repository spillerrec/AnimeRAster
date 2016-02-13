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

CoeffPlane substractLeft( const CoeffPlane& p ){
	CoeffPlane out( p );
	for( unsigned iy=0;iy<p.get_height(); iy++ ){
		auto in = p.scan_line( iy );
		auto ot = out.scan_line( iy );
		for( unsigned i=1; i<p.get_width(); i++ )
			ot[i] = in[i-1] - in[i];
	}
	return out;
}

std::vector<CoeffPlane> transformPlanes( const JpegPlane& p ){
	std::vector<CoeffPlane> out;
	
	for( auto pos : getZigZagPattern() ){
		auto coeffs = coeffsFromOffset( p, pos );
		if( pos.x == 0 && pos.y == 0 )
			coeffs = substractLeft( coeffs );
		out.push_back( coeffs );
	}
	
	return out;
}

void encodeCoeffs( const CoeffPlane& p, std::vector<uint8_t>& info, std::vector<uint8_t>& bulk ){
	auto data = packTo16bit( linearizePlane( p ) );
	auto count = trimZeroes( data );
	combine( info, packTo16bit( { count } ) );
	combine( bulk, data );
}

std::vector<uint8_t> AnimeRaster::interleavedJpegEncode( const JpegImage& img ){
	std::vector<uint8_t> info;
	std::vector<uint8_t> bulk;
	//TODO: Quant-tables
	
	std::vector<std::vector<CoeffPlane>> data;
	for( auto& p : img.planes )
		data.push_back( transformPlanes( p ) );
	
	for( unsigned ic=0; ic<data[0].size(); ic++ )
		for( unsigned ip=0; ip<data.size(); ip++ )
			encodeCoeffs( data[ip][ic], info, bulk );
	
	combine( info, bulk );
	return lzmaCompress( info );
}

std::vector<uint8_t> AnimeRaster::planarJpegEncode( const JpegImage& img ){
	std::vector<uint8_t> info;
	std::vector<uint8_t> bulk;
	//TODO: Quant-tables
	
	for( auto& plane : img.planes )
		for( auto coeffs : transformPlanes( plane ) )
			encodeCoeffs( coeffs, info, bulk );
	combine( info, bulk );
	
	return lzmaCompress( info );
}
