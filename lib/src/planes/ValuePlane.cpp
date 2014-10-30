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

#include "../transform.hpp"

using namespace std;


ValuePlane::Block::Block( const ValuePlane& plane, ValuePlane::Type t, unsigned x, unsigned y, unsigned size  )
	:	x(x), y(y), size(size), type(t) {
	unsigned width = min(x+size, plane.width) - x;
	unsigned height = min(y+size, plane.height) - y;
	
	auto filter = plane.getFilter( t );
	for( unsigned iy=y; iy < y+height; iy++ )
		for( unsigned ix=x; ix < x+width; ix++ ){
			auto val = plane.value( ix, iy ) - (plane.*filter)( ix, iy );
		//	entropy.add( val );
			count += abs( val );
		}
}

void ValuePlane::Block::data( const ValuePlane& plane, vector<int>& out ) const{
	unsigned width = min(x+size, plane.width) - x;
	unsigned height = min(y+size, plane.height) - y;
	
	out.reserve( out.size() + width * height );
	
	auto filter = plane.getFilter( type );
	for( unsigned iy=y; iy < y+height; iy++ )
		for( unsigned ix=x; ix < x+width; ix++ )
			out.emplace_back( plane.value( ix, iy ) - (plane.*filter)( ix, iy ) );
}


ValuePlane::Block ValuePlane::bestBlock(
		unsigned x, unsigned y, unsigned block_size, ValuePlane::EnabledTypes types, const Entropy& base
	) const {
	Block best( *this, NORMAL, x, y, block_size ); //TODO: avoid having to init it this way
	
	for( unsigned t=0; t<TYPE_COUNT-2; t++ ){
		if( isTypeOn( t, types ) ){
			Block block( *this, (Type)t, x, y, block_size );
			if( blockWeight(block, base) < blockWeight(best, base) )
				best = block;
		}
	}
	
	return best;
}


void ValuePlane::loadBlocks( const vector<uint8_t>& data, uint8_t block_size, unsigned type_pos, unsigned& pos ){
	//Init variables
	unsigned limit = pow( 2, depth );
	
	//Read blocks
	for( unsigned iy=0; iy<height; iy+=block_size )
		for( unsigned ix=0; ix<width; ix+=block_size ){
			//Translate settings
			auto f = getFilter( (Type)data[type_pos++] );
			auto color = data[type_pos++];
			
			//Truncate sizes to not go out of the plane
			unsigned b_w = min( ix+block_size, width );
			unsigned b_h = min( iy+block_size, height );
			
			//Read all pixels in the block
			for( unsigned jy=iy; jy < b_h; jy++ )
				for( unsigned jx=ix; jx < b_w; jx++ )
					setValue( jx, jy, unsigned(data[pos++] + (this->*f)( jx, jy )) % limit );
		}
}


vector<uint8_t> ValuePlane::saveBlocks( uint8_t block_size, ValuePlane::EnabledTypes enabled_types ) const{
	vector<uint8_t> data;
	vector<int> out, out2;
	out.reserve( width * height );
	out2.reserve( width * height );
	
	Entropy entropy;
	
	for( unsigned iy=0; iy<height; iy+=block_size )
		for( unsigned ix=0; ix<width; ix+=block_size ){
			auto block = bestBlock( ix, iy, block_size, enabled_types, entropy );
			entropy.add( block.entropy );
			
			data.push_back( block.type );
			block.data( *this, out );
		}
	
	for( auto pack : ( depth > 8 ) ? packTo16bit( out ) : packTo8bit( out ) )
		data.push_back( pack );
	
	return data;
}

