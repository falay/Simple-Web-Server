#include <iostream>
#include "WebServer.h"

using std::cerr ;

int main(int argc, char* argv[])
{
	WebServer simpleWebServer ;
	
	if( argc == 3 )
		simpleWebServer.Boot(atoi(argv[1]), argv[2]) ;
	
	else
		cerr << "Usage: ./WebServer [Port] [Document root directory]\n" ;
}