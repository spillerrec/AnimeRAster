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

#ifndef A_PLANE_HPP
#define A_PLANE_HPP

#include <cmath>
#include <iostream>
#include <vector>

template<typename T>
inline T paeth( T a, T b, T c ){
	auto p = a + b - c;
	auto pa = abs( p - a );
	auto pb = abs( p - b );
	auto pc = abs( p - c );
	
	if( pa <= pb && pa <= pb )
		return a;
	else if( pb <= pc )
		return b;
	else
		return c;
}

template<typename T>
inline T paeth_right( T a, T b, T c ){
	//TODO: check if correct!
	auto p = a + b - c;
	auto pa = abs( p - a );
	auto pb = abs( p - b );
	auto pc = abs( p - c );
	
	if( pa <= pb && pa <= pb )
		return a;
	else if( pb <= pc )
		return b;
	else
		return c;
}

template<typename T>
inline T strange( T a, T b, T c ){
	if( a < b && b < c )
		return paeth( a, b, c );
	else
		return a + b - c;
}

template<typename T>
inline T strange_right( T a, T b, T c ){
	//TODO: check if correct
	if( a < b && b < c )
		return paeth_right( a, b, c );
	else
		return a + b - c;
}

template <typename ValType>
struct APlane{
	public:
		uint32_t width;
		uint32_t height;
		uint8_t depth;
		ValType middle;
		
		std::vector<ValType> data;
		
		APlane( uint32_t width, uint32_t height, uint8_t depth=8 )
			:	width(width), height(height), depth(depth), data( width*height, ValType() ) {
				middle = ValType( pow( 2, depth-1 ) );
			}
		
		ValType value( unsigned x, unsigned y ) const{
			//TODO: only in debug?
		//	if( x >= width || y >= height )
		//		std::cout << "Out of bounds: requested " << x << "x" << y << " in (" << width << "x" << height << ")\n";
			return data[ x + y*width ];
		}
		
		void setValue( unsigned x, unsigned y, ValType value ){
			//TODO: only in debug?
		//	if( x >= width || y >= height )
		//		std::cout << "Out of bounds: set " << x << "x" << y << " in (" << width << "x" << height << ")\n";
			data[ x + y*width ] = value;
		}
		
		void reduceDepth( unsigned amount ){
			for( auto& val : data )
				val >>= amount;
		}
		
		
	public:
		typedef ValType (APlane<ValType>::*FilterFunc)( unsigned,unsigned ) const;
		
		ValType normal_predict( unsigned, unsigned ) const
			{ return middle; } //TODO: store it as a constant?
		
		//TODO: sub and up could be improved by using the other instead of normal_predict()
		ValType sub_predict( unsigned x, unsigned y ) const
			{ return ( x == 0 ) ? normal_predict( x ,y ) : value( x-1, y ); }

		ValType up_predict( unsigned x, unsigned y ) const
			{ return ( y == 0 ) ? normal_predict( x,y ) : value( x, y-1 ); }

		ValType avg_predict( unsigned x, unsigned y ) const{
			if( x == 0 || y == 0 )
				return up_predict( x, y );
			return ( value( x, y-1 ) + value( x-1, y ) ) / 2;
		}

		ValType avg_right_predict( unsigned x, unsigned y ) const{
			if( x == width-1 || y == 0 )
				return up_predict( x, y );
			return ( value( x, y-1 ) + value( x+1, y ) ) / 2;
		}

		ValType up_right_predict( unsigned x, unsigned y ) const
			{ return ( x == width-1 || y == 0 ) ? up_predict( x, y ) : value( x+1, y-1 ); }

		ValType up_left_predict( unsigned x, unsigned y ) const
			{ return ( x == 0 || y == 0 ) ? up_predict( x, y ) : value( x-1, y-1 ); }

		ValType paeth_predict( unsigned x, unsigned y ) const{
			if( x == 0 || y == 0 )
				return up_predict( x, y );
			
			auto a = value( x-1, y );
			auto b = value( x, y-1 );
			auto c = value( x-1, y-1 );
			
			return paeth( a, b, c );
		}

		ValType paeth_right_predict( unsigned x, unsigned y ) const{
			//TODO: check if correct
			if( x == width-1 || y == 0 )
				return up_predict( x, y );
			
			auto a = value( x+1, y );
			auto b = value( x, y-1 );
			auto c = value( x+1, y-1 );
			
			return paeth_right( a, b, c );
		}

