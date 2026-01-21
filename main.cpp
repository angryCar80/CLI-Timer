#include <cstdlib>
#define MINIAUDIO_IMPLEMENTATION
#include <chrono>
#include <iostream>
#include <miniaudio/miniaudio.h>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

// adding colors to the TUI tool
#define green "\033[92m"
#define reset "\033[0m"
#define cyan "\e[0;36m"

struct termios orig_termios;

void progressBar() {
  float progress = 0.0; // the value
  while (progress < 1.0) {
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
      if (i < pos)
        std::cout << "=";
      else if (i == pos)
        std::cout << ">";
      else
        std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) + 1 << " %\r";
    std::cout.flush();

    progress += 0.01; // for demonstration only
    std::this_thread::sleep_for(std::chrono::milliseconds(
        100)); // Add delay to see progress more clearly
  }
  std::cout << std::endl;
}

void wait(int amount) {
  auto start = std::chrono::steady_clock::now();
  std::this_thread::sleep_for(std::chrono::seconds(amount));
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
  ma_engine engine;
  ma_result result =
      ma_engine_init(NULL, &engine); // Initialize the audio engine

  if (result != MA_SUCCESS) {
    printf("Failed to initialize audio engine: %s\n",
           ma_result_description(result));
    return -1;
  }

  enableRawMode();

  std::vector<std::string> options = {"Seconds", "Minutes", "Hours"};
  std::vector<std::string> afteroptions = {"Get To Home", "Repeat"};

  int selected = 0;
  int anotherselected = 0;
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
    if (read(STDIN_FILENO, &c, 1) != 1) {
      continue;
    }

    // Quit if 'q' is pressed
    if (c == 'q') {
      break;
    }

    // Arrow keys for navigation
    if (c == '\x1b') { // This starts the escape sequence
      char seq[2];
      if (read(STDIN_FILENO, &seq[0], 1) != 1) {
        continue;
      }
      if (read(STDIN_FILENO, &seq[1], 1) != 1) {
        continue;
      }

      if (seq[0] == '[') {
        if (seq[1] == 'A' && selected > 0) { // up arrow
          selected--;
        } else if (seq[1] == 'B' &&
                   selected < options.size() - 1) { // down arrow
          selected++;
        }
      }
    }

    // Vim-style keys for navigation
    if (c == 'j' && selected < options.size() - 1) {
      selected++;
    } else if (c == 'k' && selected > 0) {
      selected--;
    }

    // ENTER key to proceed with the selected option
    if (c == '\r') {
      clearScreen();
      disableRawMode();
      std::cout << cyan << "You selected: " << options[selected] << reset
                << "\n";
      std::cout << green << "How many " << options[selected] << ": " << reset;
      std::cin >> timecount;

      // Starting the timer
      auto start = std::chrono::steady_clock::now();
      if (selected == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(timecount));
      } else if (selected == 1) {
        std::this_thread::sleep_for(std::chrono::minutes(timecount));
      } else if (selected == 2) {
        std::this_thread::sleep_for(std::chrono::hours(timecount));
      }
      auto end = std::chrono::steady_clock::now();

      std::chrono::duration<float> elapsed_time = end - start;

      std::cout << "TIMER END: " << elapsed_time.count() << " "
                << options[selected] << " passed\n";

      // Play the sound
      result = ma_engine_play_sound(&engine, "sound.mp3",
                                    nullptr); // Corrected path
      if (result != MA_SUCCESS) {
        std::cout << "Failed to play sound: " << ma_result_description(result)
                  << "\n";
      }

      wait(5); // Allow time to check sound
      enableRawMode();

      // After timer ends, show options
      while (true) {
        clearScreen();
        for (int i = 0; i < afteroptions.size(); ++i) {
          if (i == anotherselected) {
            std::cout << green << "> " << afteroptions[i] << reset << "\n";
          } else {
            std::cout << "  " << afteroptions[i] << "\n";
          }
        }

        char ca;
        if (read(STDIN_FILENO, &ca, 1) != 1)
          continue;

        if (ca == 'q') { // Quit if 'q' is pressed
          break;
        }

        if (ca == '\x1b') {
          char seq[2];
          if (read(STDIN_FILENO, &seq[0], 1) != 1)
            continue;
          if (read(STDIN_FILENO, &seq[1], 1) != 1)
            continue;

          if (seq[0] == '[') {
            if (seq[1] == 'A' && anotherselected > 0) { // up arrow
              anotherselected--;
            } else if (seq[1] == 'B' && anotherselected < afteroptions.size() -
                                                              1) { // down arrow
              anotherselected++;
            }
          }
        } else if (ca == 'j' && anotherselected < afteroptions.size() -
                                                      1) { // Down (Vim-style)
          anotherselected++;
        } else if (ca == 'k' && anotherselected > 0) { // Up (Vim-style)
          anotherselected--;
        } else if (ca == '\r') {
          if (anotherselected == 0) { // Home option
            std::cout << "Returning to Home...\n";
            break;
          } else if (anotherselected == 1) { // Repeat option
            std::cout
                << "Bro, I don't know how to do it, code it yourself\n"; // BRUH
                                                                         // JK
                                                                         // AM
                                                                         // COMING
                                                                         // BACK
                                                                         // TO
                                                                         // THIS
            wait(3);
          }
        }
      }
    }
  }
  disableRawMode();
  return 0;
}
