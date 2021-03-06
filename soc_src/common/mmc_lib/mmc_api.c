/*
**********************************************************************
*
*                            uC/MMC
*
*             (c) Copyright 2005 - 2007, Ingenic Semiconductor, Inc
*                      All rights reserved.
*
***********************************************************************

----------------------------------------------------------------------
File        : mmc_api.c
Purpose     : Global functions to be used by an application
              using the mmc/sd card.

----------------------------------------------------------------------
Version-Date-----Author-Explanation
----------------------------------------------------------------------
1.00.00 20060831 WeiJianli     First release

----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
(none)
----------------------------------------------------------------------
*/

/* the information structure of MMC/SD Card */
mmc_info mmcinfo;
extern int sd2_0;
int limit_41 = 0;
extern int num_6;

#if MMC_MINIOS_EN
OS_EVENT *pSemMMC = NULL;	/* semaphore that access card */
#endif

/*******************************************************************************************************************
** Name:	  void mmc_enter()
** Function:      request the right of operating mmc to OS
** Input:	  NULL
** return:	  NULL
********************************************************************************************************************/
static void mmc_enter(void)
{
#if MMC_MINIOS_EN
	unsigned char ret;
	os_SemaphorePend(pSemMMC, 0, &ret);	/* wait for semaphore that accessed Card */
#endif
}

/*******************************************************************************************************************
** Name:	  void mmc_exit()
** Function:      return the right of operating mmc to OS
** Input:	  NULL
** return:	  NULL
********************************************************************************************************************/
static void mmc_exit(void)
{
#if MMC_MINIOS_EN
	os_SemaphorePost(pSemMMC);	/* return the semaphore accessing Card to OS */
#endif
}

/*******************************************************************************************************************
** Name:	  int MMC_Initialize(unsigned int flag)
** Function:      initialize MMC/MMC card
** Input:	  NULL
** Output:	  0:  right			>0:  error code
********************************************************************************************************************/
int MMC_Initialize(unsigned int flag)
{
	int retval;

	num_6 = 0;
	limit_41 = 0;
	sd2_0 = 0;

#if MMC_MINIOS_EN
	if (pSemMMC == NULL) {
		pSemMMC = os_SemaphoreCreate(1);	/* create MMC/SD semaphore */
		if (pSemMMC == NULL)
			return MMC_NO_RESPONSE;
	}
#endif

	mmc_enter();		/* Claim the lock */

	jz_mmc_hardware_init();	/* Initialize the hardware that access SD Card */

	if (!mmc_check_status()) {	/* Check weather card is inserted */
		mmc_exit();
		return MMC_NO_RESPONSE;
	}

	if (mmc_init_card()) {	/* Initialize and detect SD card */
		mmc_exit();
		return MMC_NO_RESPONSE;
	}

	mmc_select_card(flag);	/* Select the SD card */

	mmc_exit();		/* Release the lock */

	return retval;
}
unsigned int MMC_GetSize(void)
{
	#if 0
	unsigned int size;
	size =
	    (mmcinfo.csd.c_size +
	     1) * (1 << (mmcinfo.csd.c_size_mult +
			 2)) * (1 << mmcinfo.csd.read_bl_len);
	return size;
	#endif
	return mmcinfo.block_num;

}
int MMC_GetInfo(PFS_MMC_info pFSMMCInfo)
{
	pFSMMCInfo->dwBlockSize = mmcinfo.block_len;
	pFSMMCInfo->dwSectorsPerBlock = mmcinfo.block_len / MMC_BLOCKSIZE;
	pFSMMCInfo->dwSectorSize = MMC_BLOCKSIZE;
	pFSMMCInfo->dwFSTotalBlocks = mmcinfo.block_num;
	pFSMMCInfo->dwFSTotalSectors = (mmcinfo.block_num * mmcinfo.block_len) / MMC_BLOCKSIZE;

	return 0;
}

/******************************************************************
 *
 * Read a single block from SD/MMC card
 *
 ******************************************************************/
