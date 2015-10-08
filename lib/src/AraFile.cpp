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
	
	decode( dev, version );
	if( version() > 0 )
		return false;
	
	auto header_lenght = read16( dev );
	
	decode( dev, channels, color, search_space, animation, frame_count, width, height, pixel_ratio );
	
	//skip if header is longer than read
	dev.seek( header_lenght + 6 ); //TODO: doesn't work for sequential devices
	
	//TODO: read chunks
	
	return true;
}

bool AraFile::write( QIODevice &dev ){
	dev.write( "ara", 3 );
	write8( dev, 0 );
	
	write16( dev, 18 );
	
	channels.write( dev );
	color.write( dev );
	(search_space = 0).write( dev );
	animation.write( dev );
	frame_count.write( dev );
	width.write( dev );
	height.write( dev );
	pixel_ratio.write( dev );
	
	return false;
}
