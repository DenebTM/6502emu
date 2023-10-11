#include <future>
#include <optional>
#include <readline/readline.h>
#include <string.h>
#include <thread>
using namespace std::chrono_literals;

#include "cassette.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/plugin-types.hpp"

plugin_callback_t plugin_callback;

Datasette *datasette;
Pia *pia1;

std::thread *cmd_thread;

const char *CSI = "\33[";

bool cmd_thread_running = false;
void cmd_thread_fn() {
  std::cout << CSI << "1F";
  std::cout << "=== Datasette command interface ===" << std::endl;
  std::cout << "Available commands:" << std::endl;
  for (auto cmd : {"load <filename>", "play", "stop", "rewind", "exit"}) {
    std::cout << "- " << cmd << std::endl;
  }

  // force readline to not block the thread
  rl_event_hook = []() {
    if (!cmd_thread_running)
      rl_done = true;
    return 0;
  };

  while (cmd_thread_running) {
    char *in = readline("> ");
    if (!in) {
      in = (char *)"exit";
    } else if (strlen(in) == 0) {
      continue;
    }

    char *cmd = strtok(in, " ");
    std::string cmd_str = std::string(cmd);
    if (cmd_str == "load") {
      std::string filename = std::string(strtok(NULL, " "));
      datasette->load_tap(filename);
    } else if (cmd_str == "play") {
      datasette->play();
    } else if (cmd_str == "stop") {
      datasette->stop();
    } else if (cmd_str == "rewind") {
      datasette->rewind();
    } else if (cmd_str == "exit") {
      cmd_thread_running = false;
      plugin_callback(EMU_EXIT, (void *)0);
    }
  }
}

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  plugin_callback = callback;

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc) {
  datasette = new Datasette;

  std::future<void> wait_for_pia1 = std::async(std::launch::async, [&] {
    std::optional<MemoryMappedDevice *> dev_pia1 = std::nullopt;
    do {
      dev_pia1 = add_spc.get_dev(0xe810);
      std::this_thread::sleep_for(10ms);
    } while (!dev_pia1.has_value());

    pia1 = dynamic_cast<Pia *>(dev_pia1.value());
  });

  cmd_thread_running = true;
  cmd_thread = new std::thread(cmd_thread_fn);

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  cmd_thread_running = false;
  if (cmd_thread->joinable()) {
    cmd_thread->join();
  }
  delete cmd_thread;
  return 0;
}

extern "C" EXPORT int plugin_update() {
  // show tape loading status
  if (datasette->update() > 0) {
    std::cout << std::endl
              << (datasette->tap_index - TAP_HEADER_LEN) << " / " << (datasette->tap_size - TAP_HEADER_LEN);
    std::cout << CSI << "1F";
    std::cout << "> " << rl_line_buffer;
    fflush(stdout);
  }

  return 0;
}
