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
		
		int components() const{ return cinfo.num_components; }
		JpegComponent operator[](int i) const{ return { cinfo.comp_info[i] }; }
		
		Size<unsigned> imageSize() const
			{ return { cinfo.image_width, cinfo.image_height }; }
		
		Size<int> maxSampling() const{
			Size<int> out{ 1, 1 };
			for( int i=0; i<components(); i++ )
				out = out.max( (*this)[i].sampling() );
			return out;
		}
		
		Size<unsigned> blockCount( int channel ) const
			{ return (imageSize() + DCTSIZE-1) / DCTSIZE * (*this)[channel].sampling() / maxSampling(); }
			
		Size<JDIMENSION> size() const{ return { cinfo.image_width, cinfo.image_height }; }
		
		Size<JDIMENSION> sizePadded() const{
			Size<JDIMENSION> mcu_size( cinfo.max_h_samp_factor*DCTSIZE, cinfo.max_v_samp_factor*DCTSIZE );
			return (size() + mcu_size-1) / mcu_size * mcu_size;
		}
};

void JpegBlock::fillFromRaw( const Overmix::PlaneBase<double>& input, Overmix::Point<unsigned> pos, const QuantBlock& quant ){
	for( unsigned iy=0; iy<DCTSIZE; iy++ ){
		auto in = input.scan_line( pos.y + iy );
		for( unsigned ix=0; ix<DCTSIZE; ix++ )
			table[iy][ix] = in[ix] / quant[iy][ix] / scale( ix, iy );
	}
}

void JpegBlock::fillDctPlane( DctPlane& dct, const QuantBlock& quant ) const{
		for( unsigned iy=0; iy<DCTSIZE; iy++ ){
		auto out = dct  .scan_line( iy );
		for( unsigned ix=0; ix<DCTSIZE; ix++ )
			out[ix] = table[iy][ix] * quant[iy][ix] * scale( ix, iy );
	}
}

JpegImage AnimeRaster::from_jpeg( QIODevice& dev ){
	JpegImage img;
	
	JpegDecompress jpeg;
	jpeg.setDevice( dev );
	jpeg.readHeader();
	
	jpeg.cinfo.raw_data_out = true;
	auto v_ptr = jpeg_read_coefficients( &jpeg.cinfo );
	
	
	for( int ic=0; ic<jpeg.components(); ic++ ){
		JpegPlane p( jpeg.blockCount( ic ), { jpeg.cinfo.comp_info[ic].quant_table->quantval } );
		
		auto blockarr = jpeg.cinfo.mem->access_virt_barray( (j_common_ptr)&jpeg.cinfo, v_ptr[ic], 0, 1, false );
		for( unsigned iy=0; iy<p.get_height(); iy++ )
			for( unsigned ix=0; ix<p.get_width(); ix++ )
				p.scan_line(iy)[ix] = { blockarr[iy][ix] };
		
		img.planes.emplace_back( std::move( p ) );
	}
	
	return img;
}


class RawReader{
	private:
		JpegDecompress& jpeg;
		
		std::vector<Overmix::PlaneBase<uint8_t>> plane_buf;
		std::vector<std::vector<uint8_t*>> row_bufs;
		std::vector<uint8_t**> buf_access;
		
		
	public:
		RawReader( JpegDecompress& jpeg )
			: jpeg(jpeg){
				//Create buffers
				for( int i=0; i<jpeg.components(); i++ )
					plane_buf.emplace_back( jpeg[i].sizePadded() );
				
				//Create row accesses
				for( int i=0; i<jpeg.components(); i++ ){
					row_bufs.emplace_back( plane_buf[i].get_height(), nullptr );
					for( unsigned iy=0; iy<row_bufs[i].size(); iy++ )
						row_bufs[i][iy] = plane_buf[i].scan_line( iy ).begin();
				}
				
				//Apply "fake" crop
				for( int i=0; i<jpeg.components(); i++ )
					plane_buf[i].crop( {0,0}, jpeg[i].size() );
			}
		
		///Prepare buffer for copy to lines starting from iy
		void prepare_buffer( int iy ){
			buf_access.clear();
			for( unsigned c=0; c<plane_buf.size(); c++ ){
				auto local_y = iy * jpeg[c].sampling().y / jpeg.cinfo.max_v_samp_factor;
				buf_access.push_back( row_bufs[c].data() + local_y );
			}
		}
		
		//Read a set of lines
		void readLine(){
			auto maxSize = jpeg.sizePadded();
			
			prepare_buffer( jpeg.cinfo.output_scanline );
			int remaining = maxSize.height() - jpeg.cinfo.output_scanline;
 			assert( remaining >= jpeg.cinfo.max_v_samp_factor*DCTSIZE );
			jpeg_read_raw_data( &jpeg.cinfo, buf_access.data(), remaining );
			//TODO: chroma is offset of some reason...
		}
		
		auto readAll(){
			while( jpeg.cinfo.output_scanline < jpeg.cinfo.output_height )
				readLine();
			
			return plane_buf;
		}
};

std::vector<Overmix::PlaneBase<uint8_t>> AnimeRaster::from_jpeg_decode( QIODevice& dev ){
	JpegImage img;
	
	JpegDecompress jpeg;
	jpeg.setDevice( dev );
	jpeg.readHeader();
	
	jpeg.cinfo.raw_data_out = true;
	jpeg_start_decompress( &jpeg.cinfo );
	
	RawReader reader( jpeg );
	auto planes = reader.readAll();
	
	jpeg_finish_decompress( &jpeg.cinfo );
	
	return planes;
}

PlaneBase<uint8_t> JpegPlane::toPlane() const{
	Plane out( getSize() * DCTSIZE );
	DctPlane dct( {DCTSIZE, DCTSIZE} );
	
	for( unsigned iy=0; iy<get_height(); iy++ ){
		for( unsigned ix=0; ix<get_width(); ix++ ){
			scan_line( iy )[ix].fillDctPlane( dct, quant );
			dct.toPlane( out, { DCTSIZE*ix, DCTSIZE*iy }, 255 );
		}
	}
	
	return out;
}
