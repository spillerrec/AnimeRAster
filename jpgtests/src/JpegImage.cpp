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

#include "JpegImage.hpp"

#include "gwenview/iodevicejpegsourcemanager.h"
#include <stdio.h>
#include <jpeglib.h>

#include "Geometry.hpp"
#include "planes/FourierPlane.hpp"

#include <QIODevice>
#include <QImage>
#include <QDebug>

#include <vector>

using namespace AnimeRaster;
using namespace Overmix;


struct JpegComponent{
	jpeg_component_info *info;
	
	JpegComponent( jpeg_component_info& info ) : info(&info) { }
	
	Size<JDIMENSION> size() const
		{ return { info->downsampled_width, info->downsampled_height }; }
	Size<JDIMENSION> sizePadded() const
		{ return (size() + DCTSIZE-1) / DCTSIZE * DCTSIZE; }
	
	Size<int> sampling() const
		{ return { info->h_samp_factor, info->v_samp_factor }; }
};

class JpegDecompress{
	public: //TODO:
		jpeg_decompress_struct cinfo;
		jpeg_error_mgr jerr;
		
	public:
		JpegDecompress(){
			jpeg_create_decompress( &cinfo );
			cinfo.err = jpeg_std_error( &jerr );
		}
		~JpegDecompress(){ jpeg_destroy_decompress( &cinfo ); }
		
		void setDevice( QIODevice& dev )
			{ Gwenview::IODeviceJpegSourceManager::setup( &cinfo, &dev ); }
		
		void readHeader( bool what=true )
			{ jpeg_read_header( &cinfo, what ); }
		
		int components() const{ return cinfo.output_components; }
		JpegComponent operator[](int i){ return { cinfo.comp_info[i] }; }
};


class JpegBlock{
	private:
		PlaneBase<int16_t> table; //TODO: type?
		
		double scale( int ix, int iy ) const
			{ return 2 * 2 * 4 * (ix==0?sqrt(2):1) * (iy==0?sqrt(2):1); }
			//NOTE: 4 is defined by JPEG, 2 is from FFTW, last 2???
		
		
	public:
		JpegBlock();
		JpegBlock( int16_t* input );
		
		void fillDctPlane( DctPlane& dct, const JpegBlock& quant ) const;
};

JpegBlock::JpegBlock() : table( DCTSIZE, DCTSIZE ) { table.fill( 1 ); }
JpegBlock::JpegBlock( int16_t* input ) : table( DCTSIZE, DCTSIZE ) {
	for( unsigned iy=0; iy<DCTSIZE; iy++ ){
		auto row = table.scan_line( iy );
		for( unsigned ix=0; ix<DCTSIZE; ix++ )
			row[ix] = input[iy*DCTSIZE + ix];
	}
}

void JpegBlock::fillDctPlane( DctPlane& dct, const JpegBlock& quant ) const{
		for( unsigned iy=0; iy<DCTSIZE; iy++ ){
		auto in  = table.scan_line( iy );
		auto out = dct  .scan_line( iy );
		auto q   = quant.table.scan_line( iy );
		for( unsigned ix=0; ix<DCTSIZE; ix++ )
			out[ix] = in[ix] * q[ix] * scale( ix, iy );
	}
}

QImage planeToQImage( const PlaneBase<uint8_t>& p ){
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

bool AnimeRaster::from_jpeg( QIODevice& dev ){
	JpegDecompress jpeg;
	jpeg.setDevice( dev );
	jpeg.readHeader();
	
	jpeg.cinfo.raw_data_out = true;
	auto v_ptr = jpeg_read_coefficients( &jpeg.cinfo );
	
	JpegBlock quant( (int16_t*)jpeg.cinfo.comp_info[0].quant_table->quantval );
	DctPlane dct( {DCTSIZE, DCTSIZE} );
	Plane p( DCTSIZE*40, DCTSIZE*40 );
	
	for( unsigned iy=0; iy<40; iy++ ){
	auto blockarr = jpeg.cinfo.mem->access_virt_barray( (j_common_ptr)&jpeg.cinfo, v_ptr[0], 0, 1, false );
	for( unsigned j=0; j<40; j++ ){
		JpegBlock( blockarr[iy][j] ).fillDctPlane( dct, quant );
		dct.toPlane( p, {DCTSIZE*j,DCTSIZE*iy}, 255 );
	}
	}
	planeToQImage( p ).save( "test.png" );
	return false;
	
	return true;
}