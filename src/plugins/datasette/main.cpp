#include <future>
#include <optional>
#include <readline/readline.h>
#include <string.h>
#include <thread>
using namespace std::chrono_literals;

#include "cassette.hpp"
#include "mem.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/plugin-types.hpp"

Datasette *datasette;
Pia *pia1;

std::thread *cmd_thread;

bool cmd_thread_running = false;
void cmd_thread_fn() {
  std::cout << "Datasette commands:" << std::endl;
  for (auto cmd : {"load <filename>", "play", "stop", "rewind"}) {
    std::cout << "- " << cmd << std::endl;
  }

  while (cmd_thread_running) {
    char *in = readline("> ");
    if (!in)
      return;

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
    }
  }
}

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  datasette = new Datasette;
  // datasette->load_tap("/home/deneb/Downloads/lunar-lander.tap");
  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc) {
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
  cmd_thread_running = true;
  if (cmd_thread->joinable()) {
    cmd_thread->join();
  }
  delete cmd_thread;
  return 0;
}

extern "C" EXPORT int plugin_update() {
  datasette->update();
  return 0;
}
