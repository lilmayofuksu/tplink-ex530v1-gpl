#include <linux/proc_fs.h>
#include "hostadpt.h"

#define K0_TO_PHYSICAL(x) 				(virt_to_phys(x))  /* kseg0 to physical */
#define PHYSICAL_TO_K0(x) 				(phys_to_virt(x))  /* physical to kseg0 */

#define FH_KERN_LOG_OFF 1

#ifndef  FH_KERN_LOG_OFF
    #define log(fmt, args...) printk(KERN_ERR "%s.%d " fmt, __func__, __LINE__, ##args)
#else
    #define  log(fmt, args...)
#endif

#define ecnt_dcache_inv(start,len) ecnt_dma_inv_range(start,(start+len))
volatile unsigned short int reTryTimes;


unsigned int scatter_case1;
unsigned int scatter_case2;
unsigned int scatter_case3;
unsigned int scatter_case4;
unsigned int scatter_case5;





extern void ecnt_dma_inv_range(unsigned long start, unsigned long end);

HOSTADPT_DMA_DSCP_T *pHostadptRX_BASE[HOSTADPT_RX_RING_NUM] = {NULL};
u32 hostadpt_rx_dscp_max_num[HOSTADPT_RX_RING_NUM] = {HOSTADPT_RX0_DSCP_NUM, HOSTADPT_RX1_DSCP_NUM};
u32 hostadpt_rx_dma_idx[HOSTADPT_RX_RING_NUM] = {0};
u32 hostadpt_rx_cpu_idx[HOSTADPT_RX_RING_NUM] = {0};

extern int get_npu_irq(int index);

static DEFINE_SPINLOCK(hostadpt_lock_test);
static int retry_times_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int len = 0;

	len += sprintf(buf +len, "%d\n", reTryTimes);
	if(len > PAGE_SIZE)
	{
		return -ENOBUFS;
	}
	return len;
}

static int retry_times_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char buf[16];
	unsigned long int len = count;
	unsigned short int n;

	//printk();
	if(len >= sizeof(buf))
	{
		len = sizeof(buf) - 1;
	}
	
	if(copy_from_user(buf, buffer, len))
	{
		return -EFAULT;
	}
	buf[len] = "\n";

	n = simple_strtol(buf, NULL, 10);
	if(n > 0)
	{
		reTryTimes = n;
		printk("[HOSTADPT]retry times is %d\n", reTryTimes);
	}
	else
		printk("retry times should be greater than 0\n");
	
	return len;
}

static int debug_cnt_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int len = 0;

	len += sprintf(buf +len, "%d %d %d %d %d\n", scatter_case1,scatter_case2,scatter_case3,scatter_case4,scatter_case5);
	if(len > PAGE_SIZE)
	{
		return -ENOBUFS;
	}
	printk("scatter_case1:%d,scatter_case2:%d,scatter_case3:%d,scatter_case4:%d,scatter_case5:%d\n", \
		     scatter_case1,scatter_case2,scatter_case3,scatter_case4,scatter_case5);
	return len;
}

static int debug_cnt_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char buf[64];
	unsigned long int len = count;
	unsigned short int n;
	unsigned int cnt1, cnt2, cnt3, cnt4, cnt5;


	//printk();
	if(len >= sizeof(buf))
	{
		len = sizeof(buf) - 1;
	}
	
	if(copy_from_user(buf, buffer, len))
	{
		return -EFAULT;
	}
	sscanf(buf, "%d %d %d %d %d", 
		&cnt1, &cnt2, &cnt3, &cnt4, &cnt5);

	scatter_case1 = cnt1;
	scatter_case2 = cnt2;
	scatter_case3 = cnt3;
	scatter_case4 = cnt4;
	scatter_case5 = cnt5;

	
	return len;
}

