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
* bch_encoder_hw.cpp
*
* Project:
* --------
*   ALPS
*
* Description:
* ------------
* This file is used to generate ECC parity (lower layer)
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
//Function :     m>1 RSencoder for systematic form = non-binary poly. div. 
//           and m=1 BCH encoder for systematic form = binary poly. div.
//Author   : Celesta
//Date     : 2007/08
//I/O      : meg_xdi = input message polynomial + dummy print bit
//           megsize = degree of input message
//           gen = generator polynomial,input by array
//           k = degree of generator polynomial (2*t)
//           m = genF(2^m) which primitive polynomial
//           alpha_v, alpha_p = alpha table related with primitive polynomial 
//
//=============================================================================
//#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//#include "mtk_debug.h"

#define maxt 12

#undef OUTPUT_FILE 
/*
// t = 12 (ECC capability user input parameter)
// meg_size = 512 x 
// rem = malloc((m*maxt)*sizeof(int)); m = 13, maxt = 12 

  bch_encoder_hw(meg_xdi,meg_size,t,rem, outBuf);
*/


//char *parity_path
void bch_encoder_hw(int *meg_xdi,int meg_size, int t ,int *rem, unsigned char *outBuf)
{
	
	//Variable Decalration {{{1
	int  count_k,count_n,count_r;
	int  aux_rem[4]={0};
	int  mux[32]={0};
	int  g [157];
	int  di[8]={0};
	int  rem_up;
	int  fb0 ,fb1 ,fb2 ,fb3, fb4, fb5, fb6, fb7;
#ifdef OUTPUT_FILE                        
	FILE *fptrpar;
#endif   
	int  aux_print;
	char enc_path [256]="" ;
	int  m = 13;
	int  di_bit = 8;
	int  count_m=0;
	int  outBuf_size_in_byte_org = 0;
	
	//matrix high order put in gen[0]
	int gen [157];// = {7,4,5,4,2,0,7,0,3,6,2,3,2,2,4,5,7,0,0,4,6,0,3,4,0,2,6,3,5,1,4,5,0,1,2,3,7,3,0,2,6,0,5,1,4,7,2,5,0,1,4,1,7,0,4,6,2,2,4,4,4,4,2,2,2,0,6,0,0,4,0,4,2,2,6,4,0,4,2,0,6,2,6,0,0,0,2,0,0,6,2,6,2,2,4,2,2,4,4,2,0,0,4,6,2,4,4,4,0,0,4,4,0,4,4,0,4,0,4,4,4,4,4,4,0,0,0,0,4,0,0,4,0,0,0,0,0,0,4,0,4,4,4,4,0,0,0,0,0,0,4,0,4,0,0,0,4}; 
	int g4 [4*13+1]={1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,1,0,1,0,1,0,1,1,1,0,0,0,0,1,1,0,1,
		0,1,0,1,0,1,1}; //0~52 degree=52 + 1
	int g6 [6*13+1]={ 1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,1,0,0,1,0,0,1,
		1,1,1,0,0,0,0,1,1,0,1,1,1,0,0,1,0,1,1,1,0,0,1,1,0,1,1,0,0,0,1,0,1,1,1,1,1,0,1}; //0~78
	int g8 [8*13+1]={1,0,0,0,1,0,1,0,1,1,1,1,1,1,0,0,1,0,0,0,1,0,1,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,0,1,1,0,0,0,0,1,
		1,0,0,0,0,0,1,0,0,1,1,1,0,0,0,0,1,1,1,0,1,0,0,0,0,0,1,1,1,0,0,0,1,0,1,1,1,0,0,0,1,0,0,1,1,1,
		1,1,0,1,1,0,0,1,0,0,0,1,1}; //0~104
	int g10[10*13+1]={1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,1,0,1,1,1,1,0,0,0,0,1,1,0,1,0,0,1,0,1,0,0,0,0,
		1,1,1,1,1,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,1,1,1,1,0,0,0,0,0,1,
		0,1,0,1,0,1,1,0,0,0,0,0,0,0,1,0,1,0,1,0,1,1,1,1,1,1,0,0,0,0,0,1,0,1,1,0,0,1,1,0,
		1,1,1,1,1,0,0,1,0,0,1};
	int g12[12*13+1]={1,1,1,1,0,0,1,0,0,1,0,0,0,0,1,1,1,0,0,1,1,0,0,1,0,0,1,0,1,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,1,1,
		0,1,0,0,1,0,1,0,1,1,0,0,1,1,1,1,0,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
		0,0,1,0,0,1,1,0,0,0,1,1,0,1,1,1,0,0,1,1,0,1,1,0,1,0,1,1,1,1,1,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
		1,0,1,1,1,1,0,0,0,0,0,0,1,0,1,0,0,0,1}; 
	//low indx->high indx //high order -> low order
	//End Variable Declaration }}}1
	
	//Setting and Initial all Data Array {{{1
	//-----------------------------------------------------------------------------
	
	//all combination of generation polynomial 
	mux[ 0]=0;                                          
	mux[ 1]=(t==4);
	mux[ 2]=(t==6);
	mux[ 3]=(t==4)||(t==6);
	mux[ 4]=(t==8);
	mux[ 5]=(t==8)||(t==4);
	mux[ 6]=(t==8)||(t==6);
	mux[ 7]=(t==8)||(t==6)||(t==4);
	mux[ 8]=(t==10);                     
	mux[ 9]=(t==10)||(t==4);                
	mux[10]=(t==10)||(t==6);                
	mux[11]=(t==10)||(t==4)||(t==6);        
	mux[12]=(t==10)||(t==8);                
	mux[13]=(t==10)||(t==8)||(t==4);        
	mux[14]=(t==10)||(t==8)||(t==6);        
	mux[15]=(t==10)||(t==8)||(t==6)||(t==4);
	mux[16]=(t==12);                             
	mux[17]=(t==12)||(t==4);                         
	mux[18]=(t==12)||(t==6);                         
	mux[19]=(t==12)||(t==4)||(t==6);                 
	mux[20]=(t==12)||(t==8);                         
	mux[21]=(t==12)||(t==8)||(t==4);                 
	mux[22]=(t==12)||(t==8)||(t==6);                 
	mux[23]=(t==12)||(t==8)||(t==6)||(t==4);         
	mux[24]=(t==12)||(t==10);                        
	mux[25]=(t==12)||(t==10)||(t==4);                 
	mux[26]=(t==12)||(t==10)||(t==6);                
	mux[27]=(t==12)||(t==10)||(t==4)||(t==6);        
	mux[28]=(t==12)||(t==10)||(t==8);                
	mux[29]=(t==12)||(t==10)||(t==8)||(t==4);        
	mux[30]=(t==12)||(t==10)||(t==8)||(t==6);        
	mux[31]=(t==12)||(t==10)||(t==8)||(t==6)||(t==4);
	
	for(count_k=0;count_k<=m*maxt;count_k++) //0~152
	{
		if(count_k<=4*13)       gen[m*maxt-count_k]=g12[count_k]*16+g10[count_k]*8+g8[count_k]*4+g6[count_k]*2+g4[count_k]; 
		else if(count_k<=6*13)  gen[m*maxt-count_k]=g12[count_k]*16+g10[count_k]*8+g8[count_k]*4+g6[count_k]*2;   
		else if(count_k<=8*13)  gen[m*maxt-count_k]=g12[count_k]*16+g10[count_k]*8+g8[count_k]*4;
		else if(count_k<=10*13) gen[m*maxt-count_k]=g12[count_k]*16+g10[count_k]*8;
		else                    gen[m*maxt-count_k]=g12[count_k]*16;
	} //gen is high order put in [156]
	
	for(count_k=0;count_k<=m*maxt;count_k++)    g[count_k]=gen[count_k]; //g is high order put in [156]
	
	for(count_k=0;count_k<(m*maxt);count_k++)  rem[count_k] = 0; 
	
	//}}}1
	//-----------------------------------------------------------------------------
	
	//M divides into gen 
	//-----------------------------------------------------------------------------
	// di_bit = 8
	for(count_k=0;count_k<(meg_size/di_bit);count_k++) //once input 8 bits data
	{
		//=------------------------------
		//Prepare Data and Print Data
		//=------------------------------
		//assign input to di register MSB to LSB 
#ifdef DEBUG_ECC_ENCODE_DI        
		//MTK_fprintf("\r\n +di[count_n]\r\n");
#endif     
		for(count_n=0;count_n<di_bit;count_n++) 
		{
			di[count_n] =meg_xdi[(meg_size-1)-count_k*8-count_n];
			//di[count_n] =meg_xdi[count_k*8-count_n];
#ifdef DEBUG_ECC_ENCODE_DI      
			//MTK_fprintf("%x ", di[count_n]);
#endif       
		}
#ifdef DEBUG_ECC_ENCODE_DI        
		//MTK_fprintf("\r\n -di[count_n]\r\n");
#endif     
		//=------------------------------
		
		//=------------------------------
		//Generate Parity  
		//=------------------------------
		for(count_n=0;count_n<(di_bit/8);count_n++)//for 8 = 0,1  for 16 = 0,1,2,3
		{
			
			fb0 = rem[155] ^                    di[0]; //g[156]=1
			fb1 =(rem[154])^(fb0 & mux[g[155]])^di[1];
			fb2 =(rem[153])^(fb0 & mux[g[154]])^(fb1 & mux[g[155]])^di[2];
			fb3 =(rem[152])^(fb0 & mux[g[153]])^(fb1 & mux[g[154]])^(fb2 & mux[g[155]])^di[3];  
			fb4 =(rem[151])^(fb0 & mux[g[152]])^(fb1 & mux[g[153]])^(fb2 & mux[g[154]])^(fb3 & mux[g[155]])^di[4];  
			fb5 =(rem[150])^(fb0 & mux[g[151]])^(fb1 & mux[g[152]])^(fb2 & mux[g[153]])^(fb3 & mux[g[154]])^(fb4 & mux[g[155]])^di[5];  
			fb6 =(rem[149])^(fb0 & mux[g[150]])^(fb1 & mux[g[151]])^(fb2 & mux[g[152]])^(fb3 & mux[g[153]])^(fb4 & mux[g[154]])^(fb5 & mux[g[155]])^di[6];  
			fb7 =(rem[148])^(fb0 & mux[g[149]])^(fb1 & mux[g[150]])^(fb2 & mux[g[151]])^(fb3 & mux[g[152]])^(fb4 & mux[g[153]])^(fb5 & mux[g[154]])^(fb6 & mux[g[155]])^di[7];  
			
			//update remainder/parity
			for(count_r=155;count_r>=0;count_r--) //the 156 bit remainder 
			{
				//count_r counts the remainder number
				
				rem_up=count_r-8;
				if (count_r>=8) 
				{
					rem[count_r]=rem[rem_up] ^ (fb0 & mux[g[rem_up+1]]) ^ (fb1 & mux[g[rem_up+2]]) ^ (fb2 & mux[g[rem_up+3]]) ^ (fb3  & mux[g[rem_up+4]])
						^ (fb4 & mux[g[rem_up+5]]) ^ (fb5 & mux[g[rem_up+6]]) ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7  & mux[g[rem_up+8]]);
				}
				else if (count_r==7)
				{
					rem[count_r]=0 ^ (fb0 & mux[g[rem_up+1]]) ^ (fb1 & mux[g[rem_up+2]]) ^ (fb2 & mux[g[rem_up+3]]) ^ (fb3 & mux[g[rem_up+4]])
						^ (fb4 & mux[g[rem_up+5]]) ^ (fb5 & mux[g[rem_up+6]]) ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7 & mux[g[rem_up+8]]);
				}
				
				else if (count_r==6)
				{
					rem[count_r]=0                            ^ (fb1 & mux[g[rem_up+2]]) ^ (fb2 & mux[g[rem_up+3]]) ^ (fb3 & mux[g[rem_up+4]])
						^ (fb4 & mux[g[rem_up+5]]) ^ (fb5 & mux[g[rem_up+6]]) ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7 & mux[g[rem_up+8]]);
				}
				
				else if (count_r==5)
				{
					rem[count_r]=0                                                       ^ (fb2 & mux[g[rem_up+3]]) ^ (fb3 & mux[g[rem_up+4]])
						^ (fb4 & mux[g[rem_up+5]]) ^ (fb5 & mux[g[rem_up+6]]) ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7 & mux[g[rem_up+8]]);
				}
				
				else if (count_r==4)
				{
					rem[count_r]=0                                                                                  ^ (fb3 & mux[g[rem_up+4]])
						^ (fb4 & mux[g[rem_up+5]]) ^ (fb5 & mux[g[rem_up+6]]) ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7 & mux[g[rem_up+8]]);
				}
				
				else if (count_r==3)
				{
					rem[count_r]=0 ^ (fb4 & mux[g[rem_up+5]]) ^ (fb5 & mux[g[rem_up+6]]) ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7  & mux[g[rem_up+8]]);
				}
				
				else if (count_r==2)
				{
					rem[count_r]=0 ^ (fb5 & mux[g[rem_up+6]]) ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7 & mux[g[rem_up+8]]);
				}
				
				else if (count_r==1)
				{
					rem[count_r]=0 ^ (fb6 & mux[g[rem_up+7]]) ^ (fb7 & mux[g[rem_up+8]]);
				}
				
				else if (count_r==0)
				{
					rem[count_r]=0 ^ (fb7 & mux[g[rem_up+8]]);
				}
				
			}
		} 
	}
	
	//-----------------------------------------------------------------------------
	
