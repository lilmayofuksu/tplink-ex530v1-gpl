//#define DEBUG
//#define DEBUG_2
#define VPint			*(volatile unsigned long int *)

#define S_256     (0x100)
#define S_512     (0x200)
#define S_4K      (0x1000)
#define S_32K     (0x8000)
#define S_40K     (0xa000)
#define S_48K     (0xc000)
#define S_64K     (0x10000)
#define S_256K    (0x40000)
#define S_384K    (0x60000)

#define MPIS_UNCAC_BASE         (0xa0000000)
#define MPIS_CACHE_BASE         (0x80000000)

#define NPU_4K_SRAM_TSIZE       S_512
#define NPU_4K_SRAM_ADDR7       (0x1ec0a800+(NPU_4K_SRAM_TSIZE*7))
#define NPU_4K_SRAM_UNC_ADDR7   (NPU_4K_SRAM_ADDR7+MPIS_UNCAC_BASE)
#define NPU_4K_SRAM_CAC_ADDR7   (NPU_4K_SRAM_ADDR7+MPIS_CACHE_BASE)

/*Note: NPU 64K SRAM range: 0x1e900000~0x1e910000. 
 *      the first 32K includes data/bss/heap sections, so don't test it.*/
#define NPU_64K_SRAM_TSIZE      S_4K
#define NPU_64K_SRAM_ADDR7      (0x1e908000+(NPU_64K_SRAM_TSIZE*7)) /*0x0x1e90f000*/
#define NPU_64K_SRAM_UNC_ADDR7  (NPU_64K_SRAM_ADDR7+MPIS_UNCAC_BASE)
#define NPU_64K_SRAM_CAC_ADDR7  (NPU_64K_SRAM_ADDR7+MPIS_CACHE_BASE)

#define NPU_384K_SRAM_CPU_TSIZE S_40K /*just use 40K, because HSDMA will use the last 8K*/
#define NPU_384K_SRAM_TSIZE     S_48K
#define NPU_384K_SRAM_ADDR7     (0x1e800000+(NPU_384K_SRAM_TSIZE*7)) /*0x1e854000*/
#define NPU_384K_SRAM_UNC_ADDR7 (NPU_384K_SRAM_ADDR7+MPIS_UNCAC_BASE)
#define NPU_384K_SRAM_CAC_ADDR7 (NPU_384K_SRAM_ADDR7+MPIS_CACHE_BASE)

#define ALL_FF  (0xffffffff)

unsigned int unc_access_test_addr[] = {NPU_4K_SRAM_UNC_ADDR7, NPU_64K_SRAM_UNC_ADDR7, NPU_384K_SRAM_UNC_ADDR7, 0};
unsigned int cac_access_test_addr[] = {NPU_4K_SRAM_CAC_ADDR7, NPU_64K_SRAM_CAC_ADDR7, NPU_384K_SRAM_CAC_ADDR7, 0};
unsigned int access_test_size[] = {NPU_4K_SRAM_TSIZE, NPU_64K_SRAM_TSIZE, NPU_384K_SRAM_CPU_TSIZE, 0};

typedef enum {
	ORI_PAT,
	INCR_PAT,
	ANTI_INCR_PAT,
} patType;


unsigned char defWByte[] = {1, 2 , 4 , 0};
unsigned char defRByte[] = {1, 2 , 4 , 0};
unsigned char defPat[] = {0x55, 0xaa, 0x5a, 0xa5, 0x00, 0xff, 0x12};

#ifdef TCSUPPORT_LITTLE_ENDIAN
#define SWAP_2B(x)  ( (((x)&0xff)<<8) | (((x)&0xff00)>>8) )
#define SWAP_4B(x)  ( (((x)&0xff)<<24) | (((x)&0xff00)<<8) | (((x)&0xff0000)>>8) | (((x)&0xff000000)>>24) )
#else
#define SWAP_2B(x)  (x)
#define SWAP_4B(x)  (x)
#endif