static int hostadpt_rxdscp_init(void)
{
	dma_addr_t dscpDmaAddr=0;
	int i;
	u32 dmaIdx = 0;
	struct device *dev = NULL ;
	unsigned long dscpBaseAddr=0 ;
	HOSTADPT_DMA_DSCP_T *dscp = NULL;
	struct sk_buff *skb = NULL;
	struct proc_dir_entry *entry;
	struct proc_dir_entry *entry_debug;

	/******************************************
	* Allocate descriptor DMA memory		  *
	*******************************************/
	if( (dev=get_frame_engine_dev()) == NULL ) {
		HOSTADPT_MSG(DBG_ERR, "Get device failed.\n") ; 
		return -ENOMEM ;
	}
	dscpBaseAddr = (unsigned long)dma_alloc_coherent(dev, sizeof(HOSTADPT_DMA_DSCP_T)*HOSTADPT_RX_TOTAL_DSCP_NUM, &dscpDmaAddr, GFP_KERNEL) ;
	if(!dscpBaseAddr) {
		HOSTADPT_MSG(DBG_ERR, "Allocate memory for TX/RX DSCP failed.\n") ; 
		return -ENOMEM ;
	}

	pHostadptRX_BASE[0] = (HOSTADPT_DMA_DSCP_T *)dscpBaseAddr ;
	pHostadptRX_BASE[1] = pHostadptRX_BASE[0] + HOSTADPT_RX0_DSCP_NUM;

	HOSTADPT_MSG(DBG_LOG, "dscpDmaAddr =%x\n",dscpDmaAddr) ; 

	/***************************************************
	* Initialization first DSCP for Rx DMA			  *
	****************************************************/
	dscp = pHostadptRX_BASE[0];
	for(i=0; i<HOSTADPT_RX_TOTAL_DSCP_NUM; i++) {		
		skb = alloc_skb(HOSTAPD_BUFFER_LEN, GFP_KERNEL); 
		if(skb == NULL)
		{
			HOSTADPT_MSG(DBG_ERR, "alloc skb faild!");
			return -ENOMEM ;
		}
		if ((dev=get_gdmpSram_dev())==NULL) 
		{
			HOSTADPT_MSG(DBG_ERR, "\nget dev failed\n");
			kfree_skb(skb);
			return -ENOMEM;
		}
		
		dscp->skb_p = skb;
		dscp->pkt_addr = dma_map_single(dev, (void *)((unsigned long)skb->data), HOSTAPD_BUFFER_LEN, DMA_FROM_DEVICE);
		dscp->pkt_ctrl.done = 0;
		//dma_unmap_single(dev, dscp->pkt_addr, HOSTAPD_BUFFER_LEN, DMA_FROM_DEVICE);
		dscp++;	
	}
	
	
	//write rx0/rx1 num to reg
	set_npu_hostadpt_reg(HOSTADPT_RX0_MAX_CNT, HOSTADPT_RX0_DSCP_NUM);
	set_npu_hostadpt_reg(HOSTADPT_RX1_MAX_CNT, HOSTADPT_RX1_DSCP_NUM);
	
	//set rx0/rx1 hostadpt_rx_cpu_idx/hostadpt_rx_dma_idx 
	set_npu_hostadpt_reg(HOSTADPT_RX0_CPU_IDX, 0);
	set_npu_hostadpt_reg(HOSTADPT_RX1_CPU_IDX, 0);
	
	set_npu_hostadpt_reg(HOSTADPT_RX0_DMA_IDX, 0);
	set_npu_hostadpt_reg(HOSTADPT_RX1_DMA_IDX, 0);
	
	//write rx0/rx1 base to reg
	//Please add other code before this line which is about initialization. 
	set_npu_hostadpt_reg(HOSTADPT_RX0_BASE_PTR, dscpDmaAddr);
	set_npu_hostadpt_reg(HOSTADPT_RX1_BASE_PTR, dscpDmaAddr + sizeof(HOSTADPT_DMA_DSCP_T)*HOSTADPT_RX0_DSCP_NUM);
	// create a proc for setting retry times
	entry = create_proc_entry("tc3162/hostadpt_set_retry_times", 0666, NULL);
	if(entry == NULL)
	{
		printk(KERN_WARNING"proc: unable to create proc entry\n");
		return -ENOMEM;

	}
	entry->read_proc = retry_times_proc_read;
	entry->write_proc = retry_times_proc_write;

	entry_debug= create_proc_entry("tc3162/hostadpt_debug_cnt", 0666, NULL);
	if(entry_debug == NULL)
	{
		printk(KERN_WARNING"proc: unable to create hostadpt_debug_cnt proc entry\n");
		return -ENOMEM;

	}
	entry_debug->read_proc = debug_cnt_proc_read;
	entry_debug->write_proc = debug_cnt_proc_write;

	return 0;

}