int MMC_ReadBlock(unsigned int blockaddr, unsigned char * recbuf)
{
	struct mmc_request request;
	struct mmc_response_r1 r1;
	int retval;
	mmc_enter();		/* request semaphore acessed SD/MMC to OS */
	//printf("MMC_ReadBlock ++\n");
	if (!mmc_check_status())
	{
		mmc_exit();
		printf("mmc Check card failure\n");
		return MMC_NO_RESPONSE;	/* card is not inserted entirely */
	}

	if (blockaddr > mmcinfo.block_num) {
		mmc_exit();
		return MMC_ERROR_OUT_OF_RANGE;	/* operate over the card range */
	}

	mmc_simple_cmd(&request, MMC_SEND_STATUS, mmcinfo.rca,
		       RESPONSE_R1);
	retval = mmc_unpack_r1(&request, &r1, 0);
	if (retval && (retval != MMC_ERROR_STATE_MISMATCH)) {
		mmc_exit();
		return retval;
	}

	mmc_simple_cmd(&request, MMC_SET_BLOCKLEN, MMC_BLOCKSIZE,
		       RESPONSE_R1);
	if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
		mmc_exit();
		return retval;
	}
	if (sd2_0) {
		mmc_send_cmd(&request, MMC_READ_SINGLE_BLOCK, blockaddr, 1,
			     MMC_BLOCKSIZE, RESPONSE_R1, recbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}
	} else {
		mmc_send_cmd(&request, MMC_READ_SINGLE_BLOCK,
			     blockaddr * MMC_BLOCKSIZE, 1, MMC_BLOCKSIZE,
			     RESPONSE_R1, recbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}
	}
	mmc_exit();		/* return semaphore acessed MMC/SD to OS */
	//printf("MMC_ReadBlock --\n");
	return retval;
}

/******************************************************************
 *
 * Read multiple blocks from MMC/MMC card
 *
 ******************************************************************/
int MMC_ReadMultiBlock(unsigned int blockaddr, unsigned int blocknum, unsigned char * recbuf)
{
	struct mmc_request request;
	struct mmc_response_r1 r1;
	int retval;
	//printf("MMC_ReadMultiBlock ++\n");
	mmc_enter();		/* request semaphore acessed SD/MMC to OS */
	if (!mmc_check_status()) {
		mmc_exit();
		return MMC_NO_RESPONSE;	/* card is not inserted entirely */
	}

	if ((blockaddr + blocknum) > mmcinfo.block_num) {
		mmc_exit();
		return MMC_ERROR_OUT_OF_RANGE;	/* operate over the card range */
	}

	mmc_simple_cmd(&request, MMC_SEND_STATUS, mmcinfo.rca,
		       RESPONSE_R1);
	retval = mmc_unpack_r1(&request, &r1, 0);
	if (retval && (retval != MMC_ERROR_STATE_MISMATCH)) {
		mmc_exit();
		return retval;
	}

	mmc_simple_cmd(&request, MMC_SET_BLOCKLEN, MMC_BLOCKSIZE,
		       RESPONSE_R1);
	if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
		mmc_exit();
		return retval;
	}
	if (sd2_0) {
		mmc_send_cmd(&request, MMC_READ_MULTIPLE_BLOCK, blockaddr,
			     blocknum, MMC_BLOCKSIZE, RESPONSE_R1, recbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}
	} else {
		mmc_send_cmd(&request, MMC_READ_MULTIPLE_BLOCK,
			     blockaddr * MMC_BLOCKSIZE, blocknum,
			     MMC_BLOCKSIZE, RESPONSE_R1, recbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}

	}

	mmc_simple_cmd(&request, MMC_STOP_TRANSMISSION, 0, RESPONSE_R1B);
	if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
		mmc_exit();
		return retval;
	}

	mmc_exit();
	//printf("MMC_ReadMultiBlock --\n");
	return retval;
}

/******************************************************************
 *
 * Write a block to SD/MMC card
 *
 ******************************************************************/