static int dram_pat_set(void *startAddr, unsigned long size, unsigned long pattern, int patType,unsigned char wByte)
{
	unsigned char *addr1;
	unsigned char pat1;
	unsigned short *addr2;
	unsigned short pat2;
	unsigned long *addr4;
	unsigned long pat4;
	unsigned char antiPat1;
	unsigned char antiPat2;
	unsigned char antiPat4;
	int err=0;

    #ifdef DEBUG
    printk("%s\n", __func__);
    #endif

	switch(wByte)
	{
		case 4:
			pat4 = (unsigned long)(pattern & 0xffffffff);
			addr4 = (unsigned long*)startAddr;
			size = size >> 2;
            #ifdef DEBUG
            printk("wByte%d pat:0x%x addr:0x%x size:0x%x \n", wByte, pat4, addr4, size);
            #endif
			break;
		case 2:
			pat2 = (unsigned short)(pattern & 0xffff);
			addr2 = (unsigned short*)startAddr;
			size = size >> 1;
            #ifdef DEBUG
            printk("wByte%d pat:0x%x addr:0x%x size:0x%x \n", wByte, pat2, addr2, size);
            #endif
			break;
		case 1:
			pat1 = (unsigned char)(pattern & 0xff);
			addr1 = (unsigned char*)startAddr;
            #ifdef DEBUG
            printk("wByte%d pat:0x%x addr:0x%x size:0x%x \n", wByte, pat1, addr1, size);
            #endif
			break;
		default:
        #ifdef DEBUG
			printk("dram_pat_set: ERROR! wByte=0x%x\n", wByte);
        #endif
			err=-1;;
	}


	while(size&&(err!=-1))
	{
		switch(wByte)
		{
			case 4:
				if(patType == ANTI_INCR_PAT)
				{
					antiPat4 = ~pat4;
					*addr4 = antiPat4;
				}
				else
				{
					*addr4 = pat4;
				}

                #ifdef DEBUG
				printk("wByte4 pat set: addr:0x%x, data:0x%x\n", addr4, (*addr4));
			    #endif

			    addr4++;
                
				if((patType == INCR_PAT) || (patType == ANTI_INCR_PAT))
				{
					pat4++;
				}
				break;
			case 2:
				if(patType == ANTI_INCR_PAT)
				{
					antiPat2 = ~pat2;
					*addr2 = antiPat2;
				}
				else
				{
					*addr2 = pat2;
				}
				#ifdef DEBUG
				printk("wByte2 pat set: addr:0x%x, data:0x%x\n", addr2, (*addr2));
				#endif

			    addr2++;

				if((patType == INCR_PAT) || (patType == ANTI_INCR_PAT))
				{
					pat2++;
				}
				break;
			case 1:
				if(patType == ANTI_INCR_PAT)
				{
					antiPat1 = ~pat1;
					*addr1 = antiPat1;
				}
				else
				{
				#ifdef DEBUG
                printk("wByte1 pat set: addr1:0x%x, pat:0x%x\n", addr1, pat1);
				#endif
					*addr1 = pat1;
				}

                #ifdef DEBUG
                printk("wByte1 pat set: addr:0x%x, data:0x%x\n", addr1, (*addr1));
				#endif

			    addr1++;
                #ifdef DEBUG
                printk("wByte1 pat set: addr:0x%x\n", addr1);
				#endif

				if((patType == INCR_PAT) || (patType == ANTI_INCR_PAT))
				{
					pat1++;
				}
				break;
			default:
            #ifdef DEBUG
				printk("dram_pat_set: ERROR! wByte=0x%x\n", wByte);
            #endif
				err=-1;
		}

		size--;
	}

	return err;
}

