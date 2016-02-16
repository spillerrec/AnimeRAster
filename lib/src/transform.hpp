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

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <algorithm>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <climits>

std::vector<uint8_t> packTo1bit( const std::vector<int>& in );
std::vector<uint8_t> packTo4bit( const std::vector<int>& in );
std::vector<uint8_t> packTo8bit( const std::vector<int>& in );
std::vector<uint8_t> packTo16bit( const std::vector<int>& in );

std::vector<uint8_t> reorder16bit( const std::vector<uint8_t>& in );

std::vector<uint8_t> lzmaDecompress( const std::vector<uint8_t>& in );
std::vector<uint8_t> lzmaCompress( const std::vector<uint8_t>& in );

std::vector<uint8_t> zpaqDecompress( const std::vector<uint8_t>& in );
std::vector<uint8_t> zpaqCompress( const std::vector<uint8_t>& in );

template<typename T>
std::vector<T>& operator+=( std::vector<T>& left, const T& add ){
	left.push_back( add );
	return left;
}

template<typename T>
std::vector<T>& operator+=( std::vector<T>& left, const std::vector<T>& add ){
	for( auto& val : add )
		left += val;
	return left;
}

template<typename T>
std::vector<T> operator+( std::vector<T> left, const std::vector<T>& add )
	{ return left += add; }

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

std::vector<int> interleavedNegativeData( const std::vector<int>& data );


class Remap{
	private:
		struct Counter{
			uint64_t count;
			uint8_t value;
			uint8_t mapped;
			Counter( uint64_t count, uint8_t value ) : count(count), value(value) { }
			
			bool operator<( const Counter& other ) const{ return count > other.count; }
		};
		std::vector<Counter> histogram;
		
	public:
		Remap( const std::vector<int>& vals ){
			for( unsigned i=0; i<256; i++ )
				histogram.emplace_back( 0, i );
			
			for( uint8_t val : vals )
				histogram[val].count++;
			
			std::sort( histogram.begin(), histogram.end() );
			for( unsigned i=0; i<256; i++ )
				histogram[i].mapped = i;
			
			std::sort( histogram.begin(), histogram.end(), []( Counter a, Counter b ){ return a.value < b.value; } );
			
		//	for( unsigned i=0; i<256; i++ )
		//		cout << i << " - " << (int)histogram[i].count << " - " << (int)histogram[i].mapped << endl;
		}
		
		uint8_t translate( uint8_t in ) const{
			return histogram[in].mapped;
		}
};

#endif
