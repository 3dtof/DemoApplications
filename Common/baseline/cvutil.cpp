/*! @{ addgroup util
 *====================================================================
 *
 *  @file 	util.cpp
 *
 *  @brief	utlity class
 *
 *====================================================================
 */
#include "cvutil.h"

using namespace std;

CvUtil::CvUtil()
{
}

CvUtil::~CvUtil()
{
}


/*!
 * @brief  get a key stroke from console 
 */
int CvUtil::getkey() 
{
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}
/*! @} */

