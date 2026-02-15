/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2010
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*******************************************************************************
 *
 * Filename:
 * ---------
 * bch_bma_hw.cpp
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is used to generate ECC parity (upper layer)
 *
 * Author:
 * -------
 * Rongguo Zhang(MTK80761)
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * MAY 23 2011 mtk80761
 * [ALPS] [New Feature] First release.
*******************************************************************************/
//=============================================================================
//
//Function : BCH code encode/decode 
//Author   : Celesta
//Date     : 2007/08
//I/O      :  
//
//=============================================================================
//#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include "bch_encoder_hw.c" //HW for m=13
//#include "mtk_debug.h"
#include "ECCGenerator.h"

#define m    13
#define maxt 12

void bch_encoder_hw(int *meg_xdi,int meg_size, int t ,int *rem, unsigned char *outBuf);

static void ECC_Parity_Post_Proc(unsigned char *inBuf, unsigned char *outBuf, int outBuf_size_in_byte);
static int ECC_Generator(unsigned char *inBuf, unsigned char *outBuf, int encode_size_in_byte, int encode_capability);
//int ECC_OutBuf_Size_Cal(int encode_capability);


int ECC_OutBuf_Size_Cal(int encode_capability)
{
    int di_bit = 8;    
    int outBuf_size_in_byte_org = 0;

#ifdef DEBUG    
    MTK_fprintf("encode_capability = %d\r\n", encode_capability);
#endif 
    
    if ( (encode_capability != 4) && (encode_capability != 8) && (encode_capability != 12) )
    {
        printf("encode_capability[%d] must be 4, 8 or 12, HALT!\r\n", encode_capability);
        return -1;
    }
        
    if ( 0 == (m * encode_capability) % di_bit )
    {
        outBuf_size_in_byte_org = (m * encode_capability / di_bit) ;
    }
    else
    {
        outBuf_size_in_byte_org = (m * encode_capability / di_bit) + 1;    
    }
    
    return outBuf_size_in_byte_org;
    
} 
// Here is the sample code (int main()) for caller 
unsigned char * ECC_Generator_Main(unsigned char *inBuf, unsigned char * outBuf_Post_Proc, int encode_size_in_byte, int encode_capability)
{
  
  // CAUTION: for Dev-C++ compiler, it needs to define unsigned char, 
  // otherwise it will output 32-bit for signed char  
  unsigned char * outBuf = NULL;
  int count_k = 0;
  int outBuf_size_in_byte_org = 0;
  int outBuf_size_in_byte = 0;

/*
  encode_size_in_byte = 108;
  encode_capability = 12;
*/

  if(NULL == inBuf)
  {
      printf("%s:%d Error: inBuf is NULL\r\n", __FUNCTION__, __LINE__);
      return NULL;
  }
  
  if ( (encode_capability != 4) && (encode_capability != 8) && (encode_capability != 12) )
  {
      printf("%s:%d encode_capability[%d] must be 4, 8 or 12!\r\n", __FUNCTION__, __LINE__,encode_capability);
      return NULL;
  }
  
  //MTK_fprintf("inBuf = 0x%X\r\n", inBuf);
  //MTK_fprintf("encode_size_in_byte = %d\r\n", encode_size_in_byte);
  //MTK_fprintf("encode_capability = %d\r\n", encode_capability);
     
  // define encode buffer size and ECC capability 
   /*
  t = 4,  outBuf_size_in_byte_org = 6.5 => 7
  t = 8,  outBuf_size_in_byte_org = 13
  t = 12, outBuf_size_in_byte_org = 19.5 => 20 
  */
  
   
  outBuf_size_in_byte_org = ECC_OutBuf_Size_Cal(encode_capability);
  if(-1 == outBuf_size_in_byte_org) {
		return NULL;
  }
  // use two char to store a ECC byte, so needs multiple by 2 
  outBuf_size_in_byte =   outBuf_size_in_byte_org * 2;
  
  //MTK_fprintf("outBuf_size_in_byte = %d\r\n", outBuf_size_in_byte);
  //getch();
  
  // create input / output buffer 
  outBuf = (unsigned char *)malloc(outBuf_size_in_byte); 

  //memset(inBuf, 0x00, encode_size_in_byte);
  /* for coverity(CID:270133), avoid passing null pointer "outBuf" to "memset"*/
  if(NULL == outBuf){
      printf("%s:%d Error: outBuf is NULL\r\n", __FUNCTION__, __LINE__);
      return NULL;
  }
  memset(outBuf, 0xFF, outBuf_size_in_byte);
      
  if(ECC_Generator(inBuf, outBuf, encode_size_in_byte, encode_capability) != 0) {
  	free(outBuf);
	return NULL;
  }
  
  //MTK_fprintf("Start to dump original ECC parity...\r\n");
  
  for(count_k=0; count_k < outBuf_size_in_byte; count_k++)
  {
    //MTK_fprintf("%x", outBuf[count_k]);
    
    if( (count_k + 1) %2 == 0 )
    {
        //MTK_fprintf("\r\n");                
    }
  }    
  
#if 0  
  outBuf_Post_Proc = (unsigned char *)malloc(outBuf_size_in_byte_org);
  
  if(outBuf_Post_Proc == NULL)
  {
      //MTK_fprintf("Fail to malloc outBuf_Post_Proc...\r\n");
  }
#endif 
  
  ECC_Parity_Post_Proc(outBuf, outBuf_Post_Proc, outBuf_size_in_byte);
  
  //MTK_fprintf("Start to dump post proc ECC parity...\r\n");
  
  for(count_k=0; count_k < outBuf_size_in_byte_org; count_k++)
  {
    //MTK_fprintf("%x\r\n", outBuf_Post_Proc[count_k]);
  }
  free(outBuf);
  
  return (unsigned char *)outBuf_Post_Proc;
}



