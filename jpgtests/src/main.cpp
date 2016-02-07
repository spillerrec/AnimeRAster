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

#include <QCoreApplication>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QDebug>

#include "JpegImage.hpp"

using namespace std;


int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	QStringList files = app.arguments();
	files.pop_front();
	
	for( auto filepath : files ){
		qDebug() << filepath;
		QFile file( filepath );
		if( !file.open( QIODevice::ReadOnly ) )
			return -1;
		
		AnimeRaster::from_jpeg( file );
	}
	
	return 0;
}