int MMC_WriteBlock(unsigned int blockaddr, unsigned char * sendbuf)
{
	struct mmc_request request;
	struct mmc_response_r1 r1;
	int retval;
	//printf("MMC_WriteBlock ++\n");
	mmc_enter();		/* request semaphore acessed MMC/SD to OS */
	if (!mmc_check_status()) {
		mmc_exit();
		return MMC_NO_RESPONSE;	/* card is not inserted entirely */
	}

	if (blockaddr > mmcinfo.block_num) {
		mmc_exit();
		return MMC_ERROR_OUT_OF_RANGE;	/* operate over the card range */
	}

	if (mmc_write_protect()) {
		mmc_exit();
		return MMC_ERROR_WP_VIOLATION;	/* card is write protected */
	}

	mmc_simple_cmd(&request, MMC_SEND_STATUS, mmcinfo.rca,
		       RESPONSE_R1);
	retval = mmc_unpack_r1(&request, &r1, 0);
	if (retval && (retval != MMC_ERROR_STATE_MISMATCH)) {
		mmc_exit();
		return retval;
	}

	mmc_simple_cmd(&request, MMC_SET_BLOCKLEN, MMC_BLOCKSIZE,
		       RESPONSE_R1);
	if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
		mmc_exit();
		return retval;
	}
	if (sd2_0) {
		mmc_send_cmd(&request, MMC_WRITE_BLOCK, blockaddr, 1,
			     MMC_BLOCKSIZE, RESPONSE_R1, sendbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}
	} else {
		mmc_send_cmd(&request, MMC_WRITE_BLOCK,
			     blockaddr * MMC_BLOCKSIZE, 1, MMC_BLOCKSIZE,
			     RESPONSE_R1, sendbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}

	}

	mmc_exit();
	//printf("MMC_WriteBlock --\n");
	return retval;
}

/******************************************************************
 *
 * Write multiple blocks to SD/MMC card
 *
 ******************************************************************/
int MMC_WriteMultiBlock(unsigned int blockaddr, unsigned int blocknum, unsigned char * sendbuf)
{
	struct mmc_request request;
	struct mmc_response_r1 r1;
	int retval;

	mmc_enter();		/* request semaphore acessed SD/MMC to OS */
	if (!mmc_check_status()) {
		mmc_exit();
		return MMC_NO_RESPONSE;	/* card is not inserted entirely */
	}

	if (blockaddr > mmcinfo.block_num) {
		mmc_exit();
		return MMC_ERROR_OUT_OF_RANGE;	/* operate over the card range */
	}

	if (mmc_write_protect()) {
		mmc_exit();
		return MMC_ERROR_WP_VIOLATION;	/* card is write protected */
	}

	mmc_simple_cmd(&request, MMC_SEND_STATUS, mmcinfo.rca,
		       RESPONSE_R1);
	retval = mmc_unpack_r1(&request, &r1, 0);
	if (retval && (retval != MMC_ERROR_STATE_MISMATCH)) {
		mmc_exit();
		return retval;
	}

	mmc_simple_cmd(&request, MMC_SET_BLOCKLEN, MMC_BLOCKSIZE,
		       RESPONSE_R1);
	if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
		mmc_exit();
		return retval;
	}

	if (sd2_0) {
		mmc_send_cmd(&request, MMC_WRITE_MULTIPLE_BLOCK, blockaddr,
			     blocknum, MMC_BLOCKSIZE, RESPONSE_R1,
			     sendbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}
	} else {
		mmc_send_cmd(&request, MMC_WRITE_MULTIPLE_BLOCK,
			     blockaddr * MMC_BLOCKSIZE, blocknum,
			     MMC_BLOCKSIZE, RESPONSE_R1, sendbuf);
		if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
			mmc_exit();
			return retval;
		}

	}
	mmc_simple_cmd(&request, MMC_STOP_TRANSMISSION, 0, RESPONSE_R1B);
	if ((retval = mmc_unpack_r1(&request, &r1, 0))) {
		mmc_exit();
		return retval;
	}
	mmc_exit();
	return retval;
}


#if 0
/******************************************************************
 *
 * Detect SD/MMC card
 *
 ******************************************************************/
int MMC_DetectStatus(void)
{
	if (jz_mmc_chkcard())
		return 0;	/* card exits */

	return 1;		/* card does not exit */
}

#endif
