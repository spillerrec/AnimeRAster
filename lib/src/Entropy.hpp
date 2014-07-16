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

#ifndef ENTROPY_HPP
#define ENTROPY_HPP

#include <stdint.h>
#include <vector>

class Entropy{
	private:
		std::vector<uint64_t> counts;
		uint64_t out_of_range{ 0 };
		uint64_t total{ 0 };
		
	public:
		Entropy() : counts( 256*4, 0 ) { }
		
		void add( int val );
		void add( const Entropy& other );
		uint64_t amount() const{ return total + out_of_range; }
		double entropy() const;
		double entropy( const Entropy& other ) const;
};

#endif