int rxdone_cb(struct sk_buff *skb)
{
	u32 j = 0, len = skb->len;
	u32 *data = skb->data;
	printk("host adaptor received data:\n");
	for(j = 0; j < len; j++, data++)
	{
		if(j % 20 == 0) printk("\n");
		printk("%d\t", *data);			
	}
	printk("\n");
	//free 
	kfree_skb(skb);
	return 0;
}

struct sk_buff *hostadpt_rx_handler(u32 ring_idx, unsigned char* preschedule )
{	
	HOSTADPT_DMA_DSCP_T *pHostadptRX = NULL, *pHostadptRX_tmp;
	struct sk_buff *skb = NULL;
	struct sk_buff *skb_tmp = NULL;
	struct sk_buff *skb_all = NULL;
	struct sk_buff *skb_new = NULL;
	struct device *dev = NULL ;
	unsigned char isLastPkt = 1, pkt_idx=0, pkt_cnt=0, allPkt_done=1, scatter_cnt = 0;
	u32 cpu_idx_tmp = 0, try_times=0 ,pkt_len;
	static unsigned short cur_pktSize = 0;
	HOSTADPT_MSG(DBG_LOG, "RX%d enter hostadpt_rx_handler...\n", ring_idx);
	
	unsigned char checkfail = 0;
	unsigned int lastfraglen = 0;
	u32 	pkt_addr = 0;
	u32 	scatter_pkt_len = 0;

	if(ring_idx >= HOSTADPT_RX_RING_NUM)
	{
		HOSTADPT_MSG(DBG_ERR, "ring idx error!\n");
		return NULL;
	}
	
	pHostadptRX =pHostadptRX_BASE[ring_idx] + hostadpt_rx_cpu_idx[ring_idx];
	
	if(pHostadptRX->pkt_ctrl.done == 0) 
	{
		HOSTADPT_MSG(DBG_LOG, "no data to receive!\n");
		return NULL;
	}
	HOSTADPT_MSG(DBG_LOG, "old addr: %x\n",pHostadptRX->pkt_addr);
	HOSTADPT_MSG(DBG_LOG, "pkt len: %d\n",pHostadptRX->pkt_ctrl.len);
	HOSTADPT_MSG(DBG_LOG, "recv done: %d\n",pHostadptRX->pkt_ctrl.done);
	HOSTADPT_MSG(DBG_LOG, "foe_number: %d\n",pHostadptRX->pkt_info.foe_number);
	HOSTADPT_MSG(DBG_LOG, "crsn: %d\n",pHostadptRX->pkt_info.crsn);

