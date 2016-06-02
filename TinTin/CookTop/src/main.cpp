#include "ofMain.h"
#include "ofApp.h"

#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

/*! 
 *=============================================================================
 *  @brief     Main program
 *=============================================================================
 */
int main( )
{
	ofSetupOpenGL(SCREEN_WIDTH,SCREEN_HEIGHT, OF_FULLSCREEN);			
 
	ofRunApp( new ofApp());
}
