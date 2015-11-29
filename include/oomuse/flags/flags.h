/**
 * Copyright 2015 Eric W. Barndollar. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * =============================================================================
 * Simple library for parsing and validating command-line flags. Default
 * supported flag types are bool, int32, int64, float, double, and string.
 *
 * To support other types, you can provide a specialized implementation of
 * Flag<YourType>::parseValidateAndSet(const std::string& textValue); see Flag.h
 * to reference the default implementations.
 *
 * Sample command-line usage:
 *   $ your_program_name --verbose --username=slyfox31 --retry_limit=3
 *
 * Sample code usage:
 *
 * using oomuse::Flag;
 * using oomuse::FlagRequired;
 * using oomuse::Validators;
 * namspace flags = oomuse::flags;
 *
 * Flag<int32> retryLimit("retry_limit", "Max # of times to retry",
 *                        Validators<int32>::greaterOrEqual(1));
 * Flag<string> username("username", "Username between 3 and 15 characters",,
 *                       FlagRequired::YES,
 *                       Validators<string>::sizeGreaterOrEqual(3),
 *                       Validators<string>::sizeLessOrEqual(15));
 * Flag<bool> verbose("verbose", "Set --verbose to print extra info", false);
 * Flag<string> specialGreeting("special_greeting", "An optional message",
 *                              "Hello, this is the default greeting.");
 *
 * int main(int argc, char* argv[]) {
 *   // Initialize and validate all command-line flags. If any required flags
 *   // are missing or any
 *   flags::initOrPrintUsageAndDie(
 *       &argc, argv, "your_program_name", "positional_arg1 positional_arg2",
 *       "Special notes on usage go here.");
 *
 *   // Parse any positional command-line args (flags lib updates argc and
 *   // argv[], removing all parsed flags).
 *   if (argc >= 3) {
 *     string positionalArg1 = argv[1];
 *     string positionalArg2 = argv[2];
 *     ...
 *   }
 *
 *   // Check whether optional flags are set with hasValue().
 *   if (retryLimit.hasValue()) {
 *     int32 theRetryLimit = retryLimit.value();
 *     ...
 *   }
 *
 *   // Required flags are guaranteed to be set after initialization.
 *   string theUsername = username.value();
 *   ...
 *
 *   // Flags with a default value will return the default if no user-provided
 *   // value was passed on the command line.
 *   bool shouldPrintVerboseInfo = verbose.value();
 *   string theSpecialGreeting = specialGreeting.value();
 *   ...
 * }
 */

#ifndef OOMUSE_FLAGS_FLAGS_H
#define OOMUSE_FLAGS_FLAGS_H

#include <ostream>
#include <string>

#include "oomuse/core/readability_macros.h"

namespace oomuse {
  class AbstractFlag;
}

namespace oomuse {
namespace flags {


/**
 * Parses and validates all command-line flags, removing all flags and values
 * from argv[] and updating *argcPtr to include only the program name and
 * remaining positional arguments. Any validation errors will be printed to
 * standard error, or an alternate stream can be set by setOutputStream().
 * Returns true if successful.
 */
bool init(int* argcPtr, char* argv[]);

/** Like init(), but accepts const char* argv[] instead. */
bool init(int* argcPtr, const char* argv[]);

/** Like init(), but terminates the program if unsuccessful. */
void initOrDie(int* argcPtr, const char* argv[]);

/** Like initOrDir(), but also calls printUsage() before death on error. */
void initOrPrintUsageAndDie(int* argcPtr, const char* argv[],
                            const std::string& programName,
                            const std::string& positionalArgs = "",
                            const std::string& usageNotes = "");

/**
 * Outputs info about program usage in the following format:
 *
 * Usage: <programName> [flags] [<positionalArgs>]
 *   [<usageNotes>]
 *
 * Required flags:
 *   --<flagName>: <description>
 *   ...
 *
 * Optional flags:
 *   --<flagName>: <description> (default: <defaultValue>)
 *   ...
 */
void printUsage(const std::string& programName,
                const std::string& positionalArgs = "",
                const std::string& usageNotes = "");

/** Changes output stream that error and usage messages are output to. */
void setOutputStream(std::ostream* outputStream);

/** Testing only: clears all registered flags. Test with flags on stack. */
void resetForTest();


/** For internal flags library usage. */
class FlagsInternal {
 private:
  CANT_INSTANTIATE(FlagsInternal);

  // For access to these functions:
  friend AbstractFlag;
  friend bool init(int* argcPtr, const char* argv[]);

  /** Returns output stream for error messages (standard error by default). */
  static std::ostream& outputStream();

  /** For AbstractFlag: registers given flag so it can be parsed & set. */
  static void registerFlag(AbstractFlag* flag);

  /** Parses, validates, and sets the given flag from the user's fullArg. */
  static bool parseValidateAndSet(AbstractFlag* flag,
                                  const std::string& fullArg);
};


}  // namespace flags
}  // namespace oomuse

#endif  // OOMUSE_FLAGS_FLAGS_H
