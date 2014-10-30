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

#ifndef PIXEL_PLANE_HPP
#define PIXEL_PLANE_HPP

#include "APlane.hpp"

#include <QImage>

struct Pixel{
	int color[3];
	
	Pixel() { }
	Pixel( int val ) {
		for( auto& c : color )
			c = val;
	}
	Pixel( int r, int g, int b){
		color[0] = r;
		color[1] = g;
		color[2] = b;
	}
	
	static Pixel encode( int r, int g, int b, int transform );
	static Pixel decode( const std::vector<uint8_t>& data, unsigned& pos, int transform );
	
	void trunc( unsigned limit ){
		for( auto& c : color )
			c = unsigned(c) % limit;
	}
	
//Operators needed for APlane
	const Pixel& operator>>=( unsigned amount ){
		for( auto& c : color )
			c >>= amount;
		return *this;
	}
	
	Pixel operator+( int val ) const{
		Pixel other( *this );
		for( auto& c: other.color )
			c += val;
		return other;
	}
	
	Pixel operator-( int val ) const{
		Pixel other( *this );
		for( auto& c: other.color )
			c -= val;
		return other;
	}
	
	Pixel operator*( int val ) const{
		Pixel other( *this );
		for( auto& c: other.color )
			c *= val;
		return other;
	}
	
	Pixel operator/( int val ) const{
		Pixel other( *this );
		for( auto& c: other.color )
			c /= val;
		return other;
	}
	
	Pixel operator+( Pixel p ) const{
		for( unsigned i=0; i<3; i++ )
			p.color[i] += color[i];
		return p;
	}
	
	Pixel operator-( Pixel p ) const{
		for( unsigned i=0; i<3; i++ )
			p.color[i] -= color[i];
		return p;
	}
};

inline Pixel abs( Pixel p ){
	for( auto& c : p.color )
		c = abs( c );
	return p;
}

inline Pixel paeth( Pixel a, Pixel b, Pixel c ){
	Pixel out;
	for( unsigned i=0; i<3; i++ )
		out.color[i] = paeth( a.color[i], b.color[i], c.color[i] );
	return out;
}

inline Pixel paeth_right( Pixel a, Pixel b, Pixel c ){
	Pixel out;
	for( unsigned i=0; i<3; i++ )
		out.color[i] = paeth_right( a.color[i], b.color[i], c.color[i] );
	return out;
}

inline Pixel strange( Pixel a, Pixel b, Pixel c ){
	Pixel out;
	for( unsigned i=0; i<3; i++ )
		out.color[i] = strange( a.color[i], b.color[i], c.color[i] );
	return out;
}

inline Pixel strange_right( Pixel a, Pixel b, Pixel c ){
	Pixel out;
	for( unsigned i=0; i<3; i++ )
		out.color[i] = strange_right( a.color[i], b.color[i], c.color[i] );
	return out;
}

struct PixelPlane : public APlane<Pixel>{
	public:
		struct PixelBlock : public Block{
			int ctype;
			
			PixelBlock( const PixelPlane& img, Type t, unsigned x, unsigned y, unsigned size, const Entropy& base );
		};
		
	
	public:
		PixelPlane( uint32_t width, uint32_t height, uint8_t depth ) : APlane( width, height, depth ) { }
		
		QImage asImage() const; //TODO:
		
		void load( const std::vector<uint8_t>& data );
		
		std::vector<uint8_t> save() const;
};

//TODO: template?
inline void AddEntropy( APlane<Pixel>::Block& p, Pixel& val ){
	for( auto c : val.color ){
		p.entropy.add( c );
		p.count += abs( c );
	}
}

#endif
