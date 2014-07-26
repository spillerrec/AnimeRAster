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

#ifndef VALUE_PLANE_HPP
#define VALUE_PLANE_HPP

#include "APlane.hpp"

#include <QImage>

struct ValuePlane : public APlane<int>{
	public:
		ValuePlane( uint32_t width, uint32_t height, uint8_t depth ) : APlane( width, height, depth ) { }
		
		QImage asImage() const; //TODO:
		
		void loadBlocks( const std::vector<uint8_t>& data, uint8_t block_size, unsigned type_pos, unsigned& pos );
};

#endif
