#include <chrono>
#include <iostream>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define green "\033[92m"
#define reset "\033[0m"

struct termios orig_termios;

void wait() {
  auto start = std::chrono::steady_clock::now();
  std::this_thread::sleep_for(std::chrono::seconds(15));
  auto end = std::chrono::steady_clock::now();
}

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

  std::vector<std::string> options = {"Seconds", "Minutes", "Houres"};

  int selected = 0;
  int timecount = 0;

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
    // ENTER key
    if (c == '\r') {
      clearScreen();
      disableRawMode();
      std::cout << "You selected: " << options[selected] << "\n";
      std::cout << "How many " << options[selected] << ": ";
      std::cin >> timecount;
      // STARTING TIMER
      auto start = std::chrono::steady_clock::now();
      if (selected == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(timecount));
      } else if (selected == 1) {
        std::this_thread::sleep_for(std::chrono::minutes(timecount));
      } else if (selected == 2) {
        std::this_thread::sleep_for(std::chrono::hours(timecount));
      }
      auto end = std::chrono::steady_clock::now();

      std::chrono::duration<float> elasped_time = end - start;

      std::cout << "TIMER END: " << elasped_time.count() << options[selected]
                << "passed";
      // wait();
      enableRawMode();
    }
  }

  disableRawMode();
  return 0;
}
