#pragma once
#include <string>
#include <tuple>
#include <vector>

#include "emu-common.hpp"
#include "mem-dev.hpp"

struct EmuConfig {
  using FilePath = std::string;
  using FileName = std::string;
  using StartAddress = Word;
  using Size = DWord;
  using ReadOnly = bool;
  using PluginDisable = bool;

  EmuConfig(FilePath file_name);

  std::string config_file_name;

  bool init_reset = true;
  Word init_pc;
  uint64_t clock_speed = 1000000;

  std::vector<std::pair<StartAddress, Size>> ram;
  std::vector<std::tuple<FilePath, StartAddress, ReadOnly>> roms;

  bool enumerate_plugins = true;
  std::vector<std::tuple<FileName, StartAddress, PluginDisable>> plugin_configs;
};

extern EmuConfig *config;
