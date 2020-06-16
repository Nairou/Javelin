#define _POSIX_C_SOURCE 200809L

#include "javelin.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <sys/types.h>
#endif

#ifndef VERBOSE
#define VERBOSE 0
#endif

enum JavelinError javelinWriteCharArray( struct JavelinMessageBlock* block, const char* buffer, const size_t length )
{
	if ( length >= (1 << 16) ) {
		return JAVELIN_ERROR_CHAR_ARRAY_TOO_LONG;
	}

	if ( block->size + sizeof(javelin_u16) + length > JAVELIN_MAX_MESSAGE_SIZE ) {
		return JAVELIN_ERROR_MESSAGE_FULL;
	}
	javelinWriteU16( block, (javelin_u16)length );
	memcpy( &block->payload[block->size], buffer, length );
	block->size += sizeof(javelin_u16) + length;
	return JAVELIN_ERROR_OK;
}

static void writeBufferU8( javelin_u8* buffer, size_t* offset, const javelin_u8 value )
{
	buffer[(*offset)++] = value;
}

enum JavelinError javelinWriteU8( struct JavelinMessageBlock* block, const javelin_u8 value )
{
	if ( block->size + sizeof(javelin_u8) > JAVELIN_MAX_MESSAGE_SIZE ) {
		return JAVELIN_ERROR_MESSAGE_FULL;
	}
	writeBufferU8( block->payload, &block->size, value );
	return JAVELIN_ERROR_OK;
}

static void writeBufferU16( javelin_u8* buffer, size_t* offset, const javelin_u16 value )
{
	buffer[(*offset)++] = value & 0xff;
	buffer[(*offset)++] = (value >> 8) & 0xff;
}

enum JavelinError javelinWriteU16( struct JavelinMessageBlock* block, const javelin_u16 value )
{
	if ( block->size + sizeof(javelin_u16) > JAVELIN_MAX_MESSAGE_SIZE ) {
		return JAVELIN_ERROR_MESSAGE_FULL;
	}
	writeBufferU16( block->payload, &block->size, value );
	return JAVELIN_ERROR_OK;
}

enum JavelinError javelinWriteS16( struct JavelinMessageBlock* block, const javelin_s16 value )
{
	return javelinWriteU16( block, (javelin_u16)value );
}

static void writeBufferU32( javelin_u8* buffer, size_t* offset, const javelin_u32 value )
{
	buffer[(*offset)++] = value & 0xff;
	buffer[(*offset)++] = (value >> 8) & 0xff;
	buffer[(*offset)++] = (value >> 16) & 0xff;
	buffer[(*offset)++] = (value >> 24) & 0xff;
}

enum JavelinError javelinWriteU32( struct JavelinMessageBlock* block, const javelin_u32 value )
{
	if ( block->size + sizeof(javelin_u32) > JAVELIN_MAX_MESSAGE_SIZE ) {
		return JAVELIN_ERROR_MESSAGE_FULL;
	}
	writeBufferU32( block->payload, &block->size, value );
	return JAVELIN_ERROR_OK;
}

enum JavelinError javelinWriteS32( struct JavelinMessageBlock* block, const javelin_s32 value )
{
	return javelinWriteU32( block, (javelin_u32)value );
}

static void writeBufferU64( javelin_u8* buffer, size_t* offset, const javelin_u64 value )
{
	buffer[(*offset)++] = value & 0xff;
	buffer[(*offset)++] = (value >> 8) & 0xff;
	buffer[(*offset)++] = (value >> 16) & 0xff;
	buffer[(*offset)++] = (value >> 24) & 0xff;
	buffer[(*offset)++] = (value >> 32) & 0xff;
	buffer[(*offset)++] = (value >> 40) & 0xff;
	buffer[(*offset)++] = (value >> 48) & 0xff;
	buffer[(*offset)++] = (value >> 56) & 0xff;
}

enum JavelinError javelinWriteU64( struct JavelinMessageBlock* block, const javelin_u64 value )
{
	if ( block->size + sizeof(javelin_u64) > JAVELIN_MAX_MESSAGE_SIZE ) {
		return JAVELIN_ERROR_MESSAGE_FULL;
	}
	writeBufferU64( block->payload, &block->size, value );
	return JAVELIN_ERROR_OK;
}

