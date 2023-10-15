#pragma once
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "emu-types.hpp"
#include "mem-dev.hpp"

struct EmuConfig {
  using FilePath = std::string;
  using FileName = std::string;
  using StartAddress = Word;
  using Size = DWord;
  using ReadOnly = bool;
  using PluginID = std::string;

  EmuConfig(FilePath file_name);

  std::string config_file_name;

  bool init_reset = true;
  Word init_pc;
  uint64_t clock_speed = 1000000;

  std::vector<std::pair<StartAddress, Size>> ram;
  std::vector<std::tuple<FilePath, StartAddress, ReadOnly>> roms;

  struct PluginConfig {
    PluginID id;
    FileName filename;
    bool disable;
    StartAddress map_addr;

    bool operator==(PluginConfig &other) { return other.id == this->id; }
  };

  bool enumerate_plugins = true;
  std::vector<PluginConfig> plugin_configs;
  std::optional<PluginConfig> get_plugin_config(PluginID id);
};

extern EmuConfig *config;
