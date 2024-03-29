#include "javelin.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

javelin_u32 randomNumber()
{
	javelin_u32 v1 = rand();
	javelin_u32 v2 = rand();
	javelin_u32 result = (v1 << 16) | v2;
	printf( "Random value: %u\n", result );
	return result;
}

int main()
{
	struct JavelinState netState;

#ifdef CLIENT
	if ( javelinCreate( &netState, NULL, 0, 1, randomNumber ) != JAVELIN_ERROR_OK ) {
		printf( "Error initializing javelin\n" );
		return 1;
	}

	enum JavelinError r1 = javelinConnect( &netState, "127.0.0.1", 49876 );
	if ( r1 != JAVELIN_ERROR_OK ) {
		printf( "Error connecting to server\n" );
		return 1;
	}
	assert( netState.connectionSlots[0].isActive == true );
#endif

#ifdef SERVER
	if ( javelinCreate( &netState, NULL, 49876, 8, randomNumber ) != JAVELIN_ERROR_OK ) {
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
					javelinQueueMessage( &netState.connectionSlots[slot], &block );
				}
			}
		}
#endif
		for ( int i = 0; i < 5; i++ ) {
			printf( "\033[%iC%i\033[%iD", position[i], i, position[i] );
		}
		printf( "\n" );
		usleep( 50000 );
	}
}

