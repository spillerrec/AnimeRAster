/*	This file is part of dump-tools.

	dump-tools is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dump-tools is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dump-tools.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ARA_IMAGE_HPP
#define ARA_IMAGE_HPP

#include "dump/DumpPlane.hpp"

#include <QFile>
#include <QImage>

#include <iostream>
#include <stdint.h>
#include <cstring>
#include <vector>

struct AraPlane{
	public:
		uint32_t width;
		uint32_t height;
		
		std::vector<int> data;
		
		AraPlane( uint32_t width, uint32_t height )
			:	width(width), height(height), data( width*height, 0 ) { }
		
		int value( unsigned x, unsigned y ) const{
			//TODO: only in debug?
			if( x >= width || y >= height )
				std::cout << "Out of bounds: requested " << x << "x" << y << " in (" << width << "x" << height << ")\n";
			return data[ x + y*width ];
		}
		
		void setValue( unsigned x, unsigned y, int value ){
			//TODO: only in debug?
			if( x >= width || y >= height )
				std::cout << "Out of bounds: set " << x << "x" << y << " in (" << width << "x" << height << ")\n";
			data[ x + y*width ] = value;
		}
		
		QImage asImage( unsigned depth ) const;
};

class AraImage{
	public:
		enum ColorType{
			GRAY
		,	RGB
		,	YUV
		};
		
		enum Compression{
			NONE = 0x0
		,	LINES = 0x1
		,	BLOCKS = 0x2
		};
	
	private:
		uint8_t channels{ 0 };
		uint8_t depth{ 0 };
		uint8_t sub_sampling{ 0 }; //TODO:
		uint8_t color_space{ 0 };
		Compression compression{ NONE };
		uint8_t compression_level{ 0 };
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		float pixel_ratio{ 1.0 };
		
		uint32_t data_length{ 0 };
		
		std::vector<AraPlane> planes;
		
	public:
		uint32_t getWidth() const{ return width; }
		uint32_t getHeight() const{ return height; }
		uint8_t getDepth() const{ return depth; }
		
		bool valid() const{
			return width > 0 && height > 0 && depth > 0 && planes.size() > 0 && pixel_ratio > 0.0;
		}
		
		
		void initFromQImage( QImage img );
		void initFromDump( std::vector<DumpPlane> dumps );
		
		bool read( QIODevice &dev );
		bool write( QIODevice &dev, Compression level=NONE );
		
		QImage outputPlanes() const;
		
	public: //TODO:
		typedef int (AraImage::*FilterFunc)( int,unsigned,unsigned ) const;
		int normal_filter( int plane, unsigned x, unsigned y ) const;
		int sub_filter( int plane, unsigned x, unsigned y ) const;
		int up_filter( int plane, unsigned x, unsigned y ) const;
		int avg_filter( int plane, unsigned x, unsigned y ) const;
		int paeth_filter( int plane, unsigned x, unsigned y ) const;
		int right_filter( int plane, unsigned x, unsigned y ) const;
		int prev_filter( int plane, unsigned x, unsigned y ) const;
		int diff_filter( int plane, unsigned x, unsigned y, int dx, int dy ) const;
		
		enum EnabledTypes{
			NORMAL_ON = 0x01
		,	SUB_ON = 0x02
		,	UP_ON = 0x04
		,	AVG_ON = 0x08
		,	DIFF_ON = 0x10
		,	MULTI_ON = 0x20
		,	PAETH_ON = 0x40
		,	RIGHT_ON = 0x80
		,	PREV_ON = 0x100
		,	ALL_ON = 0xFFFF
		};
		
		struct Config{
			unsigned block_size;
			unsigned min_block_size;
			unsigned search_size;
			EnabledTypes types;
			double multi_penalty;
			bool both_directions;
			int diff_save_offset{ 128 };
			bool diff_compress_offsets{ true };
			bool save_settings{ true };
		};
		
		enum Type{
			MULTI  = 0x0
		,	DIFF  = 0x1
		,	UP   = 0x2
		,	SUB  = 0x3
		,	AVG  = 0x4
		,	PAETH = 0x5
		,	RIGHT = 0x6
		,	NORMAL = 0x7
		,	PREV = 0x8
		};
			
		struct AraLine{
			Type  type{ NORMAL };
			std::vector<int> data;
			unsigned count{ 0 };
			
			AraLine( Type t, unsigned y, const AraImage& img, int plane, FilterFunc filter ) : type(t) {
				auto width = img.planes[plane].width;
				data.reserve( width );
				for( unsigned ix=0; ix<width; ix++ ){
					auto val = (img.*filter)( plane, ix, y );
					data.emplace_back( val );
					count += val;
				}
			}
		};
		
		struct AraBlock{
			Type  type{ NORMAL };
			unsigned x;
			unsigned y;
			std::vector<int> data;
			std::vector<uint8_t> types;
			std::vector<uint8_t> settings;
			unsigned count{ 0 };
			
			unsigned width;
			unsigned height;
			
			AraBlock( Type t, unsigned x, unsigned y, unsigned size, const AraImage& img, int plane )
				:	type(t), x(x), y(y) {
				types.push_back( t );
				
				width = std::min(x+size, img.planes[plane].width) - x;
				height = std::min(y+size, img.planes[plane].height) - y;
				data.reserve( width * height );
			}
			
			AraBlock( Type t, unsigned x, unsigned y, unsigned size, const AraImage& img, int plane, FilterFunc filter )
				:	AraBlock( t, x, y, size, img, plane ) {
				for( unsigned iy=y; iy < y+height; iy++ )
					for( unsigned ix=x; ix < x+width; ix++ ){
						auto val = (img.*filter)( plane, ix, iy );
						data.emplace_back( val );
						count += abs( val );
					}
			}
			
			AraBlock( unsigned x, unsigned y, const AraImage& img, int plane, Config config );
		};
		
		
		double type_weight{ 0.0 };
		double count_weight{ 1.0 };
		double setting_lin_weight{ 1.0 };
		double setting_log_weight{ 0.0 };
		
		double weight_setting( int setting ) const;
		double weight( const AraBlock& block ) const;
		double weight( unsigned type_count, unsigned count, unsigned settings_count, double settings_sum ) const;
			
		AraBlock makeMulti( unsigned x, unsigned y, int plane, Config config ) const;
		AraBlock best_block( unsigned x, unsigned y, int plane, Config config ) const;
		std::vector<int> make_optimal( Config config ) const;
		std::vector<uint8_t> make_optimal_configuration() const;
		
		void read_lines( std::vector<uint8_t> types, std::vector<uint8_t> data, unsigned offset );
		
		std::vector<uint8_t> compress_none() const;
		std::vector<uint8_t> compress_lines() const;
		std::vector<uint8_t> compress_blocks( Config config ) const;
};

#endif
