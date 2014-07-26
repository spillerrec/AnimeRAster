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

#include "Entropy.hpp"

using namespace std;

#include <cmath>
#include <iostream>


void Entropy::add( int val ){
	total++;
	
	unsigned pos = val < 0 ? (-val)*2+1 : val*2;
//	cout << val << "  -  " << pos << "  -  " << counts.size() << endl;
	if( pos > counts.size() )
		out_of_range++;
	else
		counts[pos]++;
}

void Entropy::add( const Entropy& other ){
	total += other.total;
	out_of_range += other.out_of_range;
	for( unsigned i=0; i<counts.size(); i++ )
		counts[i] += other.counts[i];
}

double Entropy::entropy() const{
	double sum = 0.0;
	for( auto count : counts ){
		double p = count / (double)amount();
		if( p != 0.0 )
			sum += p * -log2( p );
	}
	
	return sum;
}
double Entropy::entropy( const Entropy& other ) const{
	//TODO: assert that sizes are the same!
	double sum = 0.0;
	for( unsigned i=0; i<counts.size(); i++ ){
		double p = (counts[i] + other.counts[i]) / (double)(amount() + other.amount());
		if( p != 0.0 )
			sum += p * -log2( p );
	}
	
//	cout << "Total: " << total << " - " << sum << endl;
	return sum;
}
