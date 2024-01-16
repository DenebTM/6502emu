#include <atomic>
#include <thread>

#include "ui/main_window.hpp"
#include "ui/ui_thread.hpp"

std::thread *ui_thread;
std::atomic_bool ui_thread_running = true;

void start_ui_thread() {
  ui_thread = new std::thread([] {
    if (main_window_init() != 0) {
      return;
    }

    while (ui_thread_running) {
      main_window_update();
    }

    main_window_destroy();
  });
}

void stop_ui_thread() {
  ui_thread_running = false;

  if (ui_thread->joinable())
    ui_thread->join();

  delete ui_thread;
}
