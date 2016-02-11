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

#ifndef CSV_FILE_HPP
#define CSV_FILE_HPP

#include <string>
#include <fstream>

class CsvFile{
	private:
		std::ofstream file;
		
	public:	
		CsvFile( std::string filename ) : file( filename ) { }
		~CsvFile(){ file.close(); }
		
		CsvFile& add(){ return *this; }
		CsvFile& add( const char* const value ){
			file << "\"" << value << "\",";
			return *this;
		}
		CsvFile& add( std::string value ){ return add( value.c_str() ); }
		
		template<typename T>
		CsvFile& add( T value ){
			file << value << ",";
			return *this;
		}
		
		template<typename Arg, typename... Args>
		CsvFile& add( Arg arg, Args... args )
			{ return add( arg ).add( args... ); }
		
		template<typename... Args>
		void addLine( Args... args )
			{ add( args... ).stop(); }
		
		void stop(){ file << std::endl; }
};


#endif