enum JavelinError javelinWriteS64( struct JavelinMessageBlock* block, const javelin_s64 value )
{
	return javelinWriteU64( block, (javelin_u64)value );
}


size_t javelinReadCharArray( struct JavelinMessageBlock* block, char* buffer, const size_t bufferSize )
{
	if ( block->incomingReadOffset > block->size ) {
		return 0;
	}
	const size_t length = javelinReadU16( block );
	if ( block->incomingReadOffset + length > block->size ) {
		return 0;
	}
	const size_t copyCount = length < bufferSize ? bufferSize : length;
	memcpy( buffer, &block->payload[block->incomingReadOffset], copyCount );
	block->incomingReadOffset += copyCount;
	return length;
}

static javelin_u8 readBufferU8( const javelin_u8* buffer, size_t* offset )
{
	const javelin_u8 v = buffer[(*offset)++];
	return v;
}

javelin_u8 javelinReadU8( struct JavelinMessageBlock* block )
{
	if ( block->incomingReadOffset + sizeof(javelin_u8) > block->size ) {
		return 0;
	}
	return readBufferU8( block->payload, &block->incomingReadOffset );
}

static javelin_u16 readBufferU16( const javelin_u8* buffer, size_t* offset )
{
	const javelin_u8 v1 = buffer[(*offset)++];
	const javelin_u8 v2 = buffer[(*offset)++];
	return ((javelin_u16)v1) | ((javelin_u16)v2 << 8);
}

javelin_u16 javelinReadU16( struct JavelinMessageBlock* block )
{
	if ( block->incomingReadOffset + sizeof(javelin_u16) > block->size ) {
		return 0;
	}
	return readBufferU16( block->payload, &block->incomingReadOffset );
}

javelin_s16 javelinReadS16( struct JavelinMessageBlock* block )
{
	const javelin_u16 value = javelinReadU16( block );
	if ( value < ((javelin_u16)1 << 15) ) {
		return (javelin_s16)value;
	}
	return -(javelin_s16)(-value);
}

static javelin_u32 readBufferU32( const javelin_u8* buffer, size_t* offset )
{
	const javelin_u8 v1 = buffer[(*offset)++];
	const javelin_u8 v2 = buffer[(*offset)++];
	const javelin_u8 v3 = buffer[(*offset)++];
	const javelin_u8 v4 = buffer[(*offset)++];
	return ((javelin_u32)v1) | ((javelin_u32)v2 << 8) | ((javelin_u32)v3 << 16) | ((javelin_u32)v4 << 24);
}

javelin_u32 javelinReadU32( struct JavelinMessageBlock* block )
{
	if ( block->incomingReadOffset + sizeof(javelin_u32) > block->size ) {
		return 0;
	}
	return readBufferU32( block->payload, &block->incomingReadOffset );
}

javelin_s32 javelinReadS32( struct JavelinMessageBlock* block )
{
	javelin_u32 value = javelinReadU32( block );
	if ( value < ((javelin_u32)1 << 31) ) {
		return (javelin_s32)value;
	}
	return -(javelin_s32)(-value);
}

static javelin_u64 readBufferU64( const javelin_u8* buffer, size_t* offset )
{
	const javelin_u8 v1 = buffer[(*offset)++];
	const javelin_u8 v2 = buffer[(*offset)++];
	const javelin_u8 v3 = buffer[(*offset)++];
	const javelin_u8 v4 = buffer[(*offset)++];
	const javelin_u8 v5 = buffer[(*offset)++];
	const javelin_u8 v6 = buffer[(*offset)++];
	const javelin_u8 v7 = buffer[(*offset)++];
	const javelin_u8 v8 = buffer[(*offset)++];
	return ((javelin_u64)v1) | ((javelin_u64)v2 << 8) | ((javelin_u64)v3 << 16) | ((javelin_u64)v4 << 24) | ((javelin_u64)v5 << 32) | ((javelin_u64)v6 << 40) | ((javelin_u64)v7 << 48) | ((javelin_u64)v8 << 56);
}

javelin_u64 javelinReadU64( struct JavelinMessageBlock* block )
{
	if ( block->incomingReadOffset + sizeof(javelin_u64) > block->size ) {
		return 0;
	}
	return readBufferU64( block->payload, &block->incomingReadOffset );
}