#ifdef OUTPUT_FILE                      
	strcpy(enc_path,parity_path);
	
	if (((fptrpar = fopen(enc_path,"w"))==NULL)) 
		//MTK_fprintf("parity could not open");
		else //MTK_fprintf("parity open is okay\n"); 
#endif 
		// CAUTION: (t*m)/di_bit can not handle case (t = 8), so need modify 
		if ( 0 == (m * t) % di_bit )
		{
			outBuf_size_in_byte_org = (m * t / di_bit) ;
		}
		else
		{
			outBuf_size_in_byte_org = (m * t / di_bit) + 1;    
		}
		
		//count_k<(((t*m)/di_bit)+1)
		
		for(count_k=0; count_k<outBuf_size_in_byte_org; count_k++) //t=4 0~6
		{
			for(count_n=(di_bit/4)-1;count_n>=0;count_n--, count_m++)
			{
				
				if((t==4  && (count_k*di_bit+count_n*4 >= 51))) //final byte exceed 52 
				{
					outBuf[count_m] = 0xF;
#ifdef DEBUG_ECC_OUPUT
					MTK_fprintf("F");
#endif
					
#ifdef OUTPUT_FILE
					fprintf(fptrpar,"F");
#endif 
					
				}
				else if( (t==12 && (count_k*di_bit+count_n*4 >= 155) ) )//final byte exceed 156
				{
					
					outBuf[count_m] = 0;
#ifdef DEBUG_ECC_OUPUT			  
					MTK_fprintf("0");
#endif 
#ifdef OUTPUT_FILE
					fprintf(fptrpar,"0");  
#endif 
				}
				else
				{
					aux_print=rem[155-(count_k*di_bit+count_n*4  )]<<0 | rem[155-(count_k*di_bit+count_n*4+1)]<<1 
						| rem[155-(count_k*di_bit+count_n*4+2)]<<2 | rem[155-(count_k*di_bit+count_n*4+3)]<<3;
#ifdef DEBUG_ECC_OUPUT          
					//MTK_fprintf("%x",aux_print);
#endif 
					
#ifdef OUTPUT_FILE                      
					//fprintf(fptrpar,"%x",aux_print);
#endif 
					outBuf[count_m] = aux_print;            
				}
#ifdef DEBUG_ECC_ENCODE_DI        
				if(count_n==0) //MTK_fprintf("\n");
#endif         
					
#ifdef OUTPUT_FILE                              
					if(count_n==0) f//MTK_fprintf(fptrpar,"\n");
#endif             
			}
		}
		//-----------------------------------------------------------------------------
}
