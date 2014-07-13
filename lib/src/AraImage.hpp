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
		,	FILTERED = 0x1
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
		
	private:
		std::vector<uint8_t> compress_none() const;
		std::vector<uint8_t> compress_filtered() const;
};

#endif