	if(1)
	{
		if(pHostadptRX->pkt_ctrl.len != pHostadptRX->pkt_ctrl.cur_len)
		{
			cpu_idx_tmp = hostadpt_rx_cpu_idx[ring_idx];
			pkt_cnt = (pHostadptRX->pkt_info.is_last >> 4) & 0x7;
			scatter_pkt_len = pHostadptRX->pkt_ctrl.len;
			while(1)
			{
				pHostadptRX_tmp =pHostadptRX_BASE[ring_idx] + cpu_idx_tmp;
				pkt_idx = (pHostadptRX_tmp->pkt_info.is_last) & 0xf;
				if(pHostadptRX_tmp->pkt_ctrl.done == 0)
				{
					try_times++;
				}
				else
				{
					try_times = 0;
					scatter_cnt = scatter_cnt + 1;
					if(pHostadptRX_tmp->pkt_ctrl.len == pHostadptRX_tmp->pkt_ctrl.cur_len)
					{
						scatter_cnt = scatter_cnt - 1;
						if(scatter_cnt != pkt_cnt)
						{
							checkfail = 1;
							scatter_case1++;
							goto error;
						}
					}
					else
					{
						if(scatter_cnt != pkt_idx || scatter_pkt_len !=pHostadptRX_tmp->pkt_ctrl.len)
						{
							checkfail = 1;
							scatter_case2++;
							goto error;
						}

						if((pHostadptRX_tmp->pkt_ctrl.lastflag & 0x1) == 0x1)
					{
						allPkt_done = 1;
							if(scatter_cnt != pkt_cnt || scatter_pkt_len !=pHostadptRX_tmp->pkt_ctrl.len)
							{
								checkfail = 1;
								scatter_case3++;
								goto error;
						}
						break;
					}
						
					}
						
						
					cpu_idx_tmp++;
					if(cpu_idx_tmp >=  hostadpt_rx_dscp_max_num[ring_idx])
						cpu_idx_tmp = 0;
						

				}
error:				
				if((try_times > reTryTimes) || checkfail/*|| (scatter_cnt > pkt_cnt)*/)
				{
					//printk("[Hostadpt] get all scatter packets fail.\n");
					scatter_case4++;
					allPkt_done = 0;
					try_times = 0;
					//scatter_cnt = 0;
					break;
				}
				
			}
			while(scatter_cnt > 0)
			{
				lastfraglen = 0;
				pHostadptRX =pHostadptRX_BASE[ring_idx] + hostadpt_rx_cpu_idx[ring_idx];
				if(allPkt_done)
				{
					pkt_idx = (pHostadptRX->pkt_info.is_last) & 0xf;
					pkt_cnt = (pHostadptRX->pkt_info.is_last >> 4) & 0x7;
					skb_tmp = (struct sk_buff*)pHostadptRX->skb_p;
					if((pHostadptRX->pkt_ctrl.lastflag & 0x1) == 0x0)
					{
						if(pkt_idx == 1)
						{
							skb_all = dev_alloc_skb(pHostadptRX->pkt_ctrl.len);
						}
						if(skb_all)
						{
							memcpy(skb_all->data+cur_pktSize, skb_tmp->data, pHostadptRX->pkt_ctrl.cur_len);
							skb_put(skb_all, pHostadptRX->pkt_ctrl.cur_len);
						}
						else
							printk("[Hostadpt] Alloc skb for scatter packet(%d/%d/%d), but skb is NULL\n", pkt_idx, pkt_cnt,scatter_cnt);
						//isLastPkt = 0;
						cur_pktSize = cur_pktSize+ pHostadptRX->pkt_ctrl.cur_len;
		
					}
					else if((pHostadptRX->pkt_ctrl.lastflag & 0x1) == 0x1)
					{
						if(skb_all)
						{
							if(cur_pktSize+pHostadptRX->pkt_ctrl.cur_len <= pHostadptRX->pkt_ctrl.len)
								lastfraglen = pHostadptRX->pkt_ctrl.cur_len;
							else{
								scatter_case5++;
								lastfraglen = pHostadptRX->pkt_ctrl.len - cur_pktSize;

							}
								
							memcpy(skb_all->data+cur_pktSize, skb_tmp->data, lastfraglen);
							skb_put(skb_all,lastfraglen);
							skb = skb_all;
						}
						else
						{
							printk("[Hostadpt] Alloc skb for bigger packet(%d/%d/%d), but skb is NULL.\n", pkt_idx, pkt_cnt,scatter_cnt);
							skb = NULL;
						}
						cur_pktSize = 0;	
						//isLastPkt = 1;
					}
					
				

				}
				//else
				//{
					
				
				//}
				pHostadptRX->pkt_ctrl.done = 0;	
				pHostadptRX->pkt_info.is_last = 0;
				pHostadptRX->pkt_ctrl.lastflag = 0;
				scatter_cnt--;
				hostadpt_rx_cpu_idx[ring_idx]++;
				if(hostadpt_rx_cpu_idx[ring_idx] >= hostadpt_rx_dscp_max_num[ring_idx])
					hostadpt_rx_cpu_idx[ring_idx] = 0;
				
				
			}
		
		}
		else
		{
			skb = pHostadptRX->skb_p;
			//skb_put(skb,pHostadptRX->pkt_ctrl.len);
			skb_new = skbmgr_dev_alloc_skb4k();
			//skb_new = dev_alloc_skb(HOSTAPD_BUFFER_LEN);

			if(skb_new == NULL)
			{
				HOSTADPT_MSG(DBG_ERR, "alloc new skb failed!\n");
				skb_new = skb;
				skb = NULL;
				pHostadptRX->pkt_ctrl.done = 0;
				pHostadptRX->pkt_info.is_last = 0;
				pHostadptRX->pkt_ctrl.lastflag = 0;
			}
			else{


			if ((dev=get_gdmpSram_dev())==NULL) 
			{
				HOSTADPT_MSG(DBG_ERR, "\nget dev failed\n");
					dev_kfree_skb_any(skb_new);
					return NULL;
			}
				pkt_addr = dma_map_single(dev, (void *)((unsigned long)skb_new->data), HOSTAPD_BUFFER_LEN, DMA_FROM_DEVICE);
				if(dma_mapping_error(dev, pkt_addr)){
					HOSTADPT_MSG(DBG_ERR, "!!!! Hostadpt dma_mapping_error!!!!\n");
					dev_kfree_skb_any(skb_new);
					return NULL;
				
				 }else{
		
				 // unmap old skb->data
					 dma_unmap_single(dev, pHostadptRX->pkt_addr, HOSTAPD_BUFFER_LEN, DMA_FROM_DEVICE);
					 pHostadptRX->pkt_addr = pkt_addr;
			HOSTADPT_MSG(DBG_LOG, "new addr: %x\n",pHostadptRX->pkt_addr);
			pHostadptRX->pkt_ctrl.done = 0;	
					 pHostadptRX->pkt_info.is_last = 0;
					 pHostadptRX->pkt_ctrl.lastflag = 0;
					 pHostadptRX->skb_p = skb_new;
	
				 }

			}

			if (skb != NULL)
			{
				//sku_put should be here. avoiding multi put happens if skb_put action before "return NULL"
				skb_put(skb, pHostadptRX->pkt_ctrl.len);
			} else {
				HOSTADPT_MSG(DBG_OFF, "\nskb happens\n");
			}

			wmb();
		
	
			hostadpt_rx_cpu_idx[ring_idx]++;
			if(hostadpt_rx_cpu_idx[ring_idx] >= hostadpt_rx_dscp_max_num[ring_idx])
				hostadpt_rx_cpu_idx[ring_idx] = 0;
		}	
	}
	if(ring_idx == 0)
		set_npu_hostadpt_reg(HOSTADPT_RX0_CPU_IDX, hostadpt_rx_cpu_idx[ring_idx]);	
	else if(ring_idx == 1)
		set_npu_hostadpt_reg(HOSTADPT_RX1_CPU_IDX, hostadpt_rx_cpu_idx[ring_idx]);	
	
	
	if(skb != NULL){
		//skb_put(skb, pHostadptRX->pkt_ctrl.len);
		//*preschedule = pHostadptRX->pkt_ctrl.reschedule;
		if (likely(ra_sw_nat_hook_rxinfo))
		{
			pHostadptRX->pkt_info.src_port = 1;
			ra_sw_nat_hook_rxinfo(skb, FOE_MAGIC_GE, (char*)&pHostadptRX->pkt_info, 4);
		}			
	}else{
		*preschedule = 0;
	}
	return skb;
}


