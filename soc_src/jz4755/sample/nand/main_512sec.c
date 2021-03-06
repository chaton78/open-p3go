/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/
#include <os_api.h>
#include <nand_api.h>

/* File local definitions */
#define  TASK_START_PRIO 40
#define  TASK_STK_SIZE   1024 *128                      /* Size of each task's stacks (# of WORDs) */


#define NAND_512_TEST 	1

OS_TASK_STACK   TaskStartStk[TASK_STK_SIZE];
unsigned char os_malloc_completed = 0;
unsigned char os_init_completed = 0;
/* ********************************************************************* */
/* Local functions */
void __umoddi3()
{;}

void __udivdi3(){;}

#if NAND_512_TEST
#define BLOCK_SIZE	512 * 1024
#define CONFIG_SSFDC_NAND_PAGE_PER_BLOCK 256

void jz_nand_hardware_test_info (void)
{
	nand_page_info_t	*info;
	unsigned short *pucinfo;
	unsigned int page,blockid;
	int k ,i;

	jz_nand_init();
	
	pucinfo = (unsigned short *)alignAlloc(32,256*10);
		
	blockid = 512;
	page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK ;
	//pucinfo = (unsigned short *)(&info);
	
	
	memset(pucinfo, 0x3a3a , 256*10);
	info = (nand_page_info_t *)pucinfo ;
	

	jz_nand_erase_block(0,blockid);
	
	for (i =0  ; i < 1 ;i++)	
	{
		jz_nand_write_page_info(0, page, info);
	
	}
	memset(pucinfo, 0xFFFF , 256*10);
	info = (nand_page_info_t *)pucinfo ;
	jz_nand_read_page_info(0, page, info);
	
	printf("nand read successful!\r\n");
	
	unsigned char *tempbuf = (unsigned char *)pucinfo;
	for(k = 0;k < 64/8; k++)
		printf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++);
	
	while(1);
}
void jz_nand_hardware_test_page (void)
{
	nand_page_info_t	*info;
	unsigned short *pucinfo;
	unsigned int page,blockid;
	unsigned short *data_ptr , *readbuff;
	unsigned int *mybuffer, *mybuffer1;
	//flash_info_t	flashinfo;
	int k,i,ret;
	readbuff = (unsigned short *)alloc(BLOCK_SIZE);
	data_ptr = (unsigned short *)alloc(BLOCK_SIZE);
	mybuffer = (unsigned int *)data_ptr;
	
	mybuffer1 =(unsigned int *)readbuff;
	jz_nand_init();
	//jz_nand_get_info(&flashinfo);
	blockid = 512;
	page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
	
	//memset(pucinfo, 0xff, 64);
	pucinfo = (unsigned short *)alignAlloc(32,10*CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	info  =(nand_page_info_t *)pucinfo ;

	jz_nand_erase_block(0,blockid);
	
//	for(i = 0;i < 1;i++)
	if(1)
	{
		for(k = 0;k < BLOCK_SIZE/4 ;k++)
			mybuffer[k] = k+1;

		memset(pucinfo, 0xa, 256);
		//jz_nand_write_page(0, page ,data_ptr, info);
		//info ++;
		
		for(k = 0;k < BLOCK_SIZE/4;k++)
			mybuffer1[k] = 0x0;
			
		memset(pucinfo, 0xFF, 256);
		info  =(nand_page_info_t *)pucinfo ;
		//ret = jz_nand_read_page(0, page ,readbuff, info);
		
	}

	unsigned short *ptr3=(unsigned short*)info;
	for(k = 0;k < 256/8; k++)
		printf("%x %x %x %x %x %x %x %x\n",*ptr3++,*ptr3++,*ptr3++,*ptr3++,
		*ptr3++,*ptr3++,*ptr3++,*ptr3++);
	
	unsigned short *tempbuf = readbuff;//pucinfo;
	for(k = 0;k < 512/8; k++)
		printf("%x %x %x %x %x %x %x %x\n",*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++);
	while(1);
	
}
void jz_nand_hardware_test_multi (void)
{
	nand_page_info_t	*info;
	unsigned char *pucinfo;
	unsigned int page,blockid;
	unsigned char *data_ptr;
	unsigned int *mybuffer;
	unsigned char	*mask;
	int k;
	jz_nand_init();
	data_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	mask = (unsigned char *)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	pucinfo = (unsigned char	*)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK * 128);
	mybuffer = (unsigned int *)data_ptr;
	
	info = (nand_page_info_t *)pucinfo;
	blockid = 512;
	page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
	
	memset(pucinfo, 0x0, 128 * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	
	for(k = 0;k < BLOCK_SIZE/4;k++)
		mybuffer[k] = k+1;
		
	jz_nand_erase_block(0,blockid);
	
	memset(mask, 0xF, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	jz_nand_multiwrite(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, data_ptr, info);
	
	
	for(k = 0;k < BLOCK_SIZE/4;k++)
		mybuffer[k] = 0xFF;
	memset(pucinfo, 0xFF, 128 * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	memset(mask, 0x0, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	jz_nand_multiread(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, data_ptr, info);
	//jz_nand_read_page(0, page,data_ptr, info);
	//jz_nand_read_page_info(0, page, info);
	
	printf("nand read successful!\r\n");
	
	unsigned char *tempbuf = pucinfo;//pucinfo;
	for(k = 0;k < 64/8; k++)
		printf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++);
	while(1);
}
void jz_nand_hardware_test(void)
{
	nand_page_info_t	*info;
	unsigned char *pucinfo;
	unsigned int page,blockid;
	unsigned char *data_ptr;
	unsigned char *dataread_ptr;
	unsigned char *puctemp;
	unsigned char *puctemp1;
	unsigned int *mybuffer;
	unsigned int *mybufferread;
	unsigned char	*mask;
	int k,i;
	flash_info_t	flashinfo;
	
	jz_nand_init();
	jz_nand_get_info(&flashinfo);
	data_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	dataread_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	mask = (unsigned char *)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	pucinfo = (unsigned char *)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK * 64);
	mybuffer = (unsigned int *)data_ptr;
	mybufferread = (unsigned int *)dataread_ptr;
	info = (nand_page_info_t *)pucinfo;
	blockid = 512;
	#if 0
	for (k = 0;k < 256; k++)
	{
		memset(info, 0xFF, 64);
		if (k == 0)
		{
			info->lifetime = 0xABCDABCD;
			info->block_addr_field = 0x100;
		}
		info->page_status = 0x0;
		info ++;
	}
	info = (nand_page_info_t *)pucinfo;
	//memset(pucinfo, 0x0, 64 * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	
	for(k = 0;k < BLOCK_SIZE/4;k++)
	{
		mybuffer[k] = k+1;
		mybufferread[k] = 0;
	}
	
	//blockid = (flashinfo.dwFSTotalBlocks / 2 + 5000);
	for(i = 0;i < (flashinfo.dwFSTotalBlocks / 2 + 5000);i++)//flashinfo.dwFSTotalBlocks / 2
	{	
		puctemp = data_ptr;
		puctemp1 = dataread_ptr;
		page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
		jz_nand_erase_block(0,blockid);
		memset(mask, 0xf, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
		jz_nand_multiwrite(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, data_ptr, info);
		//jz_nand_write_page(0, page,data_ptr, info);	
		memset(mask, 0x0, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
		jz_nand_multiread(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, dataread_ptr, info);
		//jz_nand_read_page(0, page,dataread_ptr, info);
		//jz_nand_read_page_info(0, page, info);
			
		for(k = 0;k < BLOCK_SIZE; k++)
		{
			if (*puctemp++ != *puctemp1++ )
			{
				printf("read sector data error, k = %d,data_ptr[0x%x],dataread_ptr[0x%x]\r\n",k,*(--puctemp),*(--puctemp1));
				
				//while (1);
			}
		}
		
		printf("\r\nTest OK!xyzhang\r\n");
		blockid++;
	}
	#endif
	#if 0
	for (k = 0;k < 256; k++)
	{
		memset(info, 0xFF, 64);
		if (k == 0)
		{
			info->lifetime = 0xABCDABCD;
			info->block_addr_field = 0x100;
		}
		info->page_status = 0x0;
		info ++;
	}
	info = (nand_page_info_t *)pucinfo;
	for(k = 0;k < BLOCK_SIZE/4;k++)
	{
		mybuffer[k] = k+1;
		mybufferread[k] = 0;
	}
	for(i = 0;i < (flashinfo.dwFSTotalBlocks / 2);i++)
	{	
		page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
		jz_nand_erase_block(0,blockid);
		memset(mask, 0xf, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
		jz_nand_multiwrite(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, data_ptr, info);
		blockid++;
	}
	#else
	for(k = 0;k < BLOCK_SIZE/4;k++)
	{
		mybuffer[k] = k+1;
		mybufferread[k] = 0;
	}
	blockid = 512;

	for(i = 0;i < (flashinfo.dwFSTotalBlocks / 2);i++)
	//while(1)
	{	
		puctemp = data_ptr;
		puctemp1 = dataread_ptr;
		page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
		memset(mask, 0x0, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
		jz_nand_multiread(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, dataread_ptr, info);
					
		for(k = 0;k < BLOCK_SIZE; k++)
		{
			if (*puctemp++ != *puctemp1++ )
			{
				printf("read sector data error, k = %d,data_ptr[0x%x],dataread_ptr[0x%x]\r\n",k,*(--puctemp),*(--puctemp1));
				break;
			}
		}
		
		printf("\r\nTest OK!xyzhang\r\n");
		blockid++;
	}
	#endif
	printf("\r\nTest OK!xyzhang\r\n");
	while (1);
}
void jz_nand_hardware_test_mask (void)
{
	nand_page_info_t	*info;
	unsigned char *pucinfo;
	unsigned int page,blockid;
	unsigned char *data_ptr;
	unsigned char *dataread_ptr;
	unsigned int *mybuffer;
	unsigned int *mybufferread;
	unsigned char	*mask;
	int k,i;
	jz_nand_init();
	data_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	dataread_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	mask = (unsigned char *)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	pucinfo = (unsigned char *)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK * 64);
	mybuffer = (unsigned int *)data_ptr;
	mybufferread = (unsigned int *)dataread_ptr;
	info = (nand_page_info_t *)pucinfo;
	blockid = 512;
	page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
	
	for (k = 0;k < CONFIG_SSFDC_NAND_PAGE_PER_BLOCK; k++)
	{
		memset(info, 0xFF, 64);
		if (k == 0)
		{
			info->lifetime = 0xABCDABCD;
			info->block_addr_field = 0x100;
		}
		info->page_status = 0x0;
		info ++;
	}
	info = (nand_page_info_t *)pucinfo;
	//memset(pucinfo, 0x0, 64 * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	
	for(k = 0;k < BLOCK_SIZE/4;k++)
	{
		mybuffer[k] = k+1;
		mybufferread[k] = k+1;
	}	
	jz_nand_erase_block(0,blockid);
	
	memset(mask, 0xF, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	jz_nand_multiwrite(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, data_ptr, info);
	
	memset(mask, 0x0, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	
	jz_nand_multiread(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, dataread_ptr, info);

	//for (i = 0; i < CONFIG_SSFDC_NAND_PAGE_PER_BLOCK * 4;i++)
	//{
		for(k = 0;k < BLOCK_SIZE; k++)//BLOCK_SIZE
		{
			if (*data_ptr++ != *dataread_ptr++ )
			{
				printf("read sector data error, k = %d,data_ptr[0x%x],dataread_ptr[0x%x]\r\n",k,*(--data_ptr),*(--dataread_ptr));
				printf("i = %d\n",i);
				while (1);
			}
		}
		//data_ptr += 512;
		//dataread_ptr += 512;
	//}
		
	printf("\r\nTest OK!xyzhang\r\n");
	
	while (1);
}

#define UNCACHE(x) ((unsigned int)x | 0xa0000000)
unsigned int data_buf[1 * 1024 * 1024 / 4] __attribute__((aligned(32)));
void jz_nand_hardware_test_speed (void)
{
	nand_page_info_t	*info;
	unsigned char *pucinfo;
	unsigned int page,blockid;
	unsigned char *data_ptr;
	unsigned char *dataread_ptr;
	unsigned int *mybuffer;
	unsigned int *mybufferread;
	unsigned char	*mask;
	unsigned int 	k,i;
	unsigned long	ulTimeStart = 0,ulTimeEnd = 0,ulTime,ulSpeed;
	jz_nand_init();

	//data_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	data_ptr = (unsigned char *)UNCACHE(data_buf);
	dataread_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	mask = (unsigned char *)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	pucinfo = (unsigned char *)alloc(CONFIG_SSFDC_NAND_PAGE_PER_BLOCK * 64);
	mybuffer = (unsigned int *)data_ptr;
	mybufferread = (unsigned int *)dataread_ptr;
	
	info = (nand_page_info_t *)pucinfo;
	blockid = 512;
	page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
	jz_nand_speed_mode(1);
	memset(pucinfo, 0x0, 64 * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	
	for(k = 0;k < BLOCK_SIZE/4;k++)
		mybuffer[k] = k;
	
	#if 1
	for(k = 0;k< 10; k++)
	{	
		jz_nand_erase_block(0,(blockid + k));
	}
	
	memset(mask, 0xFF, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	
	//Init timer
	Init_PerformanceCounter();
	ulTimeEnd = Get_PerformanceCounter();
	ulTimeStart = ulTimeEnd;
	for(k = 0;k< 10; k++)
	{
		jz_nand_multiwrite(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, data_ptr, info);
		page += CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
	}
	ulTimeEnd = Get_PerformanceCounter();
	ulTime = (ulTimeEnd - ulTimeStart) / 6000;
	ulSpeed = (BLOCK_SIZE * k) / ulTime;// KB/S
	printf("Write nand speed = %dKB/S,ulTime = %dms\n", ulSpeed,ulTime);
	#endif
	
	blockid = 512;
	page = blockid * CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
	//Init timer
	Init_PerformanceCounter();
	memset(mask, 0x0, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	ulTimeEnd = Get_PerformanceCounter();
	ulTimeStart = ulTimeEnd;
	for(k = 0;k< 10; k++)
	{
		jz_nand_multiread(0, page, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK, mask, dataread_ptr, info);
		page += CONFIG_SSFDC_NAND_PAGE_PER_BLOCK;
		memset(mask, 0x0, CONFIG_SSFDC_NAND_PAGE_PER_BLOCK);
	}
	ulTimeEnd = Get_PerformanceCounter();
	ulTime = (ulTimeEnd - ulTimeStart) / 6000;
	ulSpeed = (BLOCK_SIZE * k) / ulTime;// KB/S
	printf("Read nand speed = %dKB/S,ulTime = %dms\n", ulSpeed,ulTime);
	/*
	unsigned char *tempbuf = dataread_ptr;//pucinfo;
	for(k = 0;k < 64/8; k++)
		printf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++,*tempbuf++);
	while(1);
	k--;*/
	
	for(k = 0;k < BLOCK_SIZE / 4; k++)
	{
		if (k != mybufferread[k] )//*data_ptr++ != *dataread_ptr++ )
		{
			printf("read sector data error, k = %d,data_ptr[0x%x],dataread_ptr[0x%x]\r\n",k,*(--data_ptr),*(--dataread_ptr));
			while (1);
		}
	}
	
	printf("\r\nTest OK!xyzhang\r\n");

	while (1);
}

void jz_nand_ssfdc_test(void)
{
	unsigned int *mybuffer;	
	unsigned char *data_ptr;
	unsigned int i,j,Sector;
	 
	data_ptr = (unsigned char *)alloc(BLOCK_SIZE);
	mybuffer = (unsigned int *)data_ptr;
	
	ssfdc_nftl_init();
		
	ssfdc_nftl_open(0);
	//ssfdc_nftl_format();
	
	printf("Nand data test\r\n");
	
	for(i = 0;i < BLOCK_SIZE / 4;i++)
		mybuffer[i] = i;
	
	for(i = 0; i < 1; i++)
	{
		Sector = 0 + i * 512;
		ssfdc_nftl_write(Sector,data_ptr,512);
	}
	ssfdc_nftl_flush_cache();

	printf("nand read data\r\n");
	for(i = 0; i < 1; i++) 
	{
		for(j = 0;j < BLOCK_SIZE / 4;j++)
			mybuffer[j] = -1;
		Sector = 0 + i * 512;
		ssfdc_nftl_read(Sector,data_ptr,512);
				
		for(j = 0;j < BLOCK_SIZE / 4;j++)
		{
			if(mybuffer[j] != j)
			{
				printf("read sector data error %d sector %d word->0x%x\r\n",i,j,mybuffer[j]);
				deAlloc(mybuffer);
				while(1);
			}
		}
	}
	
	printf("Test OK!xyzhang\r\n");
	deAlloc(mybuffer);
	while(1);
}

void NandTest()
{
	printf("Nand data test\r\n");
	//jz_nand_ssfdc_test(); 
	//jz_nand_hardware_test();
	jz_nand_hardware_test_speed();
	//jz_nand_hardware_test_mask();
	//jz_nand_hardware_test_page();
	//jz_nand_hardware_test_info(); 
	//jz_nand_hardware_test_multi();
}
#else
void NandTest()
{
	printf("Nand data test\r\n");
	while(1);
}
#endif


void TaskStart (void *data)
{
	unsigned char i;
	char key;
	data = data;                            /* Prevent compiler warning */
	
	JZ_StartTicker(OS_TICKS_PER_SEC);	/* os_cfg.h */
	printf("uC/OS-II, The Real-Time Kernel MIPS Ported version\n");
	printf("EXAMPLE #1 %s %s\n",__DATE__,__TIME__);
	while(0)
	{
		if(serial_tstc())
		{
			key = serial_getc();
			printf("you pressed: %c\n",key);
			if(key == 0x1B) 
			{        /* see if it's the ESCAPE key */
				printf(" Escape display of statistic\n");
				os_TaskDelete(TASK_START_PRIO);
			}
		}
		printf("time: %d\n",os_TimeGet());
		os_TimeDelayHMSM(0, 0, 1, 0);     /* Wait one second */
	}
	NandTest();
}


/* ********************************************************************* */
/* Global functions */

void APP_vMain (void)
{
	heapInit();
	os_malloc_completed = 1;
	os_init_completed = 1;
	printf("MINIOS:Start test----\n");	
	os_TaskCreate(TaskStart, 
				(void *)0, 
				(void *)&TaskStartStk[TASK_STK_SIZE - 1], 
				TASK_START_PRIO);
	
	os_SystemStart();                              /* Start multitasking */
	
	while(1);
}
/* ********************************************************************* */
