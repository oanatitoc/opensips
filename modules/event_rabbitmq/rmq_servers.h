/*
 * Copyright (C) 2017 OpenSIPS Project
 *
 * This file is part of opensips, a free SIP server.
 *
 * opensips is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * opensips is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * History:
 * ---------
 *  2017-01-24  created (razvanc)
 */

#ifndef _RMQ_SERVERS_H_
#define _RMQ_SERVERS_H_

#define RMQ_DEFAULT_HEARTBEAT		0 /* 0 seconds - disabled */
#define RMQ_DEFAULT_RETRIES			0 /* 0 times - do not retry */
#define RMQ_MIN_FRAMES				4096
#define RMQ_DEFAULT_FRAMES			131072

#define RMQ_DEFAULT_CONNECT_TIMEOUT 500 /* ms */

#ifndef AMQP_VERSION_CODE
#define AMQP_VERSION_CODE(major, minor, patch, release) \
    ((major << 24) | (minor << 16) | (patch << 8) | (release))
#endif

#include "../tls_openssl/openssl_api.h"
#include "../tls_mgm/api.h"
#include <amqp.h>
#include "../../lib/list.h"

#if AMQP_VERSION < AMQP_VERSION_CODE(0, 10, 0, 0)
#include "../../locking.h"

extern gen_lock_t *ssl_lock;
#endif

/* AMQP_VERSION was only added in v0.4.0 - there is no way to check the
 * version of the library before this, so we consider everything beyond v0.4.0
 * as old and inneficient */
#if defined AMQP_VERSION && AMQP_VERSION >= AMQP_VERSION_CODE(0, 4, 0, 0)
  #define AMQP_VERSION_v04
#include <amqp_tcp_socket.h>
#include <amqp_ssl_socket.h>
#define rmq_uri struct amqp_connection_info
#define RMQ_EMPTY amqp_empty_bytes
#else
/* although struct amqp_connection_info was added in v0.2.0, there is no way
 * to check against that version, so we assume it does not exist until v0.4.0
 */
typedef struct _rmq_uri {
	char *user;
	char *password;
	char *host;
	char *vhost;
	int port;
	int ssl;
} rmq_uri;
#define RMQ_EMPTY AMQP_EMPTY_BYTES
#endif


enum rmq_server_state { RMQS_OFF, RMQS_INIT, RMQS_CONN, RMQS_ON };

#define RMQF_IMM	(1<<0) /* message MUST be delivered to a consumer immediately. */
#define RMQF_MAND	(1<<1) /* message MUST be routed to a queue */
#define RMQF_NOPER	(1<<2) /* message must not be persistent */

typedef struct rmq_connection {
	enum rmq_server_state state;
	rmq_uri uri;
	unsigned flags;
	int heartbeat;
	str tls_dom_name;
	struct tls_domain *tls_dom;
	amqp_bytes_t exchange;
	amqp_connection_state_t conn;
} rmq_connection_t;

struct rmq_server {
	str cid; /* connection id */
	struct list_head list;
	int max_frames;
	int retries;

	rmq_connection_t conn;
};
int rmq_server_add(modparam_t type, void * val);
int fixup_rmq_server(void **param);
struct rmq_server *rmq_get_server(str *cid);
void rmq_connect_servers(void);

int rmq_send_rm(struct rmq_server *srv, str *rkey, str *body, str *ctype,
		int *names, int *values);

extern struct openssl_binds openssl_api;

#endif /* _RMQ_SERVERS_H_ */
