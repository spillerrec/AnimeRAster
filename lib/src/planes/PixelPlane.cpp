/*	This file is part of AnimeRAster.

	AnimeRAster is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AnimeRAster is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AnimeRAster.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PixelPlane.hpp"

using namespace std;


Pixel Pixel::encode( int r, int g, int b, int transform ){
	Pixel pixel( r, g, b );
	
	if( transform == 0 )
		return pixel;;
	
	transform--;
	int main = transform / 3;
	int second = ( main != 0 ) ? 0 : 1;
	int third = ( main != 1 && second != 1 ) ? 1 : 2;
	
	Pixel enc = pixel;
	switch( transform % 3 ){
		case 0:
				enc.color[second] -= enc.color[main];
				enc.color[third] -= enc.color[main];
			break;
		case 1:
				enc.color[second] -= enc.color[main];
				enc.color[third] -= enc.color[second];
			break;
		case 2:
				enc.color[second] -= enc.color[third];
				enc.color[third] -= enc.color[main];
			break;
	};
	//TODO: reorder, i.e. swap( main, 0 )
	return enc;
}
Pixel Pixel::decode( const vector<uint8_t>& data, unsigned& pos, int transform ){
	//Read
	Pixel pixel;
	for( unsigned p=0; p<3; p++ ){
		//TODO: multi-byte stuff...
		pixel.color[p] = data[pos++];
	}
	
	if( transform == 0 )
		return pixel;
	
	transform--;
	int main = transform / 3;
	int second = ( main != 0 ) ? 0 : 1;
	int third = ( main != 1 && second != 1 ) ? 1 : 2;
	
	//Revert color decorrelation
	switch( transform % 3 ){
		case 0:
				pixel.color[second] += pixel.color[main];
				pixel.color[third] += pixel.color[main];
			break;
		case 1:
				pixel.color[second] += pixel.color[main];
				pixel.color[third] += pixel.color[second];
			break;
		case 2:
				pixel.color[third] += pixel.color[main];
				pixel.color[second] += pixel.color[third];
			break;
	};
	//TODO: reorder?
	return pixel;
}


void PixelPlane::load( const vector<uint8_t>& data ){
	unsigned pos = 0;
	
	unsigned block_size = data[pos++];
	vector<uint8_t> types;
	vector<uint8_t> colors;
	
	//Init variables
	unsigned limit = pow( 2, depth );
	
	//Load types and color information
	unsigned blocks_count = ceil( width / (double)block_size ) * ceil( height / (double)block_size );
	for( unsigned i=0; i<blocks_count; i++ )
		types.push_back( data[pos++] );
	for( unsigned i=0; i<blocks_count; i++ )
		colors.push_back( data[pos++] );
	
	//Read blocks
	unsigned type_pos = 0, color_pos = 0;
	
	for( unsigned iy=0; iy<height; iy+=block_size )
		for( unsigned ix=0; ix<width; ix+=block_size ){
			auto type = types[type_pos++];
			auto color = colors[color_pos++];
			
			//Translate settings
			auto f = getFilter( (Type)type );
			
			//Truncate sizes to not go out of the plane
			unsigned b_w = min( ix+block_size, width );
			unsigned b_h = min( iy+block_size, height );
			
			for( unsigned jy=iy; jy < b_h; jy++ )
				for( unsigned jx=ix; jx < b_w; jx++ ){
					auto pixel = Pixel::decode( data, pos, color );
					
					//Revert filtering
					auto val =  pixel + (this->*f)( jx, jy );
					val.trunc( limit );
					setValue( jx, jy, val );
				}
			
		}
		
}

