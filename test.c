#include "javelin.h"
#include <stdlib.h>
#include <unistd.h>

int main()
{
	struct JavelinState netState;

#ifdef CLIENT
	if ( javelinCreate( &netState, NULL, 0, 1 ) != JAVELIN_ERROR_OK ) {
		printf( "Error initializing javelin\n" );
		return 1;
	}

	enum JavelinError r1 = javelinConnect( &netState, "127.0.0.1", 49876 );
	if ( r1 != JAVELIN_ERROR_OK ) {
		printf( "Error connecting to server\n" );
		return 1;
	}
#endif

#ifdef SERVER
	if ( javelinCreate( &netState, NULL, 49876, 8 ) != JAVELIN_ERROR_OK ) {
		printf( "Error initializing javelin\n" );
		return 1;
	}
#endif

	printf( "Beginning loop\n" );

	int position[5] = { 10, 20, 40, 60, 80 };
	while ( true ) {
		struct JavelinEvent event;
		while ( javelinProcess( &netState, &event ) ) {
			if ( event.type == JAVELIN_EVENT_CONNECT ) {
				//printf( "Event: CONNECT\n" );
			}
			else if ( event.type == JAVELIN_EVENT_DISCONNECT ) {
				//printf( "Event: DISCONNECT\n" );
			}
			else if ( event.type == JAVELIN_EVENT_DATA ) {
				//printf( "Event: DATA\n" );
				javelin_u16 s = javelinReadU16( event.message );
				javelin_u16 v = javelinReadU16( event.message );
				position[s] = v;
			}
			else {
				printf( "Event: unknown\n" );
			}
		}
#ifdef SERVER
		for ( int s = 0; s < 5; s++ ) {
			if ( rand() & 1 ) {
				continue;
			}
			position[s] += rand() % 5 - 2;
			if ( position[s] < 0 ) {
				position[s] += 120;
			}
			if ( position[s] > 120 ) {
				position[s] -= 120;
			}
			for ( int slot = 0; slot < netState.connectionLimit; slot++ ) {
				if ( netState.connectionSlots[slot].isActive ) {
					struct JavelinMessageBlock block = {0};
					javelinWriteU16( &block, s );
					javelinWriteU32( &block, position[s] );
					javelinQueueMessage( &netState, &block, slot );
				}
			}
		}
#endif
		for ( int i = 0; i < 5; i++ ) {
			printf( "\033[%iC%i\033[%iD", position[i], i, position[i] );
		}
		printf( "\n" );
		//usleep( 50000 );
	}
}

