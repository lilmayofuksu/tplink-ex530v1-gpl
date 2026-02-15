/*
** $Id: tc3162.h,v 1.7 2011/01/07 06:05:58 pork Exp $
*/
/************************************************************************
 *
 *	Copyright (C) 2006 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: tc3162.h,v $
** Revision 1.7  2011/01/07 06:05:58  pork
** add the definition of INT!16,INT32,SINT15,SINT7
**
** Revision 1.6  2010/09/20 07:08:02  shnwind
** decrease nf_conntrack buffer size
**
** Revision 1.5  2010/09/03 16:43:07  here
** [Ehance] TC3182 GMAC Driver is support TC-Console & WAN2LAN function & update the tc3182 dmt version (3.12.8.83)
**
** Revision 1.4  2010/09/02 07:04:50  here
** [Ehance] Support TC3162U/TC3182 Auto-Bench
**
** Revision 1.3  2010/08/30 07:53:02  lino
** add power saving mode kernel module support
**
** Revision 1.2  2010/06/05 05:40:29  lino
** add tc3182 asic board support
**
** Revision 1.1.1.1  2010/04/09 09:39:21  feiyan
** New TC Linux Make Flow Trunk
**
** Revision 1.4  2010/01/14 10:56:42  shnwind
** recommit
**
** Revision 1.3  2010/01/14 08:00:10  shnwind
** add TC3182 support
**
** Revision 1.2  2010/01/10 15:27:26  here
** [Ehancement]TC3162U MAC EEE is operated at 100M-FD, SAR interface is accroding the SAR_CLK to calculate atm rate.
**
** Revision 1.1.1.1  2009/12/17 01:42:47  josephxu
** 20091217, from Hinchu ,with VoIP
**
** Revision 1.2  2006/07/06 07:24:57  lino
** update copyright year
**
** Revision 1.1.1.1  2005/11/02 05:45:38  lino
** no message
**
** Revision 1.5  2005/09/27 08:01:38  bread.hsu
** adding IMEM support for Tc3162L2
**
** Revision 1.4  2005/09/14 11:06:20  bread.hsu
** new definition for TC3162L2
**
** Revision 1.3  2005/06/17 16:26:16  jasonlin
** Remove redundant code to gain extra 100K bytes free memory.
** Add "CODE_REDUCTION" definition to switch
**
** Revision 1.2  2005/06/14 10:02:01  jasonlin
** Merge TC3162L2 source code into new main trunk
**
** Revision 1.1.1.1  2005/03/30 14:04:22  jasonlin
** Import Linos source code
**
** Revision 1.4  2004/11/15 03:43:17  lino
** rename ATM SAR max packet length register
**
** Revision 1.3  2004/09/01 13:15:47  lino
** fixed when pc shutdown, system will reboot
**
** Revision 1.2  2004/08/27 12:16:37  lino
** change SYS_HCLK to 96Mhz
**
** Revision 1.1  2004/07/02 08:03:04  lino
** tc3160 and tc3162 code merge
**
*/

#ifndef _PKGID_H_
#define _PKGID_H_
#include <common/ecnt_chip_id.h>
/* EN7581 */
#define EN7581_HIR		(0xd)

/* EN7523 */
#define EN7523_HIR		(0xc)


/* EN7580 */
#define EN7580_HIR		(0xa)

#ifdef TCSUPPORT_CPU_ARMV8
extern uint32_t GET_PACKAGE_ID(void);

/* EN7523 */
#define isEN7529DU		(isEN7523 && (GET_PACKAGE_ID() == EN7529DU))
#define isEN7529DT		(isEN7523 && (GET_PACKAGE_ID() == EN7529DT))
#define isEN7529CU		(isEN7523 && (GET_PACKAGE_ID() == EN7529CU))
#define isEN7562DU		(isEN7523 && (GET_PACKAGE_ID() == EN7562DU))
#define isEN7562DT		(isEN7523 && (GET_PACKAGE_ID() == EN7562DT))
#define isEN7562CU		(isEN7523 && (GET_PACKAGE_ID() == EN7562CU))
#define isEN7523GU		(isEN7523 && (GET_PACKAGE_ID() == EN7523GU))
#define isEN7523DU		(isEN7523 && (GET_PACKAGE_ID() == EN7523DU))
#define isEN7529GTH		(isEN7523 && (GET_PACKAGE_ID() == EN7529GTH))	/* 2'b 0_1000 */
#define isEN7562GTH		(isEN7523 && (GET_PACKAGE_ID() == EN7562GTH))
#define isEN7523SU		(isEN7523 && (GET_PACKAGE_ID() == EN7523SU))
#define isEN7529GTS		(isEN7523 && (GET_PACKAGE_ID() == EN7529GTS))
#define isEN7562GTS		(isEN7523 && (GET_PACKAGE_ID() == EN7562GTS))
#define isEN7529IT		(isEN7523 && (GET_PACKAGE_ID() == EN7529IT))
#define isEN7529CT		(isEN7523 && (GET_PACKAGE_ID() == EN7529CT))
#define isEN7562CT		(isEN7523 && (GET_PACKAGE_ID() == EN7562CT))
#define isEN7523DT		(isEN7523 && (GET_PACKAGE_ID() == EN7523DT))	/* 2'b 1_0000 */
#define isEN7529DTM		(isEN7523 && (GET_PACKAGE_ID() == EN7529DTM))
#define isEN7562DTM		(isEN7523 && (GET_PACKAGE_ID() == EN7562DTM))
#define isEN7529ITM		(isEN7523 && (GET_PACKAGE_ID() == EN7529ITM))
#define isEN7529CTM		(isEN7523 && (GET_PACKAGE_ID() == EN7529CTM))
#define isEN7562CTM		(isEN7523 && (GET_PACKAGE_ID() == EN7562CTM))
#define isEN7523DTM		(isEN7523 && (GET_PACKAGE_ID() == EN7523DTM))

#else
/* EN7523 */
#define isEN7529DU		(0)
#define isEN7529DT		(0)
#define isEN7529CU		(0)
#define isEN7562DU		(0)
#define isEN7562DT		(0)
#define isEN7562CU		(0)
#define isEN7523GU		(0)
#define isEN7523DU		(0)
#define isEN7529GTH		(0)
#define isEN7562GTH		(0)
#define isEN7523SU		(0)
#define isEN7529GTS		(0)
#define isEN7562GTS		(0)
#define isEN7529IT		(0)
#define isEN7529CT		(0)
#define isEN7562CT		(0)
#define isEN7523DT		(0)
#define isEN7529DTM		(0)
#define isEN7562DTM		(0)
#define isEN7529ITM		(0)
#define isEN7529CTM		(0)
#define isEN7562CTM		(0)
#define isEN7523DTM		(0)

#endif

#endif /* _PKGID_H_ */
