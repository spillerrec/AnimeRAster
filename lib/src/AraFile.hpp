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

#include <QIODevice>

#include <stdint.h>
#include <vector>

#include "Codecs/IntCodec.hpp"

class AraFile : ACodec{
	private:
		// "ara" header
		
		/// Format version
		UInt8Codec version;
		
		// ----- 32 bits -----
		
		// uint16_t header length
		
		/** Bits:
		 *    0-1:
		 *      0x0 - gray
		 *      0x1 - RGB
		 *      0x2 - YUV
		 *      0x3 - CMYK
		 *    2-4: Reserved
		 *    5-6: sub-sampling
		 *    7:  on = contains alpha */
		UInt8Codec channels;
		
		static const unsigned COLOR_MODEL_MASK = 0x03;
		static const unsigned ALPHA_MASK = 0x80;
		static const unsigned SUB_SAMPLING_MASK = 0x60;
		
		/** Color information, bits:
		 *     0-3: color depth per channel, where 0x0 is 1 bit and 0xF is 16 bits
		 *     4-7: color space:
		 *       0x0 - UNKNOWN
		 *       0x1 - sRGB
		 *       0x2 - Adobe RGB
		 *       0x3 - rec. 601
		 *       0x4 - rec. 709 */
		UInt8Codec color;
		
		// ----- 32 bits -----
		
		/// A value by the encoder, describing the search space size. Higher is bigger
		UInt8Codec search_space;
		
		/// 0x01 indicates animation, other bits reserved
		UInt8Codec animation;
		
		/// Amount of frames
		UInt16Codec frame_count;
		
		// ----- 32 bits -----
		
		/// Canvas width
		UInt32Codec width;
		
		/// Canvas height
		UInt32Codec height;
		
		/// Pixel aspect ratio
		UInt32Codec pixel_ratio; //TODO: how to represent?
		
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
		
		bool containsAlpha() const{ return channels() & ALPHA_MASK; }
		ColorModel colorModel(){ return ColorModel(channels() & COLOR_MODEL_MASK); }
		SubSampling subSampling(){ return SubSampling(channels() & SUB_SAMPLING_MASK); }
		
		void setColorModel( ColorModel model ){      channels = (channels() & ~COLOR_MODEL_MASK)  + model; }
		void setSubSampling( SubSampling sampling ){ channels = (channels() & ~SUB_SAMPLING_MASK) + sampling; }
		void setAlpha( bool contains ){              channels = (channels() & ~ALPHA_MASK)        + ( contains ? ALPHA_MASK : 0 ); }
		
		unsigned depth() const{ return (color() & 0x0F) + 1; }
		void setDepth( unsigned depth ){ color = (color() & 0xF0) + ((depth - 1) & 0x0F); }
		
		enum ColorSpace{
			UNKNOWN = 0x00
		,	SRGB    = 0x10
		,	ARGB    = 0x20
		,	REC601  = 0x30
		,	REC709  = 0x40
		};
		
		ColorSpace colorSpace() const{ return ColorSpace(color() & 0xF0); }
		void setColorSpace( ColorSpace space ){ color = (color() & 0x0F) + space; }
		
	public:
		bool read( QIODevice &dev );
		bool write( QIODevice &dev );
};

#endif