javelin_s64 javelinReadS64( struct JavelinMessageBlock* block )
{
	javelin_u64 value = javelinReadU64( block );
	if ( value < ((javelin_u64)1 << 63) ) {
		return (javelin_s64)value;
	}
	return -(javelin_s64)(-value);
}
enum JavelinError javelinCreate( struct JavelinState* state, const char* address, const javelin_u16 port, const javelin_u32 maxConnections )
{
	static_assert( JAVELIN_MAX_PACKET_SIZE > sizeof(struct JavelinPacketHeader) + sizeof(javelin_u16) + sizeof(javelin_u16) + JAVELIN_MAX_MESSAGE_SIZE, "Max message size is too large to fit in a packet" );
	static_assert( (JAVELIN_MAX_MESSAGES & (JAVELIN_MAX_MESSAGES - 1)) == 0, "Max number of messages must be a power of two" );

	memset( state, 0, sizeof(struct JavelinState) );

#ifdef _WIN32
	WSADATA wsaData;
	int wsaResult = WSAStartup( MAKEWORD(1,1), &wsaData );
	if ( wsaResult != 0 ) {
		return JAVELIN_ERROR_WINSOCK;
	}
	if ( LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1 ) {
		WSACleanup();
		return JAVELIN_ERROR_WINSOCK;
	}
#endif

	struct addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if ( address == NULL ) {
		hints.ai_flags = AI_PASSIVE;
	}

	char portString[10];
	snprintf( portString, sizeof(portString), "%i", port );

	struct addrinfo* addrResults;
	int aiStatus = getaddrinfo( address, portString, &hints, &addrResults );
	if ( aiStatus != 0 ) {
		return JAVELIN_ERROR_GETADDRINFO;
	}

	struct addrinfo* addr;
	for ( addr = addrResults; addr != NULL; addr = addr->ai_next ) {
		state->socket = socket( addr->ai_family, addr->ai_socktype, addr->ai_protocol );
		if ( state->socket == -1 ) {
			continue;
		}

		int bindResult = bind( state->socket, addr->ai_addr, addr->ai_addrlen );
		if ( bindResult == -1 ) {
			// TODO: close socket cross-platform
			continue;
		}

		if ( addr->ai_family == AF_INET ) {
			memcpy( &state->address, addr->ai_addr, sizeof(struct sockaddr_in) );
		}
		else {
			memcpy( &state->address, addr->ai_addr, sizeof(struct sockaddr_in6) );

			int v6Only = 0;
			setsockopt( state->socket, IPPROTO_IPV6, IPV6_V6ONLY, &v6Only, sizeof(v6Only) );
		}

#ifdef _WIN32
		DWORD nonBlocking = 1;
		int nbResult = ioctlsocket( state->socket, FIONBIO, &nonBlocking );
		if ( nbResult != 0 ) {
			return JAVELIN_ERROR_WINSOCK;
		}
#else
		fcntl( state->socket, F_SETFL, O_NONBLOCK );
#endif
		break;
	}

	if ( addr == NULL ) {
		return JAVELIN_ERROR_SOCKET;
	}

	freeaddrinfo( addrResults );

	state->connectionLimit = maxConnections > 0 ? maxConnections : 1;
	state->connectionSlots = (struct JavelinConnection*)malloc( sizeof(struct JavelinConnection) * state->connectionLimit );
	if ( state->connectionSlots == 0 ) {
		return JAVELIN_ERROR_MEMORY;
	}
	memset( state->connectionSlots, 0, sizeof(struct JavelinConnection) * state->connectionLimit );

	return JAVELIN_ERROR_OK;
}

void javelinDestroy( struct JavelinState* state )
{
	if ( state->socket != 0 ) {
#ifdef _WIN32
		closesocket( state->socket );
#else
		close( state->socket );
#endif
	}
	state->socket = 0;

	free( state->connectionSlots );

#ifdef _WIN32
	WSACleanup();
#endif
}

static void writePacketHeader( struct JavelinState* state, enum JavelinPacketType type, javelin_u32 ackId )
{
	state->outgoingPacketSize = 0;
	// TODO: packet header
	writeBufferU8( state->outgoingPacketBuffer, &state->outgoingPacketSize, type );
	writeBufferU16( state->outgoingPacketBuffer, &state->outgoingPacketSize, ackId );
}

