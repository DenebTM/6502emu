#pragma once

/**
 * plugins have their symbols hidden by default as per `plugin-makefile-base`, use this macro
 * to override that.
 *
 * this should be done in particular for the plugin_init/destroy/load/update functions, and
 * classes that other plugins should be able to make use of, e.g.:
 * - `extern "C" EXPORT int <function_name>(...) { ... }`
 * - `class EXPORT <class_name> { ... };`
 */
#define EXPORT __attribute__((visibility("default")))
