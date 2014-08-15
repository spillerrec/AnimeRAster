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

#include "device.hpp"


uint8_t read8( QIODevice &dev ){
	char byte1 = 0;
	dev.getChar( &byte1 );
	return byte1;
}

uint16_t read16( QIODevice &dev ){
	char byte1, byte2;
	if( !dev.getChar( &byte1 ) )
		return 0;
	if( !dev.getChar( &byte2 ) )
		return 0;
	return ((uint16_t)byte2 << 8) + (uint8_t)byte1;
}

uint32_t read32( QIODevice &dev ){
	uint16_t byte1 = read16( dev );
	uint32_t byte2 = read16( dev );
	return (byte2 << 16) + byte1;
}
float readFloat( QIODevice &dev ){
	float val;
	dev.read( (char*)&val, 4 );
	return val;
}
void write8(  QIODevice &dev, uint8_t  val ){ dev.write( (char*)&val, 1 ); }
void write16( QIODevice &dev, uint16_t val ){ dev.write( (char*)&val, 2 ); }
void write32( QIODevice &dev, uint32_t val ){ dev.write( (char*)&val, 4 ); }
void writeFloat( QIODevice &dev, float val ){ dev.write( (char*)&val, 4 ); }