static void sendPacket( struct JavelinState* state, struct sockaddr_storage* address )
{
	int result = sendto( state->socket, state->outgoingPacketBuffer, state->outgoingPacketSize, 0, (struct sockaddr*)address, sizeof(struct sockaddr_storage) );
	if ( result < 0 ) {
		printf( "net: sendto error: %i\n", errno );
	}
}

enum JavelinError javelinConnect( struct JavelinState* state, const char* address, const javelin_u16 port )
{
	if ( address == NULL || port == 0 ) {
		return JAVELIN_ERROR_INVALID_ADDRESS;
	}

	if ( state->connectionCount == state->connectionLimit ) {
		return JAVELIN_ERROR_CONNECTION_LIMIT;
	}
	struct JavelinConnection* connection = NULL;
	for ( size_t i = 0; i < state->connectionLimit; i++ ) {
		if ( state->connectionSlots[i].isActive ) {
			continue;
		}
		connection = &state->connectionSlots[i];
		break;
	}
	if ( connection == NULL ) {
		return JAVELIN_ERROR_CONNECTION_LIMIT;
	}
	memset( connection, 0, sizeof(struct JavelinConnection) );

	char portString[10];
	snprintf( portString, sizeof(portString), "%i", port );

	struct addrinfo hints = {0};
	hints.ai_family = state->address.ss_family;
	hints.ai_socktype = SOCK_DGRAM;

	struct addrinfo* addr;
	int aiStatus = getaddrinfo( address, portString, &hints, &addr );
	if ( aiStatus != 0 || addr == NULL ) {
		return JAVELIN_ERROR_GETADDRINFO;
	}

	if ( addr->ai_family == AF_INET ) {
		memcpy( &connection->address, addr->ai_addr, sizeof(struct sockaddr_in) );
	}
	else {
		memcpy( &connection->address, addr->ai_addr, sizeof(struct sockaddr_in6) );
	}

	struct timespec ts;
	timespec_get( &ts, TIME_UTC );
	javelin_u32 currentTimeMs = (javelin_u32)((javelin_u64)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000));

	connection->isActive = true;
	connection->connectionState = JAVELIN_CONNECTIONSTATE_CONNECTING;
	connection->retryTime = JAVELIN_DEFAULT_RETRY_TIME_MS;
	if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_CONNECT_REQUEST\n" );
	writePacketHeader( state, JAVELIN_PACKET_CONNECT_REQUEST, 0 );
	sendPacket( state, &connection->address );
	connection->lastSendTime = currentTimeMs;
	connection->lastReceiveTime = currentTimeMs;

	return JAVELIN_ERROR_OK;
}

void javelinDisconnect( struct JavelinState* state )
{
	struct JavelinConnection* connection = NULL;
	for ( size_t i = 0; i < state->connectionLimit; i++ ) {
		if ( !state->connectionSlots[i].isActive ) {
			continue;
		}
		connection = &state->connectionSlots[i];
		break;
	}
	if ( connection == NULL ) {
		return;
	}
	connection->connectionState = JAVELIN_CONNECTIONSTATE_DISCONNECTING;
	// TODO: decide when we're fully disconnected
	if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_DISCONNECT\n" );
	writePacketHeader( state, JAVELIN_PACKET_DISCONNECT, 0 );
	sendPacket( state, &connection->address );
}

static bool isSameConnection( struct sockaddr_storage* first, struct sockaddr_storage* second )
{
	if ( first->ss_family != second->ss_family ) {
		return false;
	}
	if ( first->ss_family == AF_INET ) {
		struct sockaddr_in* first4 = (struct sockaddr_in*)first;
		struct sockaddr_in* second4 = (struct sockaddr_in*)second;
		if ( second4->sin_addr.s_addr == first4->sin_addr.s_addr && second4->sin_port == first4->sin_port ) {
			return true;
		}
	}
	else if ( first->ss_family == AF_INET6 ) {
		struct sockaddr_in6* first6 = (struct sockaddr_in6*)first;
		struct sockaddr_in6* second6 = (struct sockaddr_in6*)second;
		if ( memcmp( second6->sin6_addr.s6_addr, first6->sin6_addr.s6_addr, 16 ) == 0 && second6->sin6_port == first6->sin6_port ) {
			return true;
		}
	}
	return false;
}