void (*tasklet_schedule_WIFI2G)(void);
void (*tasklet_schedule_WIFI5G)(void);
static irqreturn_t rxdone0_isr(int irq, void *dev_id)
{
	unsigned long int status, tmp;
	//HOSTADPT_MSG(DBG_LOG, "rxdone0 interrput......\n"); 
	status = get_npu_hostadpt_reg(0x0030);
	//printk("host apd 0 interrput status: %x\n",status);
	set_npu_hostadpt_reg(0x0030,0x00010000);
	//status = get_npu_hostadpt_reg(0x0030);
	//printk("host apd 0 interrput status: %x\n",status);
	if(tasklet_schedule_WIFI2G){
		//printk("host apd 0 tasklet_schedule_WIFI2G: %x\n",tasklet_schedule_WIFI2G);
		tasklet_schedule_WIFI2G();
	
		tmp = get_npu_hostadpt_reg(HOSTADPT_TXRXDONE0_INT_MASK);
		tmp &= ~(0x00010000);
		set_npu_hostadpt_reg(HOSTADPT_TXRXDONE0_INT_MASK,tmp);
	}	
	//if(temp_rx_data_done){
	//	printk("Hostadp : temp_rx_data_done %x \n", temp_rx_data_done);
	//	tasklet_hi_schedule(temp_rx_data_done);
	//}	
    return IRQ_HANDLED;
}