static int dram_pat_cmp(void *startAddr, unsigned long size, unsigned long pattern, unsigned char wByte, unsigned char rByte)
{
	unsigned char *addr1;
	unsigned char pat1;
	unsigned short *addr2;
	unsigned short pat2;
	unsigned long *addr4;
	unsigned long pat4;
	unsigned long wPat;
	unsigned char pNum;
	unsigned long rPat[4] = {0};
	int i, n, err=0;
	

	#ifdef DEBUG
	printk("dram_pat_cmp:\n");
    printk("startAddr:0x%x, size:0x%x, pattern:0x%x, wByte:0x%x, rByte:0x%x\n", startAddr, size, pattern, wByte, rByte);
	#endif
	switch(wByte)
	{
		case 4:
			wPat = pattern & 0xffffffff;
			break;
		case 2:
			wPat = pattern & 0xffff;
			break;
		case 1:
			wPat = pattern & 0xff;
			break;
		default:
            #ifdef DEBUG
			printk("dram_pat_cmp: ERROR! wByte=0x%x\n",wByte);
            #endif
			err=-1;
	}

	switch(rByte)
	{
		case 4:
			addr4 = (unsigned long*)startAddr;
			size = size >> 2;
			break;
		case 2:
			addr2 = (unsigned short*)startAddr;
			size = size >> 1;
			break;
		case 1:
			addr1 = (unsigned char*)startAddr;
			break;
		default:
            #ifdef DEBUG
			printk("dram_pat_cmp: ERROR! read byte=0x%x\n", rByte);
            #endif
			err=-1;
	}

	if(wByte > rByte)
	{
		pNum = wByte / rByte;
		for(i=pNum-1; i>=0 ;i--)
		{
			switch(rByte)
			{
				case 2:
					rPat[i] = wPat & 0xffff;
					wPat = wPat >> 16;
					break;
				case 1:
					rPat[i] = wPat & 0xff;
					wPat = wPat >> 8;
					break;
				default:
                    #ifdef DEBUG
					printk("dram_pat_cmp: ERROR! read byte=0x%x\n",rByte);
                    #endif
					err=-1;
			}
		}
	}
	else if(wByte < rByte)
	{
		pNum = 1;
		n = rByte / wByte;
		for(i=0; i<(n-1); i++)
		{
			switch(wByte)
			{
				case 2:
					wPat = (wPat << 16) | wPat;
					break;
				case 1:
					wPat = (wPat << 8) | wPat;
					break;
				default:
                    #ifdef DEBUG
					printk("dram_pat_cmp: ERROR! wByte=0x%x\n",wByte);
                    #endif
					err=-1;
			}
		}
		rPat[0] = wPat;
	}
	else
	{
		pNum = wByte / rByte;
		rPat[0] = wPat;
	}

	i = 0;
    
	while(size&&(err!=-1))
	{
		switch(rByte)
		{
			case 4:
				pat4 = (unsigned long) rPat[i] & 0xffffffff;
				if(*addr4 != pat4)
				{
                    printk("error: rByte:4 *addr:0x%x != pat4:0x%x at addr:0x%x\n", *addr4, pat4, addr4);
                    err=-1;
				}
			    addr4++;
                
				break;
			case 2:
				pat2 = (unsigned short) rPat[i] & 0xffff;
				if(*addr2 != pat2)
				{
                    printk("error: rByte:2 *addr:0x%x != pat2:0x%x at addr:0x%x\n", *addr2, pat2, addr2);
					err=-1;
				}

			    addr2++;

				break;
			case 1:
				pat1 = (unsigned char) rPat[i] & 0xff;
				if(*addr1 != pat1)
				{
                    printk("error: rByte:1 *addr:0x%x != pat1:0x%x at addr:0x%x\n", *addr1, pat1, addr1);
					err=-1;
				}

			    addr1++;

				break;
			default:
                #ifdef DEBUG
				printk("dram_pat_cmp: ERROR! read byte=0x%x\n", rByte);
                #endif
				err=-1;
		}

		i++;
		if(i >= pNum)
		{
			i = 0;
		}
        
		size--;
	}
    
    if (err==-1)
       printk("dram_pat_cmp: fail\n"); 
	return err;
}


