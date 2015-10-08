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

#ifndef CODEC__INTEGER_HPP
#define CODEC__INTEGER_HPP

#include "ACodec.hpp"
#include "../device.hpp"

class UInt8Codec : ACodec{
	private:
		unsigned value;
	public:
		void read( QIODevice& dev ) { value = read8( dev ); }
		void write( QIODevice& dev ) const { write8( dev, value ); }
		
		unsigned operator()() const{ return value; }
		UInt8Codec& operator=( unsigned newValue ){
			value = newValue;
			return *this;
		}
};

class UInt16Codec : ACodec{
	private:
		unsigned value;
	public:
		void read( QIODevice& dev ) { value = read16( dev ); }
		void write( QIODevice& dev ) const { write16( dev, value ); }
		
		unsigned operator()() const{ return value; }
		UInt16Codec& operator=( unsigned newValue ){
			value = newValue;
			return *this;
		}
};

class UInt32Codec : ACodec{
	private:
		unsigned value;
	public:
		void read( QIODevice& dev ) { value = read32( dev ); }
		void write( QIODevice& dev ) const { write32( dev, value ); }
		
		unsigned operator()() const{ return value; }
		UInt32Codec& operator=( unsigned newValue ){
			value = newValue;
			return *this;
		}
};

#endif
