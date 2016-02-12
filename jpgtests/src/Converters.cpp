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

template<typename T>
std::vector<T> makeRange( T init, T end, T step ){
	std::vector<T> out;
	for( T i=init; (step>0) ? i<=end : i>=end; i+=step )
		out.push_back( i );
	return out;
}

template<typename T>
std::vector<T> reversed( std::vector<T> in ){
	for( unsigned i=0; i<in.size()/2; i++ )
		std::swap( in[i], in[in.size()-i-1] );
	return in;
}

template<typename T>
std::vector<T> flatten( const std::vector<std::vector<T>>& in ){
	std::vector<T> out;
	for( auto list : in )
		for( auto val : list )
			out.push_back( val );
	return out;
}

std::vector<std::vector<int>> zigZagInternal( unsigned size ){
	int max = size*2;
	
	std::vector<std::vector<int>> lists_x;
	for( int counter=0; counter<max; counter+=2 ){
		lists_x.push_back( makeRange( 0, counter, 1 ) );
		lists_x.push_back( makeRange( counter+1, 0, -1 ) );
	}
	for( int counter=2; counter<max; counter+=2 ){
		lists_x.push_back( makeRange( counter-1, max-1, 1 ) );
		lists_x.push_back( makeRange( max-1, counter, -1 ) );
	}
	if( max > 0 )
		lists_x.push_back( { max-1 } );
	
	return lists_x;
}

std::vector<int> zigZagX( unsigned size=4 )
	{ return flatten( zigZagInternal( size ) ); }

std::vector<int> zigZagY( unsigned size=4 ){
	auto lists_y = zigZagInternal(size);
	for( auto& list : lists_y )
		list = reversed( list );
	return flatten( lists_y );
}

std::vector<Overmix::Point<>> AnimeRaster::getZigZagPattern(){
	auto x = zigZagX();
	auto y = zigZagY();
	std::vector<Overmix::Point<>> out;
	for( unsigned i=0; i<x.size(); i++ )
		out.emplace_back( x[i], y[i] );
	return out;
}

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

