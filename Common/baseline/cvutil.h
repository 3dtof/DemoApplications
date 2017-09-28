/*! 
 * ===================================================================
 *
 *  @addtogroup	   	baseline	
 *  @{
 *
 *  @file 	util.h
 *
 *  @brief	utlity routines
 *
 *====================================================================
 */
#include <stdio.h>
#include <unistd.h>
#include <termio.h>
#include <stdint.h>
#include <cstring>

#ifndef __UTIL_H__
#define __UTIL_H__

class CvUtil
{
public:
    CvUtil();
    ~CvUtil();
    int getkey();
};

#endif // __UTIL_H__

/*! @} */

