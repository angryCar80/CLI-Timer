#include <map>
#define green "\e[0;32m"
#define reset "\e[0;37m"

#include <cstdio>
#include <iostream>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    perror("tcsetattr");
  }
}

void enableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    perror("tcsetattr");
  }

  struct termios raw = orig_termios;

  // raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
}

int main() {
  std::cout << green << "=== Welcome to the timer ===\n" << reset;
  std::map<std::string, int> options;

  options["Seconds"] = 0;
  options["Minutes"] = 1;
  options["Houres"] = 2;

  std::cout << "Counting Options: \n";
  std::cout << green << "> " << reset;
  for (const auto &option : options) {
    std::cout << option.first << "\n";
  }

  return 0;
}
