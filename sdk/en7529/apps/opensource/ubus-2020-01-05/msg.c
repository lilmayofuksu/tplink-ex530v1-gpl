/*
 * Copyright (C) 2011 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <unistd.h>
#include <arpa/inet.h>

#include "libubus.h"

#include "msg_ext.h"

static struct blob_buf b;
static int timeout = 30;
static bool simple_output = false;


static void receive_call_result_data(struct ubus_request *req, int type, struct blob_attr *msg)
{
	return;
}

static void receive_event(struct ubus_context *ctx, struct ubus_event_handler *ev,
			  const char *type, struct blob_attr *msg)
{
	printf("event = %s\n", type);
	fflush(stdout);
}



static int ubus_cli_call(struct ubus_context *ctx, int argc, char **argv)
{
	uint32_t id;

	if (argc < 2 || argc > 3)
		return -2;

	blob_buf_init(&b, 0);

	
	if (argc == 3)
		blob_put_string(&b,1,argv[2]);

	id = atoi(argv[0]);

//	printf("ubus_cli_call: id = %d \n",id);
	
	return ubus_invoke(ctx, id, argv[1], b.head, receive_call_result_data, NULL, timeout * 1000);
}

static int ubus_cli_listen(struct ubus_context *ctx, int argc, char **argv)
{
	static struct ubus_event_handler listener;
	const char *event;
	int ret = 0;

	memset(&listener, 0, sizeof(listener));
	listener.cb = receive_event;

	if (argc > 0) {
		event = argv[0];
	} else {
		event = "*";
		argc = 1;
	}

	do {
		ret = ubus_register_event_handler(ctx, &listener, event);
		if (ret)
			break;

		argv++;
		argc--;
		if (argc <= 0)
			break;

		event = argv[0];
	} while (1);

	if (ret) {
		if (!simple_output)
			fprintf(stderr, "Error while registering for event '%s': %s\n",
				event, ubus_strerror(ret));
		return -1;
	}

	uloop_init();
	ubus_add_uloop(ctx);
	uloop_run();
	uloop_done();

	return 0;
}

static int ubus_cli_send(struct ubus_context *ctx, int argc, char **argv)
{
	if (argc < 1 || argc > 2)
		return -2;

	blob_buf_init(&b, 0);

	if (argc == 2)
		blob_put_string(&b,1,argv[1]);
	
	return ubus_send_event(ctx, argv[0], b.head);
}

static int ubus_cli_send_ext(struct ubus_context *ctx, int argc, char **argv)
{
	int        type = 0;
	unsigned int evt_id = 0;
	wan_evt_t wan_evt;
	voip_evt_t voip_evt;
	mesh_evt_t mesh_evt;
	char evt_type[EVT_TYPE_LENGTH];
	
	if(3 != argc)
	{
		return -2;
	}

	memset(&wan_evt, 0, sizeof(wan_evt));
	memset(&voip_evt, 0, sizeof(voip_evt));
	memset(&mesh_evt, 0, sizeof(mesh_evt));
	memset(evt_type, 0, sizeof(evt_type));
	type = atoi(argv[0]);
	switch (type)
	{
		case EVT_WAN_INTERNAL_TYPE :
		{
			strcpy(evt_type, EVT_WAN_INTERNAL);
			break;
		}
		case EVT_WAN_EXTERNAL_TYPE:
		{
			strcpy(evt_type, EVT_WAN_EXTERNAL);
			break;
		}
		case EVT_VOIP_INTERNAL_TYPE:
		{
			strcpy(evt_type, EVT_VOIP_INTERNAL);
			break;
		}
		case EVT_MESH_INTERNAL_TYPE:
		{
			snprintf(evt_type, sizeof(evt_type), "%s", EVT_MESH_INTERNAL);
			break;
		}
		default:
		{
			return -2;
		}
	}
	evt_id = atoi(argv[1]);
	switch (evt_id)
	{
		case EVT_CFG_WAN_ENTRY_DELETE :
		case EVT_CFG_WAN_ENTRY_UPDATE :
		case EVT_WAN_CONN_GETV4 :
		case EVT_WAN_CONN_LOSTV4 :
		case EVT_WAN_CONN_GETV6 :
		case EVT_WAN_CONN_LOSTV6 :
		case EVT_DUMP_WAN_SRV_ENTRY_DEV:  
		case EVT_DUMP_WAN_SRV_ENTRY_PATH:  
		case EVT_DUMP_WAN_SRV_ALL_ENTRY:  
		case EVT_WAN_SRV_READY_IPV6_GATEWAY:  
		case EVT_WAN_SRV_DEBUG_LEVEL:
		case EVT_XPON_UP:
		case EVT_XPON_DOWN:
		case EVT_WAN_ENTRY_DELETE :
		case EVT_WAN_ENTRY_UPDATE :
		case EVT_WAN_IPV4_UP :
		case EVT_WAN_IPV4_DOWN :
		case EVT_WAN_IPV6_UP :
		case EVT_WAN_IPV6_DOWN :
		case EVT_WAN_BRIDGE_UP:  
		case EVT_WAN_BRIDGE_DOWN:  
		case EVT_WAN_UPDATE_CONNREQ_PORT: 
		case EVT_WAN_AP_BRIDGE_GET4:
		case EVT_WAN_AP_BRIDGE_LOST4:
		case EVT_WAN_AP_STATUS_UPDATE:
		case EVT_WAN_AP_START_BRIDGE:
		case EVT_WAN_AP_START_CLIENT:
		{
			strcpy(wan_evt.buf, argv[2]);
			blob_buf_init(&b, 0);
			blob_put_u32(&b, 1, htonl(evt_id));
			blob_put(&b, 2, &wan_evt, sizeof(wan_evt));
			break;
		}
		case EVT_VOIP_SRV_DEBUG_LEVEL :
		{
			strcpy(voip_evt.buf, argv[2]);
			blob_buf_init(&b, 0);
			blob_put_u32(&b, 1, htonl(evt_id));
			blob_put(&b, 2, &voip_evt, sizeof(voip_evt));
			break;
		}
		case EVT_MESH_SRV_DEBUG_LEVEL:
		case EVT_CFG_MESH_BOOT:
		case EVT_CFG_UPDATE_DAT:
		case EVT_CFG_UPDATE_MAP:
		case EVT_CFG_UPDATE_COMMON:
		case EVT_CFG_UPDATE_MAPD:
		case EVT_CFG_UPDATE_STEER:
		case EVT_CFG_UPDATE_RADIO:
		case EVT_CFG_ACTION:
		case EVT_TRIGGER_MESH_REINIT:
		{
			snprintf(mesh_evt.buf, sizeof(mesh_evt.buf), "%s", argv[2]);
			blob_buf_init(&b, 0);
			blob_put(&b, 1, &evt_id, sizeof(evt_id));
			blob_put(&b, 2, &mesh_evt, sizeof(mesh_evt));
			break;
		}
		default:
		{
			return -2;
		}
	}

	return ubus_send_event(ctx, evt_type, b.head);
}



static int usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s [<options>] <command> [arguments...]\n"
		"Options:\n"
		" -s <socket>:		Set the unix domain socket to connect to\n"
		" -t <timeout>:		Set the timeout (in seconds) for a command to complete\n"
		"\n"
		"Commands:\n"
		" - call <path> <method> [<message>]	Call an object method\n"
		" - listen [<path>...]			Listen for events\n"
		" - send  <type> [<message>]		Send an event\n"
		" - send_ext [type 1 wan internal / 2 wan external / 3 voip internal / 5 mesh internal] [evt_id ] [message] send an evt\n"
		"\n", prog);
	return 1;
}


struct {
	const char *name;
	int (*cb)(struct ubus_context *ctx, int argc, char **argv);
} commands[] = {
	{ "call", ubus_cli_call },
	{ "listen", ubus_cli_listen },
	{ "send", ubus_cli_send },
	{ "send_ext", ubus_cli_send_ext },
	
};

int main(int argc, char **argv)
{
	const char *progname, *ubus_socket = NULL;
	static struct ubus_context *ctx;
	char *cmd;
	int ret = 0;
	int i, ch;

	progname = argv[0];

	while ((ch = getopt(argc, argv, "vs:t:S")) != -1) {
		switch (ch) {
		case 's':
			ubus_socket = optarg;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		default:
			return usage(progname);
		}
	}

	argc -= optind;
	argv += optind;

	cmd = argv[0];
	if (argc < 1)
		return usage(progname);

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		if (!simple_output)
			fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	argv++;
	argc--;

	ret = -2;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(commands[i].name, cmd) != 0)
			continue;

		ret = commands[i].cb(ctx, argc, argv);
		break;
	}

	if (ret > 0 && !simple_output)
		fprintf(stderr, "Command failed: %s\n", ubus_strerror(ret));
	else if (ret == -2)
		usage(progname);

	ubus_free(ctx);
	return ret;
}
