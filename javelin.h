#ifndef JAVELIN_H
#define JAVELIN_H

#ifndef JAVELIN_MAX_MESSAGE_SIZE 
#define JAVELIN_MAX_MESSAGE_SIZE 128
#endif
#ifndef JAVELIN_MAX_MESSAGES 
#define JAVELIN_MAX_MESSAGES 4096
#endif
#ifndef JAVELIN_MAX_PACKET_SIZE 
#define JAVELIN_MAX_PACKET_SIZE 1400
#endif
#ifndef JAVELIN_MAX_PENDING_CONNECTIONS
#define JAVELIN_MAX_PENDING_CONNECTIONS 128
#endif
#ifndef JAVELIN_CONNECTION_TIMEOUT_MS
#define JAVELIN_CONNECTION_TIMEOUT_MS 5000
#endif

#define JAVELIN_DEFAULT_RETRY_TIME_MS 100

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

typedef uint8_t javelin_u8;
typedef uint16_t javelin_u16;
typedef int16_t javelin_s16;
typedef uint32_t javelin_u32;
typedef int32_t javelin_s32;
typedef uint64_t javelin_u64;
typedef int64_t javelin_s64;

enum JavelinError {
	JAVELIN_ERROR_OK = 0,
	JAVELIN_ERROR_WINSOCK,
	JAVELIN_ERROR_MEMORY,
	JAVELIN_ERROR_SOCKET,
	JAVELIN_ERROR_GETADDRINFO,
	JAVELIN_ERROR_INVALID_ADDRESS,
	JAVELIN_ERROR_CONNECTION_LIMIT,
	JAVELIN_ERROR_INVALID_MESSAGE,
	JAVELIN_ERROR_MESSAGE_FULL,
	JAVELIN_ERROR_MESSAGE_BUFFER_FULL,
	JAVELIN_ERROR_CHAR_ARRAY_TOO_LONG,
};

enum JavelinConnectionStateType {
	JAVELIN_CONNECTIONSTATE_NONE = 0,
	JAVELIN_CONNECTIONSTATE_CONNECTING,
	JAVELIN_CONNECTIONSTATE_CONNECTED,
	JAVELIN_CONNECTIONSTATE_DISCONNECTING,
	JAVELIN_CONNECTIONSTATE_DISCONNECTED,
};

enum JavelinPacketType {
	JAVELIN_PACKET_UNKNOWN = 0,
	JAVELIN_PACKET_CONNECT_REQUEST,
	JAVELIN_PACKET_CONNECT_CHALLENGE,
	JAVELIN_PACKET_CONNECT_CHALLENGE_RESPONSE,
	JAVELIN_PACKET_CONNECT_ACCEPT,
	JAVELIN_PACKET_CONNECT_REJECT,
	JAVELIN_PACKET_DISCONNECT,
	JAVELIN_PACKET_PING,
	JAVELIN_PACKET_DATA,
};

struct JavelinPacketHeader {
	// TODO: header, crc, salt, etc.
	enum JavelinPacketType type;
	javelin_u32 ackMessageId;
};

struct JavelinMessageBlock {
	javelin_u32 messageId;
	javelin_u32 outgoingLastSendTime;
	size_t incomingReadOffset;
	size_t size;
	javelin_u8 payload[JAVELIN_MAX_MESSAGE_SIZE];
};

struct JavelinConnection {
	bool isActive;
	struct sockaddr_storage address;
	enum JavelinConnectionStateType connectionState;
	javelin_u32 challenge;	// TODO: make this value meaningful beyond existing
	javelin_u32 lastSendTime;
	javelin_u32 lastReceiveTime;
	javelin_u32 retryTime;
	struct JavelinMessageBlock incomingMessageBuffer[JAVELIN_MAX_MESSAGES];
	javelin_u16 incomingLastIdProcessed;
	struct JavelinMessageBlock outgoingMessageBuffer[JAVELIN_MAX_MESSAGES];
	javelin_u16 outgoingLastIdSent;
	javelin_u16 outgoingLastIdAcknowledged;
};

struct JavelinPendingConnection {
	struct sockaddr_storage address;
	enum JavelinConnectionStateType connectionState;
	javelin_u32 challenge;	// TODO: make this value meaningful beyond existing
	javelin_u32 lastSendTime;
	javelin_u32 lastReceiveTime;
};

struct JavelinState {
	struct JavelinConnection* connectionSlots;
	javelin_u32 connectionCount;
	javelin_u32 connectionLimit;
	struct JavelinPendingConnection pendingConnectionSlots[JAVELIN_MAX_PENDING_CONNECTIONS];
	javelin_u32 pendingConnectionCount;
	javelin_u32 incomingLastPacketSlot;
	javelin_u8 outgoingPacketBuffer[JAVELIN_MAX_PACKET_SIZE];
	size_t outgoingPacketSize;
	int socket;
	struct sockaddr_storage address;
};

enum JavelinEventType {
	JAVELIN_EVENT_DATA = 0,
	JAVELIN_EVENT_CONNECT,
	JAVELIN_EVENT_DISCONNECT,
};

struct JavelinEvent {
	const struct JavelinConnection* connection;
	enum JavelinEventType type;
	struct JavelinMessageBlock* message;
};

enum JavelinError javelinCreate( struct JavelinState* state, const char* address, const javelin_u16 port, const javelin_u32 maxConnections );
void javelinDestroy( struct JavelinState* state );
enum JavelinError javelinConnect( struct JavelinState* state, const char* address, const javelin_u16 port );
void javelinDisconnect( struct JavelinState* state );
bool javelinProcess( struct JavelinState* state, struct JavelinEvent* outEvent );
enum JavelinError javelinQueueMessage( struct JavelinState* state, struct JavelinMessageBlock* block, const javelin_u32 slot );

enum JavelinError javelinWriteCharArray( struct JavelinMessageBlock* block, const char* values, const size_t length );
enum JavelinError javelinWriteU8( struct JavelinMessageBlock* block, const javelin_u8 value );
enum JavelinError javelinWriteU16( struct JavelinMessageBlock* block, const javelin_u16 value );
enum JavelinError javelinWriteS16( struct JavelinMessageBlock* block, const javelin_s16 value );
enum JavelinError javelinWriteU32( struct JavelinMessageBlock* block, const javelin_u32 value );
enum JavelinError javelinWriteS32( struct JavelinMessageBlock* block, const javelin_s32 value );
enum JavelinError javelinWriteU64( struct JavelinMessageBlock* block, const javelin_u64 value );
enum JavelinError javelinWriteS64( struct JavelinMessageBlock* block, const javelin_s64 value );

size_t javelinReadCharArray( struct JavelinMessageBlock* block, char* buffer, const size_t bufferSize );
javelin_u8 javelinReadU8( struct JavelinMessageBlock* block );
javelin_u16 javelinReadU16( struct JavelinMessageBlock* block );
javelin_s16 javelinReadS16( struct JavelinMessageBlock* block );
javelin_u32 javelinReadU32( struct JavelinMessageBlock* block );
javelin_s32 javelinReadS32( struct JavelinMessageBlock* block );
javelin_u64 javelinReadU64( struct JavelinMessageBlock* block );
javelin_s64 javelinReadS64( struct JavelinMessageBlock* block );


#ifdef __cplusplus
}
#endif

#endif	// JAVELIN_H

