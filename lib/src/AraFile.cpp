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

#include "device.hpp"

using namespace std;


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
	
	//skip if header is longer than read
	dev.seek( header_lenght + 6 ); //TODO: doesn't work for sequential devices
	
	//TODO: read chunks
	
	return true;
}

bool AraFile::write( QIODevice &dev ){
	dev.write( "ara", 3 );
	write8( dev, 0 );
	
	write16( dev, 18 );
	
	write8( dev, channels );
	write8( dev, color );
	write8( dev, search_space=0 );
	write8( dev, animation );
	write16( dev, frame_count );
	write32( dev, width );
	write32( dev, height );
	write32( dev, pixel_ratio );
	
	return false;
}