int ECC_Generator(unsigned char *inBuf, unsigned char *outBuf, int encode_size_in_byte, int encode_capability)
{

  //Variable Declaration {{{1
  //Assign case parameter
  
  int  *meg = NULL,*meg_xdi = NULL,*rem = NULL;
  int   aux_meg, mask; 
  int   degree_g;  //calculate data size  
  int   count_k,count_n;
  int   meg_size,t;
  int   aux_print;
  int   i=0, j=0;
  int	ret = 0;
  //End Variable Declaration }}}1


  //-----------------------------------------------------------------------------
  // Initial Parameter when start message block {{{1
  //-----------------------------------------------------------------------------
  //Input Parameters 
  //-----------------------------------------------------------------------------
  meg_size    = encode_size_in_byte*8;  //bit resolution but always input byte 
  t           = encode_capability;
  
  //strcpy(source_path, argv[3]);
  //MTK_fprintf("meg_size    is %d.\n", meg_size);
  //MTK_fprintf("t           is %d.\n", t);

  //-----------------------------------------------------------------------------
  //Encoder Array 
  meg     = (int *)malloc((meg_size)*sizeof(int));     //message data array      
  if(NULL == meg) {
  	ret = -1;
	goto exit;
  }
  //Force input message bit in Byte resolution
  //aux_meg = (di_bit-(meg_size%di_bit))%di_bit;  //for print dump data file
  aux_meg = 0;
  meg_xdi = (int *) malloc((meg_size+aux_meg)*sizeof(int));
  if(NULL == meg_xdi) {
	ret = -1;
	goto exit;
  }

/*
#define m    13
#define maxt 12
*/
  //8 bit for dummy generate parity
  rem     = (int *)malloc((m*maxt)*sizeof(int));      //Remainder degree defined by maxt
  if(NULL == rem) {
	ret = -1;
	goto exit;
  }
  // CAUTION: Need add Free() later to handle memory leakage
  mask=1;
  for(count_k=0; count_k<meg_size; count_k++)           meg    [count_k]=0; //reset message bit array
  for(count_k=0; count_k<(meg_size+aux_meg); count_k++) meg_xdi[count_k]=0; //reset message bit array
  for(count_k=0; count_k<(m*maxt); count_k++)           rem    [count_k]=0; //reset remainder bit array 

  degree_g=m*maxt-1;                            //max generator polynomial 

  //-}}}1------------------------------------------------------------------------

  // Input Specific Data {{{1  
  //=------------------------------

#ifdef DEBUG
  //MTK_fprintf("loop %d\r\n", ((meg_size/32)+1));
#endif   
  // CAUTION: modify meg_size/32 to meg_size/8, sometimes meg_size is not DWORD alignment 
  for(count_k=0;count_k<(meg_size/8);count_k++) //ECC_source data print to meg array
  {
#ifdef DEBUG_ECC_GEN_MEG
    //MTK_fprintf("\r\n%d\r\n", count_k);                                                 
#endif     

    //fread(&aux_print, sizeof(char), 1, fptrkirk);  
    aux_print = inBuf[count_k];
#ifdef DEBUG_ECC_GEN_MEG
    //MTK_fprintf("\r\n%d, %x\r\n", i, aux_print);
    i++;
#endif                 
    // change count_n(k)<32 to count_n(k)<8 
    for(count_n=0;count_n<8;count_n++)
    {
      j = ( meg_size-1 ) - ( count_k*8 + count_n );

      meg[ ( meg_size-1 ) - ( count_k*8 + count_n ) ]=(mask & aux_print>>count_n); // save each bit of aux_print to meg int array 
      //meg[count_k*8+count_n]=(mask & (aux_print>>(7-count_n)) ); // save each bit of aux_print to meg int array 
#ifdef DEBUG_ECC_GEN_MEG      
      //MTK_fprintf("%d,%x ", j, meg[j] );
#endif       
     
    }
  }

#ifdef DEBUG  
  //MTK_fprintf("end loop %d\r\n", count_k);
#endif
  //=------------------------------}}}1

  //-----------------------------------------------------------------------------
  //Systematic Encoder {{{1
  //-----------------------------------------------------------------------------
  for(count_k=0;count_k<(meg_size);count_k++)
  {
        meg_xdi[count_k] = meg[count_k-aux_meg];        
        ////MTK_fprintf("%x ", meg_xdi[count_k]);
  }      

  // t = 12 (ECC capability user input parameter)
  // meg_size = 512 x 
  // rem = malloc((m*maxt)*sizeof(int)); m = 13, maxt = 12 
    //bch_encoder_hw(meg_xdi, meg_size, t, rem, parity_path);
    bch_encoder_hw(meg_xdi, meg_size, t, rem, outBuf);
    
  //--}}}1-----------------------------------------------------------------------
  //-----------------------------------------------------------------------------
exit:
	if(NULL != meg) {
		free(meg);
	}
	if(NULL != rem) {
		free(rem);
	}
	if(NULL != meg_xdi) {
		free(meg_xdi);
	}
  
	return ret;
}  //end BCH_BMA_HW.c


void ECC_Parity_Post_Proc(unsigned char *inBuf, unsigned char *outBuf, int outBuf_size_in_byte)
{
    int count=0;
    int count_k=0;
    
    for(count=0, count_k=0; count < outBuf_size_in_byte ; count = count+2, count_k++)
    {
        outBuf[count_k] = inBuf[count]*16 + inBuf[count+1];
#ifdef DEBUG                          
        //MTK_fprintf("%x, %x, %x\r\n", inBuf[count]*16, inBuf[count+1], outBuf[count]);
#endif         
    }    
}
