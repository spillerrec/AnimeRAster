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

#ifndef JPEG_IMAGE_HPP
#define JPEG_IMAGE_HPP

#include "planes/PlaneBase.hpp"

#include <vector>

class QIODevice;

namespace Overmix{ class DctPlane; }

namespace AnimeRaster{

template<typename T>
class Block{
	private:
		static constexpr int DCTSIZE = 8;
		T items[DCTSIZE*DCTSIZE];
		
	public:
		Block( int init ){
			for( auto& val : items )
				val = init;
		}
		
		Block( T* copy ){
			for( unsigned i=0; i<DCTSIZE*DCTSIZE; i++ )
				items[i] = copy[i];
		}
		
		      T* operator[]( int y )      { return items + y*DCTSIZE; }
		const T* operator[]( int y ) const{ return items + y*DCTSIZE; }
};

using QuantBlock = Block<uint16_t>;

class JpegBlock{
	private:
		Block<int16_t> table;
		
		double scale( int ix, int iy ) const
			{ return 2 * 2 * 4 * (ix==0?sqrt(2):1) * (iy==0?sqrt(2):1); }
			//NOTE: 4 is defined by JPEG, 2 is from FFTW, last 2???
		
		
	public:
		JpegBlock() : table( 1 ) { }
		JpegBlock( int16_t* input ) : table( input ) { }
		
		void fillFromRaw( const Overmix::PlaneBase<double>& input, Overmix::Point<unsigned> pos, const QuantBlock& quant );
		void fillDctPlane( Overmix::DctPlane& dct, const QuantBlock& quant ) const;
		
		      int16_t* operator[]( int y )      { return table[y]; }
		const int16_t* operator[]( int y ) const{ return table[y]; }
};

using PlaneJpegBlock = Overmix::PlaneBase<JpegBlock>;

class JpegPlane : public PlaneJpegBlock{
	public:
		QuantBlock quant;
	
	public:
		JpegPlane( QuantBlock quant )
			:	PlaneJpegBlock(),     quant(quant) { }
		JpegPlane( Overmix::Size<unsigned> size, QuantBlock quant )
			:	PlaneJpegBlock(size), quant(quant) { }
		
		Overmix::PlaneBase<uint8_t> toPlane() const;
};

class JpegImage{
	public:
		std::vector<JpegPlane> planes;
};

JpegImage from_jpeg( QIODevice& dev );

std::vector<Overmix::PlaneBase<uint8_t>> from_jpeg_decode( QIODevice& dev );

}

#endif
