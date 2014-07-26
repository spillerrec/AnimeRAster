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

#include "ValuePlane.hpp"

using namespace std;


void ValuePlane::loadBlocks( const vector<uint8_t>& data, uint8_t block_size, unsigned type_pos, unsigned& pos ){
	//Init variables
	unsigned limit = pow( 2, depth );
	
	//Read blocks
	for( unsigned iy=0; iy<height; iy+=block_size )
		for( unsigned ix=0; ix<width; ix+=block_size ){
			//Translate settings
			auto f = getFilter( (Type)data[type_pos++] );
			
			//Truncate sizes to not go out of the plane
			unsigned b_w = min( ix+block_size, width );
			unsigned b_h = min( iy+block_size, height );
			
			//Read all pixels in the block
			for( unsigned jy=iy; jy < b_h; jy++ )
				for( unsigned jx=ix; jx < b_w; jx++ )
					setValue( jx, jy, unsigned(data[pos++] + (this->*f)( jx, jy )) % limit );
		}
}

