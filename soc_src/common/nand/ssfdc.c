/********************** BEGIN LICENSE BLOCK ************************************
 *
 * JZ4740  mobile_tv  Project  V1.0.0
 * INGENIC CONFIDENTIAL--NOT FOR DISTRIBUTION IN SOURCE CODE FORM
 * Copyright (c) Ingenic Semiconductor Co. Ltd 2005. All rights reserved.
 * 
 * This file, and the files included with this file, is distributed and made 
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * 
 * http://www.ingenic.cn 
 *
 ********************** END LICENSE BLOCK **************************************
 *
 *  Author:  <dsqiu@ingenic.cn>  <jgao@ingenic.cn> 
 *
 *  Create:   2008-06-26, by dsqiu
 *            
 *  Maintain: 2008-06-26, by jgao
 *            
 *
 *******************************************************************************
 */



#include "header.h"
#include "function.h"
unsigned int ssfdc_nftl_read(unsigned int	nSectorAddr,
							 unsigned char	*pBuffer,
							 unsigned int	nSectorNum)
{
	return 0;
}


//------------------------------------------------------------------------------
// ssfdc_nftl_write:   sector write
//                     
unsigned int ssfdc_nftl_write(	unsigned int	nSectorAddr,
								unsigned char	*pBuffer,
								unsigned int	nSectorNum)
{
	return 0;
}



//------------------------------------------------------------------------------

int ssfdc_nftl_flush_cache(void)
{
	return 0;
}

//------------------------------------------------------------------------------

void ssfdc_nftl_getInfo(unsigned int zone, pflash_info_t pflashinfo)
{
}


int ssfdc_nftl_open(void)
{
	return (0);
}

//------------------------------------------------------------------------------

int ssfdc_nftl_init(void)
{
	return (0);
}

//------------------------------------------------------------------------------

static void ssfdc_deinit(void)
{
}

//------------------------------------------------------------------------------

int ssfdc_nftl_release(void)
{
	return (0);
}

//------------------------------------------------------------------------------

int ssfdc_nftl_format(void)
{
	return (0);
}

//------------------------------------------------------------------------------