static int dram_incrPat_cmp(void *startAddr, unsigned long size, unsigned long pattern, unsigned char wByte, unsigned char rByte)
{
	unsigned char *addr1;
	unsigned char pat1;
	unsigned short *addr2;
	unsigned short pat2;
	unsigned long *addr4;
	unsigned long pat4;
	int i, n;
	unsigned long rPat[4] = {0};
	unsigned char pNum;
	int patIdx = -1;
	unsigned long tmp;
	unsigned char shiftByte;
	int err=0;
    unsigned short tmp2b;
    unsigned long tmp4b;


	switch(wByte)
	{
		case 4:
			pat4 = (unsigned long)(pattern & 0xffffffff);
			break;
		case 2:
			pat2 = (unsigned short)(pattern & 0xffff);
			break;
		case 1:
			pat1 = (unsigned char)(pattern & 0xff);
			break;
		default:
            #ifdef DEBUG
			printk("dram_incrPat_cmp: ERROR! wByte=0x%x\n", wByte);
            #endif
			err=-1;
	}
	switch(rByte)
	{
		case 4:
			addr4 = (unsigned long*)startAddr;
			size = size >> 2;
			break;
		case 2:
			addr2 = (unsigned short*)startAddr;
			size = size >> 1;
			break;
		case 1:
			addr1 = (unsigned char*)startAddr;
			break;
		default:
            #ifdef DEBUG
			printk("dram_incrPat_cmp: ERROR! read byte=0x%x\n", rByte);
            #endif
			err=-1;
	}


	while(size&&(err!=-1))
	{
        #ifdef DEBUG
		printk("patIdx:0x%x\n", patIdx);
        #endif
        
		if(wByte > rByte)
		{
			pNum = wByte / rByte;
			if((patIdx >= pNum-1) || (patIdx == -1))
			{
				switch(wByte)
				{
					case 4:
						tmp = SWAP_4B(pat4);
						pat4++;
						break;
					case 2:
						tmp = SWAP_2B(pat2);
                        #ifdef DEBUG
						printk("pat2:0x%x, tmp:0x%x\n", pat2, tmp);
                        #endif
						pat2++;
						break;
					case 1:
						tmp = pat1;
						pat1++;
						break;
					default:
                        #ifdef DEBUG
						printk("dram_incrPat_cmp: ERROR! wByte=0x%x\n", wByte);
                        #endif
						err=-1;
				}
				for(i=pNum-1; i>=0 ;i--)
				{
					switch(rByte)
					{
						case 2:
							rPat[i] = tmp & 0xffff;
							tmp = tmp >> 16;
							break;
						case 1:
							rPat[i] = tmp & 0xff;
							tmp = tmp >> 8;
                            #ifdef DEBUG
    						printk("rPat[i]:0x%x\n", rPat[i]);
                            #endif
							break;
						default:
                            #ifdef DEBUG
							printk("dram_incrPat_cmp: ERROR! read byte=0x%x\n", rByte);
                            #endif
							err=-1;
					}
				}
				patIdx = 0;
			}
			else
			{
				patIdx++;
			}
		}
		else if(wByte < rByte)
		{
			/* take wByte=1 rByte=4 pat1=0x01 for example,
			 * it will get rPat[0]= 0 | ((0x01 <<8) | 0x02) in the first for while,
			 * and then get rPat[0]= (0x0102 << 16 )| ((0x03 <<8) | 0x04) in the second for while.
			 * In this case, n=2 , so the result is rPat[0]=0x01020304 */
			patIdx = 0;
			n = rByte / (wByte * 2);
			rPat[patIdx] = 0;
			shiftByte = 1;

			for(i=0; i<n; i++)
			{
				switch(wByte)
				{
					case 2:
						rPat[patIdx] = (rPat[patIdx] << 16) | ((SWAP_2B(pat2) << 16));
                        pat2++;
                        rPat[patIdx] |= (SWAP_2B(pat2));
						pat2++;
						break;
					case 1:
						rPat[patIdx] = (rPat[patIdx] << (8 * shiftByte)) | ((pat1 << 8) | (++pat1));
						pat1++;
						shiftByte = shiftByte << 1;
						break;
					default:
                        #ifdef DEBUG
						printk("dram_incrPat_cmp: ERROR! wByte=0x%x\n", wByte);
                        #endif
						err=-1;
				}
			}
		}
		else
		{
			patIdx = 0;
			switch(wByte)
			{
				case 4:
					rPat[patIdx] = pat4;
					pat4++;
					break;
				case 2:
					rPat[patIdx] = pat2;
					pat2++;
					break;
				case 1:
					rPat[patIdx] = pat1;;
					pat1++;
					break;
				default:
                    #ifdef DEBUG
					printk("dram_incrPat_cmp: ERROR! wByte=0x%x\n", wByte);
                    #endif
					err=-1;
			}
		}
        

		switch(rByte)
		{
			case 4:
                if (wByte==1 || wByte==2)
                    tmp4b = SWAP_4B(*addr4);
                else
                    tmp4b = *addr4;
				if(tmp4b != (unsigned long)(rPat[patIdx] & 0xffffffff))
				{
                    printk("error! rByte:4! *addr:0x%x != rPat:0x%x at addr:0x%x\n", tmp4b, rPat[patIdx], addr4);
					err=-1;
				}

			    addr4++;
                
				break;
			case 2:
                if (wByte==1 || wByte==4)
                    tmp2b = SWAP_2B(*addr2);
                else
                    tmp2b = *addr2;
				if(tmp2b != (unsigned short)(rPat[patIdx] & 0xffff))
				{
                    printk("error! rByte:2! *addr:0x%x != rPat:0x%x at addr:0x%x\n", tmp2b, rPat[patIdx], addr2);
					err=-1;
				}
                
			    addr2++;

				break;
			case 1:
				if(*addr1 != (unsigned char)(rPat[patIdx] & 0xff))
				{
                    printk("error! rByte:1! *addr:0x%x != rPat:0x%x at addr:0x%x patIdx:0x%x\n", *addr1, rPat[patIdx], addr1, patIdx);
					err=-1;
				}
                
			    addr1++;

				break;
			default:
                #ifdef DEBUG
				printk("dram_incrPat_cmp: ERROR! read byte=0x%x\n", rByte);
                #endif
				err=-1;
		}

		size--;
	}

    if (err==-1)
        printk("dram_incrPat_cmp: fail\n");

	return err;
}


