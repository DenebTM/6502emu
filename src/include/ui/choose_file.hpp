#pragma once
#include <functional>
#include <optional>
#include <string>

namespace ui {

/**
 * open a file picker in a background thread, call `func` with the chosen file name when done
 *
 * only one file picker may be opened at a time
 */
void choose_file(std::function<void(std::string)> func);

/**
 * open a file picker in a background thread, store the chosen file name in `out` when done
 *
 * only one file picker may be opened at a time
 */
void choose_file(std::string &out);

} // namespace ui
