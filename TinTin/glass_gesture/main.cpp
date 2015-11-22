#include "AirMouse.h"

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

int main(int argc, char *argv[])
{
   AirMouse am;
   
   if (argc == 2) {
      if (string(argv[1]) == string("d")) {
         cout << "debug." << endl;
         am.setDebug(true);
      }
   }

   am.start();

   bool done = false;
   while (!done) {
      int key = getkey();
      if (key == 'q') done = true;
      usleep(100000);
   }
}
