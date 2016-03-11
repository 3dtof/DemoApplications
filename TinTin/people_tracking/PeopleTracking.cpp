#include "Horus.h"

int getkey() {
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

#define TOF_FRAME_TYPE		DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME
//#define TOF_FRAME_TYPE		DepthCamera::FRAME_DEPTH_FRAME

int main(int argc, char *argv[])
{
   int key;
   bool done = false;
   Mat bImg;
   //Horus eye(80, 60);
   //Horus eye(160, 120);
   Horus eye(320, 240);
   
   if (!eye.connect(TOF_FRAME_TYPE)) {
      cout << "Cannot connect" << endl;
      return -1;
   }

   eye.start();
   while (!done) {
      char key = getkey();
      if (key == 'q') 
         done = true;
      else if (key == 'b') 
         eye.resetBackground();
      
      usleep(100000);
   }

err_exit:
   eye.stop();
}