static irqreturn_t rxdone1_isr(int irq, void *dev_id)
{
	unsigned long int status, tmp;
	//status = get_npu_hostadpt_reg(0x0030);
	//printk("Hostadp : rxdone1_isr interrput status: %x\n",status);
	set_npu_hostadpt_reg(0x0030,0x00020000);
	//status = get_npu_hostadpt_reg(0x0030);
	//printk("host apd 1 interrput status: %x\n",status);
	if(tasklet_schedule_WIFI5G){
		//printk("Hostadp : rxdone1_isr tasklet_schedule_WIFI5G: %x\n",tasklet_schedule_WIFI5G);
		tasklet_schedule_WIFI5G();

		tmp = get_npu_hostadpt_reg(HOSTADPT_TXRXDONE1_INT_MASK);
		tmp &= ~(0x00020000);
		set_npu_hostadpt_reg(HOSTADPT_TXRXDONE1_INT_MASK,tmp);
	}	
	//if(temp_rx_data_done5G){
	//	printk("Hostadp : temp_rx_data_done5G %x\n", temp_rx_data_done5G);
	//	tasklet_hi_schedule(temp_rx_data_done5G);
	//}	
    return IRQ_HANDLED;
}

void hostadpt_regiter_interrupt(u32 irq_num, irqreturn_t (*int_isr)(int, void*), char *irq_name)
{
	int ret = 0;
	if(irq_num > 5)
	{
		HOSTADPT_MSG(DBG_ERR, "irq num error, valid irq num : 0 - 5 \n");
		return ;
	}
	ret = request_irq(get_npu_irq(irq_num), int_isr, 0, irq_name, NULL);
	if(ret == 0) HOSTADPT_MSG(DBG_LOG, "interrupt register success!\n");
	return ;
}

