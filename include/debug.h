/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Jz.
 *       Filename:  debug.h
 *
 *    Description:  
 *         Others:
 *
 *        Version:  1.0
 *        Created:  01/05/2017 10:08:04 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy. Hou (hwt), houwentaoff@gmail.com
 *   Organization:  Jz
 *
 * =====================================================================================
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

#define     PRT_DEBUG                             1  /* 1 is open , o is close */
#define     PRT_ERROR                             1  /* 1 is open , 0 is close */

#define     WT_DEBUG                             1  /* 1 is open , o is close */
#define     WT_ERROR                             1  /* 1 is open , 0 is close */

#if PRT_DEBUG
#define prt_dbg(fmt, args...) printf("[prt][\033[1;34mDEBUG\033[0m]: ---> %s():"fmt"\n", __func__, ##args)/*  */
#else
#define prt_dbg(fmt, args...)
#endif

#if PRT_ERROR
#define prt_err(fmt, args...)                                                             \
    do                                                                                          \
{                                                                                           \
        printf("[prt][\033[1;31mERROR\033[0m] ---> %s ():line[%d]:", __func__, __LINE__);      \
        printf(" "fmt"\n", ##args);                                                                 \
}while(0)    /*  */
#else
#define prt_err(fmt, args...)
#endif


#if WT_DEBUG
#define wt_dbg(fmt, args...) printf("[wt][\033[1;34mDEBUG\033[0m]: ---> %s():"fmt"\n", __func__, ##args)/*  */
#else
#define wt_dbg(fmt, args...)
#endif

#if WT_ERROR
#define wt_err(fmt, args...)                                                             \
    do                                                                                          \
{                                                                                           \
        printf("[wt][\033[1;31mERROR\033[0m] ---> %s ():line[%d]:", __func__, __LINE__);      \
        printf(" "fmt"\n", ##args);                                                                 \
}while(0)    /*  */
#else
#define wt_err(fmt, args...)
#endif



#endif

