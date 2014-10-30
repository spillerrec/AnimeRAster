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

#include "../transform.hpp"

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
	//Init variables
	unsigned block_size = data[0];
	unsigned limit = pow( 2, depth );
	unsigned blocks_count = ceil( width / (double)block_size ) * ceil( height / (double)block_size );
	
	//Calculate offsets
	unsigned type_pos = 1;
	unsigned color_pos = type_pos + blocks_count;
	unsigned pos = color_pos + blocks_count;
	
	//Read blocks
	for( unsigned iy=0; iy<height; iy+=block_size )
		for( unsigned ix=0; ix<width; ix+=block_size ){
			//Translate settings
			auto type = data[type_pos++];
			auto color = data[color_pos++];
			auto f = getFilter( (Type)type );
			
			//Truncate sizes to not go out of the plane
			unsigned b_w = min( ix+block_size, width );
			unsigned b_h = min( iy+block_size, height );
			
			//Read all pixels in the block
			for( unsigned jy=iy; jy < b_h; jy++ )
				for( unsigned jx=ix; jx < b_w; jx++ ){
					//Revert filtering
					auto val = Pixel::decode( data, pos, color ) + (this->*f)( jx, jy );
					val.trunc( limit );
					setValue( jx, jy, val );
				}
		}
}


template<typename T1, typename T2>
void addData( vector<T1>& output, const vector<T2>& input ){
	for( auto& in : input )
		output.push_back( in );
}

PixelPlane::PixelBlock::PixelBlock( const PixelPlane& img, Type t, unsigned x, unsigned y, unsigned size, const Entropy& base )
		:	Block( t, x, y, size, img, 0, 0 ) {
		
	ctype = 0;
	//TODO: do color
		/*
	vector<AraBlock> blocks{ AraBlock( t, x, y, size, img, 0 )
		,	AraBlock( t, x, y, size, img, 1 )
		,	AraBlock( t, x, y, size, img, 2 )
		};
	
	//Start with untouched colors
	double best = blockWeight( blocks[0], base ) + blockWeight( blocks[1], base ) + blockWeight( blocks[2], base );
	double best1 = blockWeight( blocks[0], base );
	double best_zero = best1;
	int best_x=0, best_y=0;
	
	blocks = { { t, x, y, size, img, 0, -best_x, -best_y }
		,	{ t, x, y, size, img, 1, -best_x, -best_y }
		,	{ t, x, y, size, img, 2, -best_x, -best_y }
		};
	
	vector<AraBlock> out = blocks;
	
	//Main color iteration
	for( int main=0; main<0; main++ ){
		int second = ( main != 0 ) ? 0 : 1;
		int third = ( main != 1 && second != 1 ) ? 1 : 2;
		
		double current = blockWeight( blocks[main], base );
		
		// subtract main main
		auto b_mm1 = AraBlock::subtract( blocks[second], blocks[main] );
		auto b_mm2 = AraBlock::subtract( blocks[third], blocks[main] );
		double w_mm = current + blockWeight( b_mm1, base ) + blockWeight( b_mm2, base );
		if( w_mm < best ){
			best = w_mm;
			ctype = main*3 + 1;
			out[main]   = blocks[main];
			out[second] = b_mm1;
			out[third]  = b_mm2;
		}
		
		// subtract main second
		auto b_ms1 = AraBlock::subtract( blocks[second], blocks[main] );
		auto b_ms2 = AraBlock::subtract( blocks[third], blocks[second] );
		double w_ms = current + blockWeight( b_ms1, base ) + blockWeight( b_ms2, base );
		if( w_ms < best ){
			best = w_ms;
			ctype = main*3 + 2;
			out[main]   = blocks[main];
			out[second] = b_ms1;
			out[third]  = b_ms2;
		}
		
		// subtract third main
		auto b_tm1 = AraBlock::subtract( blocks[second], blocks[third] );
		auto b_tm2 = AraBlock::subtract( blocks[third], blocks[main] );
		double w_tm = current + blockWeight( b_tm1, base ) + blockWeight( b_tm2, base );
		if( w_tm < best ){
			best = w_tm;
			ctype = main*3 + 3;
			out[main]   = blocks[main];
			out[second] = b_tm1;
			out[third]  = b_tm2;
		}
	}
	
	//Set data
	for( unsigned i=0; i<out[0].data.size(); i++ ){
		data.push_back( out[0].data[i] );
		data.push_back( out[1].data[i] );
		data.push_back( out[2].data[i] );
	}
	
	//Set statistics
	count = out[0].count + out[1].count + out[2].count;
	entropy.add( out[0].entropy );
	entropy.add( out[1].entropy );
	entropy.add( out[2].entropy );
	weight = best;
	*/
}

vector<uint8_t> PixelPlane::save() const{
	auto save_settings = true;
	auto block_size = 4;
	
	vector<int> out, out2, out3;
	vector<int> types{ block_size };
	vector<int> settings;
	
	Entropy entropy;
	
	for( unsigned iy=0; iy<height; iy+=block_size ){
	//	cout << "line: " << iy << endl;
		for( unsigned ix=0; ix<width; ix+=block_size ){
			//auto block = bestColorBlock( ix, iy, config, entropy );
			auto block = PixelBlock( *this, NORMAL, ix, iy, block_size, entropy );
			entropy.add( block.entropy );
			
			//addData( out, block.data );
			for( auto p : block.data )
				for( auto c : p.color )
					out.push_back( c );
			types.push_back( block.type );
			types.push_back( block.ctype );
			addData( settings, block.settings );
		}
	}
	
	
	vector<uint8_t> data;
	//Add settings
	if( save_settings ){
		addData( data, types );
		addData( data, settings );
	}
	
	auto packed = depth>8 ? packTo16bit( out ) : packTo8bit( out );
	addData( data, packed );
	
	return data;
}

