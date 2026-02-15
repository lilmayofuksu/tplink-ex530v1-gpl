/******************************************************************************/
/*
 * Copyright (C) 1994-2008 TrendChip Technologies, Corp.
 * All Rights Reserved.
 *
 * TrendChip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of TrendChip Technologies, Corp. and
 * shall not be reproduced, copied, disclosed, or used in whole or
 * in part for any reason without the prior express written permission of
 * TrendChip Technologies, Corp.
 */
/******************************************************************************/

#include "ecmh.h"

void grpclient_destroy(struct grpclientlistnode *grpclient)
{
	if (!grpclient) return;

	/* Free the node */
	free(grpclient);
}

struct grpclientlistnode *grpclient_create(const unsigned char* mac)
{
	if (!mac) return NULL;

	struct grpclientlistnode *grpclient = malloc(sizeof(*grpclient));

	if (!grpclient) return NULL;

	/* Fill her in */
	memcpy(grpclient->mac, mac, sizeof(grpclient->mac));

	/* All okay */
	return grpclient;
}

struct grpclientlistnode *grpclient_find
(const struct list *list, const unsigned char* mac)
{
	struct grpclientlistnode *grpclient;
	struct listnode		*ln;

	if (!mac) return NULL;

	LIST_LOOP(list, grpclient, ln)
	{
		if ( 0 == memcmp(grpclient->mac, mac, sizeof(grpclient->mac)) ) return grpclient;
	}
	return NULL;
}
