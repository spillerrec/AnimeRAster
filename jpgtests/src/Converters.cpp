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

#include "Converters.hpp"

#include "JpegImage.hpp"

using namespace AnimeRaster;

CoeffPlane AnimeRaster::coeffsFromOffset( const JpegPlane& p, Overmix::Point<unsigned> offset ){
	CoeffPlane out( p.getSize() );
	
	for( unsigned iy=0; iy<p.get_height(); iy++ )
		for( unsigned ix=0; ix<p.get_width(); ix++ )
			out.scan_line(iy)[ix] = p.scan_line(iy)[ix][offset.y][offset.x];
	
	return out;
}

std::vector<int> AnimeRaster::linearizePlane( const CoeffPlane& plane ){
	std::vector<int> out( plane.get_width() * plane.get_height() );
	
	for( unsigned iy=0; iy<plane.get_height(); iy++ )
		for( unsigned ix=0; ix<plane.get_width(); ix++ )
			out[ix + iy*plane.get_width()] = plane.scan_line(iy)[ix];
	
	return out;
}
