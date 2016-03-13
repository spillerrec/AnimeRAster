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


#include <QImage>

using namespace AnimeRaster;


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

PngImage::PngImage( QImage img ) : ycbcr( img.width(), img.height() ){
	for( unsigned iy=0; iy<ycbcr.get_height(); iy++ ){
		auto out = ycbcr.scan_line( iy );
		auto in  = (const QRgb*)img.constScanLine( iy );
		for( unsigned ix=0; ix<ycbcr.get_width(); ix++ )
			out[ix] = Color{ in[ix] }.rgbToYcbcr();
	}
	
	QImage test( img );
	for( unsigned iy=0; iy<ycbcr.get_height(); iy++ ){
		auto in  = ycbcr.scan_line( iy );
		auto out = (QRgb*)test.scanLine( iy );
		for( unsigned ix=0; ix<ycbcr.get_width(); ix++ )
			out[ix] = qRgb( in[ix].x*255, in[ix].y*255, in[ix].z*255 );
	}
	test.save( "output.png" );
}

