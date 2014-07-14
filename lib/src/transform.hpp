/*	This file is part of dump-tools.

	dump-tools is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dump-tools is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dump-tools.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <stdint.h>
#include <iostream>
#include <vector>
#include <climits>

std::vector<uint8_t> packTo1bit( const std::vector<int>& in );
std::vector<uint8_t> packTo4bit( const std::vector<int>& in );
std::vector<uint8_t> packTo8bit( const std::vector<int>& in );
std::vector<uint8_t> packTo16bit( const std::vector<int>& in );

std::vector<uint8_t> lzmaCompress( const std::vector<uint8_t>& in );

struct Statistics{
	int min{ INT_MAX };
	int max{ INT_MIN };
	int avg{ 0 };
	unsigned variance{ 0 };
	
	void debug() const{
		std::cout << "Stats: " << min << " - " << max << " - " << avg << " - " << variance << std::endl;
	}
};

Statistics statistics( const std::vector<int>& data );

std::vector<int> offsetData( const std::vector<int>& in, int offset );
std::vector<int> offsetData( const std::vector<int>& data, const std::vector<int>& offsets );

std::vector<int> invertData( const std::vector<int>& data );

#endif