static int dram_antiIncrPat_cmp(void *startAddr, unsigned long size, unsigned long pattern, unsigned char wByte)
{
	unsigned char *addr1;
	unsigned char pat1;
	unsigned char antiPat1;
	unsigned short *addr2;
	unsigned short pat2;
	unsigned char antiPat2;
	unsigned long *addr4;
	unsigned long pat4;
	unsigned char antiPat4;
	int err=0;


	#ifdef DEBUG
	printk("dram_antiIncrPat_cmp:\n");
    printk("startAddr:0x%x, size:0x%x, pattern:0x%x, wByte:0x%x\n", startAddr, size, pattern, wByte);
	#endif
	switch(wByte)
	{
		case 4:
			pat4 = (unsigned long)(pattern & 0xffffffff);
			addr4 = (unsigned long*)startAddr;
			size = size >> 2;
			break;
		case 2:
			pat2 = (unsigned short)(pattern & 0xffff);
			addr2 = (unsigned short*)startAddr;
			size = size >> 1;
			break;
		case 1:
			pat1 = (unsigned char)(pattern & 0xff);
			addr1 = (unsigned char*)startAddr;
			break;
		default:
            #ifdef DEBUG
			printk("dram_antiIncrPat_cmp: ERROR! wByte=%d \n", wByte);
			#endif
			err=-1;
	}


	while(size&&(err!=-1))
	{
		switch(wByte)
		{
			case 4:
				antiPat4 = ~pat4;
				if(*addr4 != antiPat4)
				{
					printk("error! wByte:4! *addr:0x%x != antiPat4:0x%x at addr:0x%x\n", *addr4, antiPat4, addr4);
					err=-1;
				}

			    addr4++;
                
				pat4++;
				break;
			case 2:
				antiPat2 = ~pat2;
				if(*addr2 != antiPat2)
				{
                    printk("error! wByte:2! *addr:0x%x != antiPat2:0x%x at addr:0x%x\n", *addr2, antiPat2, addr2);
					err=-1;
				}

			    addr2++;

				pat2++;
				break;
			case 1:
				antiPat1 = ~pat1;
				if(*addr1 != antiPat1)
				{
                    printk("error! wByte:1! *addr:0x%x != antiPat1:0x%x at addr:0x%x\n", *addr1, antiPat1, addr1);
					err=-1;
				}

			    addr1++;

				pat1++;
				break;
			default:
                #ifdef DEBUG
				printk("dram_antiIncrPat_cmp: ERROR! wByte=0x%x\n", wByte);
                #endif
				err=-1;
		}

		size--;
	}

    if (err==-1)
        printk("dram_antiIncrPat_cmp: fail\n");

	return err;
}

