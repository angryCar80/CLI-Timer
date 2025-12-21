#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define green "\033[92m"
#define reset "\033[0m"

struct termios orig_termios;

void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON | ISIG);
  raw.c_iflag &= ~(ICRNL | IXON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void clearScreen() { std::cout << "\x1b[2J\x1b[H"; }

int main() {
  enableRawMode();

  std::vector<std::string> options = {"Seconds", "Minuts", "Houres"};

  int selected = 0;

  while (true) {
    clearScreen();

    for (int i = 0; i < options.size(); i++) {
      if (i == selected) {
        std::cout << green << "> " << options[i] << reset << "\n";
      } else {
        std::cout << "  " << options[i] << "\n";
      }
    }

    char c;
    if (read(STDIN_FILENO, &c, 1) != 1)
      continue;

    // Quit
    if (c == 'q')
      break;

    // ENTER key
    if (c == '\r') {
      clearScreen();
      std::cout << "You selected: " << options[selected] << "\n";
      break;
    }

    // Arrow keys
    if (c == '\x1b') {
      char seq[2];
      if (read(STDIN_FILENO, &seq[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        continue;

      if (seq[0] == '[') {
        if (seq[1] == 'A' && selected > 0) {
          selected--; // up
        } else if (seq[1] == 'B' && selected < options.size() - 1) {
          selected++; // down
        }
      }
      continue;
    }

    // Vim-style keys
    if (c == 'j' && selected < options.size() - 1)
      selected++;
    else if (c == 'k' && selected > 0)
      selected--;
  }

  disableRawMode();
  return 0;
}
