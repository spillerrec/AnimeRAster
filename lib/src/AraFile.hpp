/*	This file is part of AnimeRAster.

	AnimeRAster is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AnimeRAster is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AnimeRAster.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ARA_FILE_HPP
#define ARA_FILE_HPP

#include <stdint.h>
#include <vector>


class AraFile{
	private:
		// "ara" header
		
		/// Format version
		uint8_t version;
		
		// ----- 32 bits -----
		
		// uint16_t header length
		
		/** Bits:
		 *    0-1:
		 *      00 - gray
		 *      01 - RGB
		 *      02 - YUV
		 *      03 - CMYK
		 *    2-4: Reserved
		 *    5-6: sub-sampling
		 *    7:  on = contains alpha
		 */
		uint8_t channels;
		
		static const unsigned COLOR_MODEL_MASK = 0x03;
		static const unsigned ALPHA_MASK = 0x80;
		static const unsigned SUB_SAMPLING_MASK = 0x60;
		
		/** Color depth per channel */
		uint8_t depth{ 8 };
		
		// ----- 32 bits -----
		
		/** UNKNOWN, sRGB, rec. 601, rec. 709 */
		uint8_t color_space;
		
		uint8_t compression;
		uint8_t compression_level;
		uint8_t compression_options;
		
		// ----- 32 bits -----
		
		/// Canvas width
		uint32_t width;
		
		/// Canvas height
		uint32_t height;
		
		/// Pixel aspect ratio
		uint32_t pixel_ratio; //TODO:
		
	public:
		enum ColorModel{
			GRAY = 0x0
		,	RGB  = 0x1
		,	YUV  = 0x2
		,	CMYK = 0x3
		};
		
		enum SubSampling{
			SUB_NONE   = 0x00
		,	SUB_HALF_V = 0x20
		,	SUB_HALF_H = 0x40
		,	SUB_BOTH   = 0x60
		};
		
		bool containsAlpha() const{ return channels & ALPHA_MASK; }
		ColorModel colorModel(){ return ColorModel(channels & COLOR_MODEL_MASK); }
		SubSampling subSampling(){ return SubSampling(channels & SUB_SAMPLING_MASK); }
		
		void setColorModel( ColorModel model ){      channels = (channels & ~COLOR_MODEL_MASK)  + model; }
		void setSubSampling( SubSampling sampling ){ channels = (channels & ~SUB_SAMPLING_MASK) + sampling; }
		void setAlpha( bool contains ){              channels = (channels & ~ALPHA_MASK)        + ( contains ? ALPHA_MASK : 0 ); }
		
		
};

#endif
