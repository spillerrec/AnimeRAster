/*
	This file is part of Overmix.

	Overmix is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Overmix is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Overmix.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "FourierPlane.hpp"

#include <algorithm>
#include <cmath>
#include <QDebug>

#include "../PlaneExtras.hpp"

using namespace std;
using namespace Overmix;


FourierPlane::FourierPlane( const Plane& p, double range )
	:	PlaneBase( p.get_width() / 2 + 1, p.get_height() ){
	real_width = p.get_width();
	scaling = 1.0 / (real_width * get_height() * range);
	vector<double> raw( p.get_width() * p.get_height() );
	
	fftw_plan plan = fftw_plan_dft_r2c_2d( p.get_height(), p.get_width(), raw.data(), (fftw_complex*)data.get(), FFTW_ESTIMATE );
	
	//Fill data
	for( unsigned iy=0; iy<p.get_height(); iy++ ){
		auto row = p.scan_line( iy );
		for( unsigned ix=0; ix<p.get_width(); ix++ )
			raw[iy*p.get_width()+ix] = color::asDouble( row[ix] ) * range;
	}
	
	fftw_execute( plan );
	
	fftw_destroy_plan( plan );
}

DctPlane::DctPlane( Size<unsigned> size ) :	PlaneBase( size ){
	plan_dct  = fftw_plan_r2r_2d( size.height(), size.width(), scan_line(0).begin(), scan_line(0).begin(), FFTW_REDFT10, FFTW_REDFT10, FFTW_MEASURE );
	plan_idct = fftw_plan_r2r_2d( get_height(), get_width(), scan_line(0).begin(), scan_line(0).begin(), FFTW_REDFT01, FFTW_REDFT01, FFTW_MEASURE );
}
DctPlane::~DctPlane(){
	fftw_destroy_plan( plan_dct );
	fftw_destroy_plan( plan_idct );
}
void DctPlane::initialize( const PlaneBase<double>& p, Point<unsigned> pos, double range ){
	//Fill data
	auto size = p.getSize().min( getSize()+pos ) - pos; //Keep inside
	fill( 0.0 );
	for( unsigned iy=0; iy<size.height(); iy++ ){
		auto row_in  = p.scan_line( iy+pos.y );
		auto row_out =   scan_line( iy       );
		for( unsigned ix=0; ix<size.width(); ix++ )
			row_out[ix] = row_in[ix+pos.x] * range - 128;
	}
	
	//Transform
	fftw_execute( plan_dct );
}

void DctPlane::toPlane( Plane& p, Point<unsigned> pos, double range ){
	fftw_execute( plan_idct );
	auto size = p.getSize().min( getSize()+pos ) - pos; //Keep inside
	
	//Convert range
	for( unsigned iy=0; iy<size.height(); iy++ ){
		auto row = scan_line( iy );
		auto row_out = p.scan_line( iy+pos.y );
		for( unsigned ix=0; ix<size.width(); ix++ ){
			auto norm = row[ix] / (2*get_height() * 2*get_width());
			row_out[ix+pos.x] = color::fromDouble( (norm + 128) / range );
		}
	}
}

Plane FourierPlane::asPlane() const{
	//Find maximum value
	double max_real = 0.0;
	for( auto row : *this ){
		for( auto val : row )
			max_real = max( max_real, abs( val ) );
	}
	
	Plane output( getSize() );
	
	double scale = 100000;
	auto half_size = get_height() / 2;
	for( unsigned iy=0; iy<get_height(); iy++ ){
		auto pos = iy < half_size ? iy + half_size : iy - half_size;
		for( auto val : makeZipRowIt( output.scan_line(iy), scan_line( pos ) ) )
			val.first = color::fromDouble( log( abs( val.second ) / max_real * scale + 1 ) / log( scale + 1 ) );
	}
	
	return output;
}

Plane FourierPlane::toPlaneInvalidate(){
	//Convert
	vector<double> raw( real_width * get_height() );
	fftw_plan plan = fftw_plan_dft_c2r_2d( get_height(), real_width, (fftw_complex*)data.get(), raw.data(), FFTW_ESTIMATE );
	fftw_execute( plan );
	fftw_destroy_plan( plan );
	
	//Fill data into plane
	Plane p( real_width, get_height() );
	for( unsigned iy=0; iy<p.get_height(); iy++ ){
		auto row = p.scan_line( iy );
		for( unsigned ix=0; ix<p.get_width(); ix++ ){
			auto val = raw[iy*p.get_width()+ix] * scaling;//(real_width * get_height());
			row[ix] = color::fromDouble( max( min( val, 1.0 ), 0.0 ) );
		}
	}
	return p;
}