/*
 * test_case==1 for NPU uncached access test
 *      -- including NPU 4k/64K/384K SRAM
 * test_case==2 for NPU cached access test
 *      -- including NPU 4k/64K/384K SRAM
 */
int npuRamTest(int test_case)
{
    unsigned int startAddr, testingSize;
    int l, j, k, i, ret;
    unsigned int *start_addr_p;
    unsigned int *test_size_p;

    if (test_case ==1) { 
        
        start_addr_p = unc_access_test_addr;
        test_size_p = access_test_size;
    }
    else if (test_case ==2) {

        start_addr_p = cac_access_test_addr;
        test_size_p = access_test_size;
    }
    else {
        printk("\n\nERROR: wrong test_case:%d\n\n", test_case);
        return -1;
    }

    for(l=0; start_addr_p[l]!=0; l++) {
        
        startAddr = start_addr_p[l];
        testingSize = test_size_p[l];
        ret=0;

        if (test_case==0)
    	    printk("%s startAddr:0x%x, testSize:0x%x\n", __func__, startAddr, testingSize);
        else {
            #ifdef DEBUG_2
            printk("[H]startAddr:0x%x, testSize:0x%x\n", startAddr, testingSize);
            #endif
        }

    	for(j=0; defWByte[j]!=0; j++)
    	{
    	    #ifdef DEBUG_2
            printk("defWByte:0x%x\n", defWByte[j]);
            #endif

    		/* use the default patterns, incremental pattern */
    		dram_pat_set((void*)startAddr, testingSize, 0, INCR_PAT, defWByte[j]);
            
    		for(k=0; defRByte[k]!=0; k++)
    		{
    		    #ifdef DEBUG_2
                printk("defRByte:0x%x\n", defRByte[k]);
                #endif
    			ret += dram_incrPat_cmp((void*)startAddr, testingSize, 0, defWByte[j], defRByte[k]);
                if (ret) 
                    goto npuRamTest_end;
    		}
    	}
        
    	/* use the default patterns, "anti-incremental patten, 0x5a, 0xa5, 0x0, 0xff" */
        #ifdef DEBUG_2
        printk("dram_antiIncrPat test\n");
        #endif
    	dram_pat_set((void*)startAddr, testingSize, 0, ANTI_INCR_PAT, 1);
    	ret += dram_antiIncrPat_cmp((void*)startAddr, testingSize, 0, 1);	
        if (ret) 
            goto npuRamTest_end;

        /* test with different patterns */	
    	for(i=0; i<sizeof(defPat); i++)
    	{
    	    #ifdef DEBUG_2
            printk("defPat:0x%x\n", defPat[i]);
            #endif
    		dram_pat_set((void*)startAddr, testingSize, defPat[i], ORI_PAT, 1);
    		ret += dram_pat_cmp((void*)startAddr, testingSize, defPat[i], 1, 1);
            if (ret) 
                goto npuRamTest_end;
    	}
    }
    
npuRamTest_end:
    if(ret) {
        if (test_case==0)
            printk("%s FAILED!\n", __func__);
        else
            printk("[H]ERROR: access test failed!\n");
        return -1;
    }
    else {
        #ifdef DEBUG_2
        printk("%s SUCCESS!\n", __func__);
        #endif
        return 0;
    }
}


