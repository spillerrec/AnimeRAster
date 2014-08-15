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

#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <QIODevice>


uint8_t read8( QIODevice &dev );
uint16_t read16( QIODevice &dev );
uint32_t read32( QIODevice &dev );
float readFloat( QIODevice &dev );
void write8(  QIODevice &dev, uint8_t  val );
void write16( QIODevice &dev, uint16_t val );
void write32( QIODevice &dev, uint32_t val );
void writeFloat( QIODevice &dev, float val );


#endif