		ValType right_predict( unsigned x, unsigned y ) const
			{ return ( x == width-1 ) ? normal_predict( x,y ) : value( x+1, y ); }

		ValType strange_predict( unsigned x, unsigned y ) const{
			if( x == 0 || y == 0 )
				return up_predict( x, y );
			
			auto a = value( x-1, y );
			auto b = value( x, y-1 );
			auto c = value( x-1, y-1 );
			
			return strange( a, b, c );
		}

		ValType strange_right_predict( unsigned x, unsigned y ) const{
			//TODO: check if correct
			if( x == width-1 || y == 0 )
				return up_predict( x, y );
			
			auto a = value( x+1, y );
			auto b = value( x, y-1 );
			auto c = value( x+1, y-1 );
			
			return strange_right( a, b, c );
		}
	
	
	public:
		
		enum Type{
			NORMAL    = 0x0
		,	UP        = 0x1
		,	SUB       = 0x2
		,	UP_LEFT   = 0x3
		,	AVG       = 0x4
		,	PAETH     = 0x5
		,	STRANGE   = 0x6
		
		,	RIGHT     = 0x7
		,	UP_RIGHT  = 0x8
		,	AVG_RIGHT = 0x9
		,	PAETH_RIGHT   = 0xA
		,	STRANGE_RIGHT   = 0xB
		
		,	MULTI     = 0xC
		,	DIFF       = 0xD
		,	TYPE_COUNT = 0xE
		};
		
		enum EnabledTypes{
			NORMAL_ON        = 0x0001
		,	SUB_ON           = 0x0002
		,	UP_ON            = 0x0004
		,	AVG_ON           = 0x0008
		,	DIFF_ON          = 0x0010
		,	MULTI_ON         = 0x0020
		,	PAETH_ON         = 0x0040
		,	RIGHT_ON         = 0x0080
		,	AVG_RIGHT_ON     = 0x0100
		,	UP_RIGHT_ON      = 0x0200
		,	UP_LEFT_ON       = 0x0400
		,	STRANGE_ON       = 0x0800
		,	STRANGE_RIGHT_ON = 0x1000
		,	PAETH_RIGHT_ON   = 0x2000
		,	ALL_ON = 0xFFFF
		};
		
		static FilterFunc getFilter( Type t ){
			switch( t ){
				case UP:            return &APlane<ValType>::up_predict;
				case SUB:           return &APlane<ValType>::sub_predict;
				case AVG:           return &APlane<ValType>::avg_predict;
				case PAETH:         return &APlane<ValType>::paeth_predict;
				case RIGHT:         return &APlane<ValType>::right_predict;
				case NORMAL:        return &APlane<ValType>::normal_predict;
				case AVG_RIGHT:     return &APlane<ValType>::avg_right_predict;
				case UP_RIGHT:      return &APlane<ValType>::up_right_predict;
				case UP_LEFT:       return &APlane<ValType>::up_left_predict;
				case STRANGE:       return &APlane<ValType>::strange_predict;
				case STRANGE_RIGHT: return &APlane<ValType>::strange_right_predict;
				case PAETH_RIGHT:   return &APlane<ValType>::paeth_right_predict;
				default:
					std::cout << "getFilter( " << t << " ): Not an usable filter! " << t << std::endl;
					return &APlane<ValType>::normal_predict;
			}
		};
		
		static bool typeIsRight( int t ){ return t >= RIGHT; } //TODO: this will not work if we reorder!
		
		static EnabledTypes enabledType( int t ){
			switch( t ){
				case UP:            return UP_ON;
				case SUB:           return SUB_ON;
				case UP_LEFT:       return UP_LEFT_ON;
				case AVG:           return AVG_ON;
				case PAETH:         return PAETH_ON;
				case RIGHT:         return RIGHT_ON;
				case NORMAL:        return NORMAL_ON;
				case AVG_RIGHT:     return AVG_RIGHT_ON;
				case UP_RIGHT:      return UP_RIGHT_ON;
				case STRANGE:       return STRANGE_ON;
				case STRANGE_RIGHT: return STRANGE_RIGHT_ON;
				case PAETH_RIGHT:   return PAETH_RIGHT_ON;
				
				default:
					std::cout << "typeIsRight(): N/A! " << t << std::endl;
					return ALL_ON;
			};
		}
		
		static bool isTypeOn( int t, EnabledTypes types ){
			return types & enabledType( t );
		}
};

#endif