void hostadpt_free_interrupt(u32 irq_num)
{
	if(irq_num > 5)
	{
		HOSTADPT_MSG(DBG_ERR, "irq num error, valid irq num : 0 - 5 \n");
		return ;
	}
	free_irq(get_npu_irq(irq_num), NULL);
	return ;
}
void hostdapt_enable_interrupt(unsigned int ringIdx)
{
	unsigned long int tmp;

	
	//printk("hostdapt_enable_interrupt %d\n", ringIdx);
	if(ringIdx == 0){
		tmp = get_npu_hostadpt_reg(HOSTADPT_TXRXDONE0_INT_MASK);
		tmp |= (0x00010000);
		set_npu_hostadpt_reg(HOSTADPT_TXRXDONE0_INT_MASK,tmp);
	}else{
		tmp = get_npu_hostadpt_reg(HOSTADPT_TXRXDONE1_INT_MASK);
		tmp |= (0x00020000);
		set_npu_hostadpt_reg(HOSTADPT_TXRXDONE1_INT_MASK,tmp);
	}
}
void hostdapt_disable_interrupt(unsigned int ringIdx)
{
	unsigned long int tmp;

	
	//printk("hostdapt_diable_interrupt %d\n", ringIdx);
	if(ringIdx == 0){
		tmp = get_npu_hostadpt_reg(HOSTADPT_TXRXDONE0_INT_MASK);
		tmp &= ~(0x00010000);
		set_npu_hostadpt_reg(HOSTADPT_TXRXDONE0_INT_MASK,tmp);
	}else{
		tmp = get_npu_hostadpt_reg(HOSTADPT_TXRXDONE1_INT_MASK);
		tmp &= ~(0x00020000);
		set_npu_hostadpt_reg(HOSTADPT_TXRXDONE1_INT_MASK,tmp);
	}
}

void hostdapt_registe_wifitask(unsigned int ringIdx, void *func)
{
	printk("hostdapt_registe_wifitask %d\n", ringIdx);
	if(ringIdx == 0){
		tasklet_schedule_WIFI2G = func;
	}else{
		tasklet_schedule_WIFI5G = func;
	}
}

static int hostadpInit(void)
{
	HOSTADPT_MSG(DBG_LOG, "host adaptor init :\n");
	unsigned long flags;
	reTryTimes = 10000;
        scatter_case1 = 0;
	scatter_case2 = 0;
	scatter_case3 = 0;
	scatter_case4 = 0;
	scatter_case5 = 0;
	if(SKBMGR_4K_RX_BUF_LEN<3565)     /* 3565 means PKTBUFFER_SIZE(3500)+padding(64)+gdma_copy(1) */
	{
		printf("SKBMGR_4K_RX_BUF_LEN is %d,less than PKTBUFFER_SIZE \n",SKBMGR_4K_RX_BUF_LEN);
		printf(" hostadpInit fail !!\n");
		return;
	}
	if(hostadpt_rxdscp_init())
	{
		HOSTADPT_MSG(DBG_ERR, "hostadpt_rxdscp_init failed!\n");
		return -ENODEV;
	}
	
	hostadpt_regiter_interrupt(0, rxdone0_isr, "rxdone0");
	set_npu_hostadpt_reg(HOSTADPT_TXRXDONE0_INT_MASK,0x00010000);

	hostadpt_regiter_interrupt(1, rxdone1_isr, "rxdone1");
	set_npu_hostadpt_reg(HOSTADPT_TXRXDONE1_INT_MASK,0x00020000);


	rcu_assign_pointer(fromHostadptPktHandle_hook, hostadpt_rx_handler);
	rcu_assign_pointer(hostdapt_enable_int_hook, hostdapt_enable_interrupt);
	rcu_assign_pointer(hostdapt_disable_int_hook, hostdapt_disable_interrupt);
	rcu_assign_pointer(hostdapt_registe_wifitask_hook, hostdapt_registe_wifitask);
	return 0;
}

static void __exit hostadpExit(void)
{
	hostadpt_free_interrupt(0);
	hostadpt_free_interrupt(1);
	
	rcu_assign_pointer(fromHostadptPktHandle_hook, NULL);
	rcu_assign_pointer(hostdapt_enable_int_hook, NULL);
	rcu_assign_pointer(hostdapt_disable_int_hook, NULL);
	rcu_assign_pointer(hostdapt_registe_wifitask_hook, NULL);
	HOSTADPT_MSG(DBG_LOG, "host adaptor exit .\n");
}

module_init(hostadpInit);
module_exit(hostadpExit);

MODULE_DESCRIPTION("EcoNet Host Adaptor Driver");
