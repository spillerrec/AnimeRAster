/*
	This file is part of AnimeRaster.

	AnimeRaster is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AnimeRaster is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AnimeRaster.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PNG_IMAGE_HPP
#define PNG_IMAGE_HPP

#include "planes/PlaneBase.hpp"

class QImage;

namespace AnimeRaster{

class PngImage {
	public:
		std::vector<Overmix::PlaneBase<double>> raw;
	
	public:
		PngImage( QImage img );
		
		void saveRaw( QString prefix_filename ) const;
};

}

#endif
