#include <thread>
#include <list>
#include <algorithm>
#include <iostream>
#include "Common/TaskQueue.h"

using namespace std;

typedef TaskQueue< string > Queue;
Queue tested( 3 );

void producer( string base )
{
	for ( unsigned i = 0; i < 100; ++ i )
		tested.put( base + to_string( i ) );
	tested.producerDone();
}

void consumer( list< string > * target )
{
	try {
		while ( true ) {
			string task = tested.get();
			target->emplace( target->end(), std::move( task ) );
		}
	} catch ( Queue::NoMoreTasksError ) {}
}

bool assertFound( list< string > & out, string lookingFor )
{
	if ( find( out.begin(), out.end(), lookingFor ) == out.end() ) {
		cerr << "Failed on find: " << lookingFor;
		return false;
	} else
		return true;
}

list< string > out1;
list< string > out2;
list< string > out3;

int main()
{
	thread c1( consumer, & out1 );
	thread t1( producer, "first" );
	thread t2( producer, "second" );
	thread t3( producer, "third" );
	thread c2( consumer, & out2 );
	thread c3( consumer, & out3 );

	t1.join();
	t2.join();
	t3.join();
	c1.join();
	c2.join();
	c3.join();

	unsigned total = out1.size() + out2.size() + out3.size();	
	if ( total != 300 ) {
		cerr << "Failed on count: " << total;
		return 1;
	}

	list< string > out;
	out.splice( out.end(), out1 );
	out.splice( out.end(), out2 );
	out.splice( out.end(), out3 );
	for ( unsigned i = 0; i < 100; ++ i )
		if ( not assertFound( out, string( "first" ) + to_string( i ) ) or
				not assertFound( out, string( "second" ) + to_string( i ) ) or
				not assertFound( out, string( "third" ) + to_string( i ) ) )
			return 1;

	cout << "testtaskqueue completed successfully" << endl;
	return 0;
}
