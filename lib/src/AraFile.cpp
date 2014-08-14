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

#include "AraFile.hpp"

using namespace std;


static uint8_t read8( QIODevice &dev ){
	char byte1 = 0;
	dev.getChar( &byte1 );
	return byte1;
}

static uint16_t read16( QIODevice &dev ){
	char byte1, byte2;
	if( !dev.getChar( &byte1 ) )
		return 0;
	if( !dev.getChar( &byte2 ) )
		return 0;
	return ((uint16_t)byte2 << 8) + (uint8_t)byte1;
}

static uint32_t read32( QIODevice &dev ){
	uint16_t byte1 = read16( dev );
	uint32_t byte2 = read16( dev );
	return (byte2 << 16) + byte1;
}
static float readFloat( QIODevice &dev ){
	float val;
	dev.read( (char*)&val, 4 );
	return val;
}
static void write8(  QIODevice &dev, uint8_t  val ){ dev.write( (char*)&val, 1 ); }
static void write16( QIODevice &dev, uint16_t val ){ dev.write( (char*)&val, 2 ); }
static void write32( QIODevice &dev, uint32_t val ){ dev.write( (char*)&val, 4 ); }
static void writeFloat( QIODevice &dev, float val ){ dev.write( (char*)&val, 4 ); }


bool AraFile::read( QIODevice &dev ){
	//read magic, should be "ara"
	auto m1 = read8( dev );
	auto m2 = read8( dev );
	auto m3 = read8( dev );
	if( m1 != m3 && m1 != 'a' && m2 != 'r' )
		return false;
	
	auto version = read8( dev );
	if( version > 0 )
		return false;
	
	auto header_lenght = read16( dev );
	
	channels     = read8( dev );
	color        = read8( dev );
	search_space = read8( dev );
	animation    = read8( dev );
	frame_count  = read16( dev );
	width        = read32( dev );
	height       = read32( dev );
	pixel_ratio  = read32( dev );
	
	//TODO: animation and chunks stuff
	
	//skip if header is longer than read
	dev.seek( header_lenght + 6 ); //TODO: doesn't work for sequential devices
	
	//TODO: read chunks
	
	return true;
}

bool AraFile::write( QIODevice &dev ){
	return false;
}