static bool idIsGreater( const javelin_u32 first, javelin_u32 second )
{
	return ((first > second) && (first - second <= (1 << 15))) ||
			((first < second) && (second - first > (1 << 15)));
}

bool javelinProcess( struct JavelinState* state, struct JavelinEvent* outEvent )
{
	struct timespec ts;
	timespec_get( &ts, TIME_UTC );
	javelin_u32 currentTimeMs = (javelin_u32)((javelin_u64)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000));

	// TODO: add unreliable message buffer
	// TODO: add bandwidth tracking to adjust packet size or number sent

	for ( size_t slot = 0; slot < state->connectionLimit; slot++ ) {
		struct JavelinConnection* connection = &state->connectionSlots[slot];
		if ( !connection->isActive ) {
			continue;
		}
		bool messagesToSend = false;
		writePacketHeader( state, JAVELIN_PACKET_DATA, connection->incomingLastIdProcessed );
		size_t messageIndex = (connection->outgoingLastIdAcknowledged + 1) % JAVELIN_MAX_MESSAGES;
		while ( connection->outgoingMessageBuffer[messageIndex].messageId != connection->outgoingLastIdSent ) {
			struct JavelinMessageBlock* block = &connection->outgoingMessageBuffer[messageIndex];
			if ( state->outgoingPacketSize + sizeof(javelin_u16) + sizeof(javelin_u16) + block->size > JAVELIN_MAX_MESSAGE_SIZE ) {
				break;
			}
			if ( block->outgoingLastSendTime == -1 || (block->outgoingLastSendTime - currentTimeMs) > connection->retryTime ) {
				if ( VERBOSE ) printf( "queuing message to send: id = %i, size = %zu\n", block->messageId, block->size );
				// message header
				writeBufferU16( state->outgoingPacketBuffer, &state->outgoingPacketSize, block->messageId );
				writeBufferU16( state->outgoingPacketBuffer, &state->outgoingPacketSize, block->size );
				memcpy( &state->outgoingPacketBuffer[state->outgoingPacketSize], block->payload, block->size );
				state->outgoingPacketSize += block->size;
				block->outgoingLastSendTime = currentTimeMs;
				messagesToSend = true;
			}
			messageIndex = (messageIndex + 1) % JAVELIN_MAX_MESSAGES;
		}
		if ( messagesToSend ) {
			if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_DATA\n" );
			sendPacket( state, &connection->address );
			connection->lastSendTime = currentTimeMs;
		}
	}

	// Scan all connections for connection packets to resend
	for ( size_t slot = 0; slot < state->connectionLimit; slot++ ) {
		struct JavelinConnection* connection = &state->connectionSlots[slot];
		if ( !connection->isActive ) {
			continue;
		}
		if ( currentTimeMs - connection->lastSendTime < connection->retryTime ) {
			continue;
		}

		if ( connection->connectionState == JAVELIN_CONNECTIONSTATE_CONNECTING ) {
			if ( connection->challenge == 0 ) {
				if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_CONNECT_REQUEST\n" );
				writePacketHeader( state, JAVELIN_PACKET_CONNECT_REQUEST, 0 );
				sendPacket( state, &connection->address );
				connection->lastSendTime = currentTimeMs;
			}
			else {
				if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE\n" );
				writePacketHeader( state, JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE, 0 );
				// TODO: include challenge response
				sendPacket( state, &connection->address );
				connection->lastSendTime = currentTimeMs;
			}
		}
	}

	// Scan all connections and pendingConnections for timeouts
	size_t pendingConnectionIndex = 0;
	while ( pendingConnectionIndex < state->pendingConnectionCount ) {
		struct JavelinPendingConnection* pendingConnection = &state->pendingConnectionSlots[pendingConnectionIndex];
		if ( currentTimeMs - pendingConnection->lastReceiveTime >= JAVELIN_CONNECTION_TIMEOUT_MS ) {
			if ( VERBOSE ) printf( "net: pending connection timeout: %zu\n", pendingConnectionIndex );
			*pendingConnection = state->pendingConnectionSlots[--state->pendingConnectionCount];
		}
		else {
			pendingConnectionIndex++;
		}
	}
	for ( size_t slot = 0; slot < state->connectionLimit; slot++ ) {
		struct JavelinConnection* connection = &state->connectionSlots[slot];
		if ( !connection->isActive ) {
			continue;
		}
		if ( currentTimeMs - connection->lastReceiveTime >= JAVELIN_CONNECTION_TIMEOUT_MS ) {
			if ( VERBOSE ) printf( "connection timeout for slot %zu\n", slot );
			connection->isActive = false;
			connection->connectionState = JAVELIN_CONNECTIONSTATE_DISCONNECTED;
			outEvent->connection = connection;
			outEvent->type = JAVELIN_EVENT_DISCONNECT;
			return true;
		}
		else if ( currentTimeMs - connection->lastSendTime >= connection->retryTime && connection->connectionState == JAVELIN_CONNECTIONSTATE_CONNECTED ) {
			if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_PING\n" );
			writePacketHeader( state, JAVELIN_PACKET_PING, connection->incomingLastIdProcessed );
			sendPacket( state, &connection->address );
			connection->lastSendTime = currentTimeMs;
		}
	}

	// Keep reading packets until we have a message to return
	while ( true ) {
		struct JavelinConnection* lastPacketConnection = &state->connectionSlots[state->incomingLastPacketSlot];
		const javelin_u32 nextIndex = (lastPacketConnection->incomingLastIdProcessed + 1) & 0xffff;
		const javelin_u32 messageIndex = nextIndex % JAVELIN_MAX_MESSAGES;
		if ( lastPacketConnection->incomingMessageBuffer[messageIndex].messageId == nextIndex ) {
			if ( VERBOSE ) printf( "returning queued message %u\n", lastPacketConnection->incomingMessageBuffer[messageIndex].messageId );
			outEvent->connection = lastPacketConnection;
			outEvent->type = JAVELIN_EVENT_DATA;
			outEvent->message = &lastPacketConnection->incomingMessageBuffer[messageIndex];
			lastPacketConnection->incomingLastIdProcessed = nextIndex;
			return true;
		}

		struct sockaddr_storage fromAddress;
		int fromLength;
		javelin_u8 packetBuffer[JAVELIN_MAX_PACKET_SIZE];
		int receivedLength = recvfrom( state->socket, packetBuffer, JAVELIN_MAX_PACKET_SIZE, 0, (struct sockaddr*)&fromAddress, (socklen_t*)&fromLength );
		if ( receivedLength <= 0 ) {
			if ( receivedLength == -1 && errno != EAGAIN && errno != EWOULDBLOCK ) {
				printf( "net: recvfrom error: %i\n", errno );
			}
			return false;
		}

		char addrstr[1024];
		char portstr[16];
		int gniresult = getnameinfo( (struct sockaddr*)&fromAddress, sizeof(struct sockaddr_storage), addrstr, sizeof(addrstr), portstr, sizeof(portstr), NI_NUMERICHOST|NI_NUMERICSERV );
		if ( gniresult == 0 ) {
			//printf( "net: packet from %s:%s\n", addrstr, portstr );
		}

		size_t readOffset = 0;
		struct JavelinPacketHeader packetHeader;
		packetHeader.type = readBufferU8( packetBuffer, &readOffset );
		packetHeader.ackMessageId = readBufferU16( packetBuffer, &readOffset );

		struct JavelinConnection* packetConnection = NULL;
		for ( size_t slot = 0; slot < state->connectionLimit; slot++ ) {
			struct JavelinConnection* connection = &state->connectionSlots[slot];
			if ( !connection->isActive ) {
				continue;
			}
			if ( isSameConnection( &fromAddress, &connection->address ) ) {
				packetConnection = connection;
				break;
			}
		}

		if ( packetConnection == NULL ) {
			struct JavelinPendingConnection* pendingConnection = NULL;
			for ( size_t slot = 0; slot < state->pendingConnectionCount; slot++ ) {
				struct JavelinPendingConnection* connection = &state->pendingConnectionSlots[slot];
				if ( connection->connectionState == JAVELIN_CONNECTIONSTATE_NONE ) {
					continue;
				}
				if ( isSameConnection( &fromAddress, &connection->address ) ) {
					pendingConnection = connection;
					break;
				}
			}
			if ( pendingConnection == NULL ) {
				if ( state->pendingConnectionCount == JAVELIN_MAX_PENDING_CONNECTIONS ) {
					if ( VERBOSE ) printf( "net: server full: %i = %i\n", state->pendingConnectionCount, JAVELIN_MAX_PENDING_CONNECTIONS );
					// TODO: send "server full" packet
					continue;	// next packet, no room for another connection attempt
				}
				if ( VERBOSE ) printf( "net: new pending slot: %i\n", state->pendingConnectionCount );
				pendingConnection = &state->pendingConnectionSlots[state->pendingConnectionCount++];
				pendingConnection->address = fromAddress;
				pendingConnection->connectionState = JAVELIN_CONNECTIONSTATE_NONE;
				pendingConnection->lastSendTime = 0;
				pendingConnection->challenge = 0;
			}
			pendingConnection->lastReceiveTime = currentTimeMs;
			if ( packetHeader.type == JAVELIN_PACKET_CONNECT_REQUEST ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_CONNECT_REQUEST\n" );
				pendingConnection->connectionState = JAVELIN_CONNECTIONSTATE_CONNECTING;
				if ( pendingConnection->challenge == 0 ) {
					pendingConnection->challenge = 123;	// TODO: use random salt value
				}
				if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_CONNECT_CHALLENGE\n" );
				writePacketHeader( state, JAVELIN_PACKET_CONNECT_CHALLENGE, 0 );
				writeBufferU32( state->outgoingPacketBuffer, &state->outgoingPacketSize, pendingConnection->challenge );
				sendPacket( state, &pendingConnection->address );
				pendingConnection->lastSendTime = currentTimeMs;
			}
			else if ( packetHeader.type == JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE && pendingConnection->connectionState == JAVELIN_CONNECTIONSTATE_CONNECTING && pendingConnection->challenge != 0 ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE\n" );
				// TODO: test validity of challenge response
				javelin_u32 availableSlot = -1;
				for ( size_t slot = 0; slot < state->connectionLimit; slot++ ) {
					struct JavelinConnection* connection = &state->connectionSlots[slot];
					if ( !connection->isActive ) {
						availableSlot = slot;
						break;
					}
				}
				if ( availableSlot == -1 ) {
					// TODO: send "server full" packet
					if ( VERBOSE ) printf( "net: server full\n" );
					*pendingConnection = state->pendingConnectionSlots[--state->pendingConnectionCount];
					continue;	// next packet
				}
				struct JavelinConnection* connection = &state->connectionSlots[availableSlot];
				memset( connection, 0, sizeof(struct JavelinConnection) );
				connection->isActive = true;
				connection->address = pendingConnection->address;
				connection->connectionState = JAVELIN_CONNECTIONSTATE_CONNECTED;
				connection->challenge = pendingConnection->challenge;
				connection->retryTime = JAVELIN_DEFAULT_RETRY_TIME_MS;

				if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_CONNECT_ACCEPT\n" );
				writePacketHeader( state, JAVELIN_PACKET_CONNECT_ACCEPT, 0 );
				sendPacket( state, &connection->address );
				connection->lastSendTime = currentTimeMs;
				connection->lastReceiveTime = currentTimeMs;

				outEvent->connection = connection;
				outEvent->type = JAVELIN_EVENT_CONNECT;
				return true;
			}

			continue;	// next packet
		}

		packetConnection->lastReceiveTime = currentTimeMs;
		if ( idIsGreater( packetHeader.ackMessageId, packetConnection->outgoingLastIdAcknowledged ) ) {
			packetConnection->outgoingLastIdAcknowledged = packetHeader.ackMessageId;
			if ( VERBOSE ) printf( "net: acknowledged up to %u\n", packetConnection->outgoingLastIdAcknowledged );
		}
		if ( packetConnection->connectionState == JAVELIN_CONNECTIONSTATE_CONNECTING ) {
			if ( packetHeader.type == JAVELIN_PACKET_CONNECT_CHALLENGE ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_CONNECT_CHALLENGE\n" );
				packetConnection->challenge = readBufferU32( packetBuffer, &readOffset );
				if ( VERBOSE ) printf( "net: received challenge: %i\n", packetConnection->challenge );
				if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE\n" );
				writePacketHeader( state, JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE, 0 );
				// TODO: include challenge response
				sendPacket( state, &packetConnection->address );
				packetConnection->lastSendTime = currentTimeMs;
			}
			else if ( packetHeader.type == JAVELIN_PACKET_CONNECT_ACCEPT && packetConnection->challenge != 0 ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_CONNECT_ACCEPT\n" );
				packetConnection->connectionState = JAVELIN_CONNECTIONSTATE_CONNECTED;
				outEvent->connection = packetConnection;
				outEvent->type = JAVELIN_EVENT_CONNECT;
				return true;
			}
			else if ( packetHeader.type == JAVELIN_PACKET_CONNECT_REJECT && packetConnection->challenge != 0 ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_CONNECT_REJECT\n" );
				packetConnection->connectionState = JAVELIN_CONNECTIONSTATE_DISCONNECTED;
				outEvent->connection = packetConnection;
				outEvent->type = JAVELIN_EVENT_DISCONNECT;
				return true;
			}
		}
		else if ( packetConnection->connectionState == JAVELIN_CONNECTIONSTATE_CONNECTED ) {
			if ( packetHeader.type == JAVELIN_PACKET_DATA ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_DATA\n" );
				while ( readOffset < receivedLength ) {
					// message header
					const javelin_u16 id = readBufferU16( packetBuffer, &readOffset );
					const javelin_u32 size = readBufferU16( packetBuffer, &readOffset );
					if ( VERBOSE ) printf( "received message: id = %i, size = %i\n", id, size );
					if ( readOffset + size > receivedLength ) {
						// If reported size is bad, ignore the rest of the packet
						break;
					}
					if ( id - packetConnection->incomingLastIdProcessed < JAVELIN_MAX_MESSAGES ) {
						if ( VERBOSE ) printf( "storing message %u\n", id );
						struct JavelinMessageBlock* block = &packetConnection->incomingMessageBuffer[id % JAVELIN_MAX_MESSAGES];
						block->messageId = id;
						block->incomingReadOffset = 0;
						block->size = size;
						memcpy( block->payload, &packetBuffer[readOffset], size );
					}
					readOffset += size;
				}
			}
			else if ( packetHeader.type == JAVELIN_PACKET_PING ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_PING\n" );
				// nothing
			}
			else if ( packetHeader.type == JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE && packetConnection->challenge != 0 ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE\n" );
				// We think the client is already connected, but they may not have received the accept packet
				if ( VERBOSE ) printf( "net: Send packet: JAVELIN_PACKET_CONNECT_ACCEPT\n" );
				writePacketHeader( state, JAVELIN_PACKET_CONNECT_ACCEPT, 0 );
				sendPacket( state, &packetConnection->address );
				packetConnection->lastSendTime = currentTimeMs;
			}
			else if ( packetHeader.type == JAVELIN_PACKET_DISCONNECT ) {
				if ( VERBOSE ) printf( "net: Received JAVELIN_PACKET_DISCONNECT\n" );
				packetConnection->isActive = false;
				packetConnection->connectionState = JAVELIN_CONNECTIONSTATE_DISCONNECTED;
				outEvent->connection = packetConnection;
				outEvent->type = JAVELIN_EVENT_DISCONNECT;
				return true;
			}
		}
	}

	return false;
}

enum JavelinError javelinQueueMessage( struct JavelinState* state, struct JavelinMessageBlock* block, const javelin_u32 slot )
{
	if ( block->size == 0 || block->size > JAVELIN_MAX_MESSAGE_SIZE ) {
		return JAVELIN_ERROR_INVALID_MESSAGE;
	}

	struct JavelinConnection* connection = &state->connectionSlots[slot];
	if ( connection->outgoingLastIdSent - connection->outgoingLastIdAcknowledged >= JAVELIN_MAX_MESSAGES ) {
		return JAVELIN_ERROR_MESSAGE_BUFFER_FULL;
	}

	const javelin_u32 messageIndex = (connection->outgoingLastIdSent + 1) % JAVELIN_MAX_MESSAGES;
	struct JavelinMessageBlock* outgoingBlock = &connection->outgoingMessageBuffer[messageIndex];
	memcpy( outgoingBlock->payload, block->payload, block->size );
	outgoingBlock->size = block->size;
	outgoingBlock->messageId = ++connection->outgoingLastIdSent & 0xffff;
	outgoingBlock->outgoingLastSendTime = -1;
	if ( VERBOSE ) printf( "net: message queued as %i\n", outgoingBlock->messageId );
	return JAVELIN_ERROR_OK;
}



