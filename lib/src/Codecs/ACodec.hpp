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

#ifndef A_CODEC_HPP
#define A_CODEC_HPP

class QIODevice;

class ACodec{
	public:
		template<typename T>
		void decode( QIODevice& dev, T& val ) const { val.read( dev ); }
		template<typename T>
		void encode( QIODevice& dev, T& val ) const { val.write( dev ); }
		
		void decode( QIODevice& ){ } //TODO: we would like to avoid this case
		template<typename T, typename... Args>
		void decode( QIODevice& dev, T& val, Args&... vals ){
			decode( dev, val );
			decode( dev, vals... );
		}
};

#endif
