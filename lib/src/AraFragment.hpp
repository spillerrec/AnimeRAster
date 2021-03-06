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

#ifndef ARA_FRAGMENT_HPP
#define ARA_FRAGMENT_HPP

#include <QIODevice>

#include <stdint.h>
#include <vector>


class AraFragment{
	public:
	//	uint8_t type;
		uint8_t options; //TODO: specify
	// TODO: uint16_t
		uint32_t offset_x;
		uint32_t offset_y;
		uint32_t width;
		uint32_t height;
		
	public:
		bool read( QIODevice &dev );
		bool write( QIODevice &dev );
};

#endif
