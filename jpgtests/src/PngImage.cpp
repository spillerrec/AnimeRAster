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

#include "PngImage.hpp"

#include "planes/FourierPlane.hpp"
#include "PlaneExtras.hpp"
#include "JpegImage.hpp"


#include <QImage>

using namespace AnimeRaster;

	
struct Color{
	double x, y, z;
	Color() : x(0.0), y(0.0), z(0.0) { }
	Color( double x, double y, double z ) : x(x), y(y), z(z) { }
	Color( QRgb rgb );
	Color rgbToYcbcr() const;
	
};

Color::Color( QRgb rgb )
	:	x( qRed(   rgb ) / 255.0 )
	,	y( qGreen( rgb ) / 255.0 )
	,	z( qBlue(  rgb ) / 255.0 )
	{ }
	

Color Color::rgbToYcbcr() const{
	double kr=0.299, kg=0.587, kb=0.114;
	
	double y  = kr*x + kg*this->y + kb*z;
	double cb = (z-y) / (1-kb) * 0.5 + 0.5;
	double cr = (x-y) / (1-kr) * 0.5 + 0.5;
	
	return { y, cb, cr };
}

PngImage::PngImage( QImage img )
	:	raw( 3, Overmix::PlaneBase<double>(img.width(), img.height()) )
{
	//Convert from RGB to YCbCr
	Overmix::PlaneBase<Color> ycbcr( img.width(), img.height() );
	for( unsigned iy=0; iy<ycbcr.get_height(); iy++ ){
		auto out = ycbcr.scan_line( iy );
		auto in  = (const QRgb*)img.constScanLine( iy );
		for( unsigned ix=0; ix<ycbcr.get_width(); ix++ )
			out[ix] = Color{ in[ix] }.rgbToYcbcr();
	}
	
	//Split up into seperate components
	for( unsigned iy=0; iy<ycbcr.get_height(); iy++ ){
		auto in = ycbcr.scan_line( iy );
		auto y  = raw[0].scan_line( iy );
		auto cr = raw[1].scan_line( iy );
		auto cb = raw[2].scan_line( iy );
		
		for( unsigned ix=0; ix<ycbcr.get_width(); ix++ ){
			 y[ix] = in[ix].x;
			cr[ix] = in[ix].y;
			cb[ix] = in[ix].z;
		}
	}
}
	
void PngImage::saveRaw( QString prefix_filename ) const{
	for( unsigned i=0; i<raw.size(); i++ ){
		auto copy = raw[i].to<uint8_t>( [](auto val){ return val*255; } );
		planeToQImage( copy ).save( prefix_filename + QString::number(i) + ".png" );
	}
}

JpegPlane PngImage::toPlane( unsigned index, QuantBlock quant ) const{
	auto& p = raw[index];
	Overmix::DctPlane dct( {8, 8} );
	JpegPlane plane( p.getSize()/8, quant );
	
	for( unsigned iy=0; iy<plane.get_height(); iy++ ){
		auto out = plane.scan_line( iy );
		for( unsigned ix=0; ix<plane.get_width(); ix++ ){
			dct.initialize( p, {ix*8, iy*8}, 255 );
			out[ix].fillFromRaw( dct, {0,0}, plane.quant );
		}
	}
	
	return plane;
}

JpegImage PngImage::toImage( QuantBlock quant ) const{
	JpegImage img;
	
	img.planes.reserve( raw.size() );
	for( unsigned i=0; i<raw.size(); i++ )
		img.planes.push_back( toPlane(i, quant) );
	
	return img;
}
