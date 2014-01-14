/**
 *  cwebsocket: A fast, lightweight websocket client/server
 *
 *  Copyright (c) 2014 Jeremy Hahn
 *
 *  This file is part of cwebsocket.
 *
 *  cwebsocket is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  cwebsocket is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with cwebsocket.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <wchar.h>

#ifdef THREADED
	#include <pthread.h>
#endif

#ifndef HANDSHAKE_BUFFER_MAX
	#define HANDSHAKE_BUFFER_MAX 256
#endif

#ifndef DATA_BUFFER_MAX
	#define DATA_BUFFER_MAX 65536
#endif

#define WEBSOCKET_STATE_CONNECTING (1 << 0)
#define WEBSOCKET_STATE_CONNECTED  (1 << 1)
#define WEBSOCKET_STATE_OPEN       (1 << 2)
#define WEBSOCKET_STATE_CLOSING    (1 << 3)
#define WEBSOCKET_STATE_CLOSED     (1 << 4)

typedef enum {
	TRUE,
	FALSE
} bool;

typedef enum {
	CONTINUATION = 0x00,
	TEXT_FRAME = 0x01,
	BINARY_FRAME = 0x02,
	CLOSE = 0x08,
	PING = 0x09,
	PONG = 0x0A,
} opcode;

typedef struct {
	uint32_t opcode;
#if defined(__arm__ ) || defined(__i386__)
	uint32_t payload_len;
#else
	uint64_t payload_len;
#endif
	char *payload;
} cwebsocket_message;

typedef struct {
	bool fin;
	bool rsv1;
	bool rsv2;
	bool rsv3;
	opcode opcode;
	bool mask;
	int payload_len;
	uint32_t masking_key[4];
} cwebsocket_frame;

typedef struct _cwebsocket {
	int sock_fd;
#ifdef THREADED
	pthread_t thread;
	pthread_mutex_t lock;
#endif
	uint8_t state;
	void (*onopen)(struct _cwebsocket *);
	void (*onmessage)(struct _cwebsocket *, cwebsocket_message *message);
	void (*onclose)(struct _cwebsocket *, const char *message);
	void (*onerror)(struct _cwebsocket *, const char *error);
} cwebsocket_client;

typedef struct {
	cwebsocket_client *socket;
	cwebsocket_message *message;
} cwebsocket_thread_args;

// "public"
int cwebsocket_connect(cwebsocket_client *websocket, const char *uri);
int cwebsocket_read_data(cwebsocket_client *websocket);
ssize_t cwebsocket_write_data(cwebsocket_client *websocket, const char *data, int len);
void cwebsocket_close(cwebsocket_client *websocket, const char *message);
void cwebsocket_run(cwebsocket_client *websocket);

// "private"
int cwebsocket_read_handshake(cwebsocket_client *websocket, char *seckey);
int cwebsocket_handshake_handler(cwebsocket_client *websocket, const char *message, const char *seckey);
void cwebsocket_print_frame(cwebsocket_frame *frame);

#endif
