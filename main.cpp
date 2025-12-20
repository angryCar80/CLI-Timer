#define Colorgreen "\033[32m"
#define Colorreset "\033"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <ncurses.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCIFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios)) {
    die("tcsetattr");
  }

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_iflag &= ~(ICRNL);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

int main() {
  std::map<std::string, int> options;
  options["Seconds"] = 0;
  options["Minutes"] = 1;
  options["Hours"] = 2;

  // Initialize ncurses
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();

  printw(Colorgreen "> " Colorreset);
  for (auto option : options) {
    printw("%s\n", option.first.c_str());
  }

  printw("\nSelect an option (0 for Seconds, 1 for Minutes, 2 for Hours): ");
  refresh();

  int ch = getch(); // Wait for input
  if (ch == '0') {
    printw("You selected Seconds.\n");
  } else if (ch == '1') {
    printw("You selected Minutes.\n");
  } else if (ch == '2') {
    printw("You selected Hours.\n");
  } else {
    printw("Invalid selection.\n");
  }

  refresh();
  getch(); // Wait for the user to press any key to exit

  // Clean up ncurses
  endwin();
  return 0;
}
