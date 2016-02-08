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

#include "PlaneExtras.hpp"

#include <QImage>

using namespace AnimeRaster;

Plane AnimeRaster::normalized( const CoeffPlane& plane ){
	Plane out( plane.getSize() );
	
	auto max = maxPlaneValue( plane );
	auto min = minPlaneValue( plane );
	
	for( unsigned iy=0; iy<out.get_height(); iy++ )
		for( unsigned ix=0; ix<out.get_width(); ix++ )
			out.scan_line(iy)[ix] = color::fromDouble( (plane.scan_line(iy)[ix] - min) / double(max-min) );
	
	return out;
}

QImage AnimeRaster::planeToQImage( const Plane& p ){
	QImage out( p.get_width(), p.get_height(), QImage::Format_RGB32 );
	out.fill( qRgb(0,0,0) );
	
	for( int iy=0; iy<out.height(); iy++ ){
		auto out_row = (QRgb*)out.scanLine( iy );
		auto in      = p.scan_line( iy );
		for( int ix=0; ix<out.width(); ix++ )
			out_row[ix] = qRgb( in[ix], in[ix], in[ix] );
	}
	
	return out;
}
