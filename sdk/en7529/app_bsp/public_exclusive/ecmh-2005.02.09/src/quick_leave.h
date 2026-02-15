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


/* The node used to hold the clients */
struct grpclientlistnode
{
	unsigned char mac[6];
};

void grpclient_destroy(struct grpclientlistnode *grpclient);
struct grpclientlistnode *grpclient_create(const unsigned char* mac);
struct grpclientlistnode *grpclient_find
(const struct list *list, const unsigned char* mac);

