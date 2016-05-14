#include "Jive.h"


#define TOF_FRAME_TYPE		DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME
//#define TOF_FRAME_TYPE		DepthCamera::FRAME_DEPTH_FRAME

vector<int> g_sliderPos;

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


/*!
 * @brief   Initialize control window based on Jive parameters
 */
void onTrackBar(int barVal, void *p)
{
   Mat control;
   std::tuple<float*,int, float> *t = (std::tuple<float*,int, float> *)p;
   float *fval = std::get<0>(*t);
   *fval = (float)barVal/(float)std::get<1>(*t);
}


/*!
 * @brief   Initialize control window based on Jive parameters
 */
void initControls(std::string ctrlWindow, Jive &eye)
{
   int i=0;
   for (map <std::string, std::tuple<float*,int,float> >::iterator it = eye.getParamMap().begin(); 
                                                                   it != eye.getParamMap().end(); it++) 
   {
      std::string s = it->first;
      std::tuple<float*,int,float> *t = &eye.getParamMap()[s];
      int maxVal = (int)floor(std::get<2>(*t)*std::get<1>(*t));
      g_sliderPos.push_back((int)floor(*std::get<0>(*t)*std::get<1>(*t)));
      createTrackbar(s, ctrlWindow, &g_sliderPos.data()[i++], maxVal, onTrackBar, (void*)t);
   }
}


/*!
 * @brief    Main program entry
 */
int main(int argc, char *argv[])
{
   int key;
   bool done = false;
   Jive eye(320,240);

   if (!eye.connect(TOF_FRAME_TYPE)) {
      cout << "Cannot connect" << endl;
      return -1;
   }
   
   eye.addMapToDisplay("zBkgMap");
   eye.addMapToDisplay("drawing");

   eye.initDisplays();
   eye.start();
   while (!done) {
      char key = getkey();
      if (key == 'q') 
         done = true;    
      else if (key == 'b')
         eye.sampleBackground();

      usleep(100000);
   }

err_exit:
   eye.stop();
}
