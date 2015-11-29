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
 */

#include "oomuse/flags/flags.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include "oomuse/core/strings.h"
#include "oomuse/flags/Flag.h"

using oomuse::AbstractFlag;
using std::cerr;
using std::endl;
using std::exit;
using std::map;
using std::ostream;
using std::string;
using std::vector;

namespace strings = oomuse::strings;

namespace {


bool hasBeenInitialized = false;
ostream* output = &cerr;


map<string, AbstractFlag*>& flagMap() {
  // Use static variable to control static initialization order.
  static map<string, AbstractFlag*> theMap;
  return theMap;
}


string getFlagName(const string& fullArg) {
  if (!strings::startsWith(fullArg, "--") || (fullArg.length() < 3)) {
    return "";
  }

  // Flag name continues up to equals sign (if any).
  auto equalsIndex = fullArg.find('=');
  auto nameLength = (equalsIndex != string::npos) ? equalsIndex - 2
                                                  : fullArg.length() - 2;
  return fullArg.substr(2, nameLength);
}


AbstractFlag* getFlag(const string& flagName) {
  if (flagMap().count(flagName) == 0) {
    *output << "Unrecognized command-line flag: --" << flagName << endl;
    return nullptr;
  }

  return flagMap()[flagName];
}


vector<const AbstractFlag*> getFlagsByRequiredness(bool required) {
  vector<const AbstractFlag*> flags;

  for (auto& entry : flagMap()) {
    AbstractFlag* flag = entry.second;
    if (flag->isRequired() == required) {
      flags.push_back(flag);
    }
  }

  return flags;
}


void printFlags(const vector<const AbstractFlag*>& flags, const string& label) {
  if (flags.empty()) {
    return;
  }

  *output << endl << label << endl;

  for (auto& flag : flags) {
    *output << "  --" << flag->name() << ": " << flag->description();
    if (flag->hasDefaultValue()) {
      *output << " (default: " << flag->printableDefaultValue() << ")";
    }
    *output << endl;
  }
}


bool areAllRequiredFlagsSet() {
  bool allAreSet = true;

  for (auto& entry : flagMap()) {
    AbstractFlag* flag = entry.second;
    if (flag->isRequired() && !flag->hasValue()) {
      *output << "Missing required command-line flag --"
              << flag->name() << "." << endl;
      allAreSet = false;
    }
  }

  return allAreSet;
}


}  // namespace


namespace oomuse {
namespace flags {


bool init(int* argcPtr, char* argv[]) {
  return init(argcPtr, const_cast<const char**>(argv));
}


bool init(int* argcPtr, const char* argv[]) {
  assert(!hasBeenInitialized);
  hasBeenInitialized = true;

  // Iterate over all command-line args and set any matching flags.
  // Remove flags from argv[], keeping only remaining positional args.
  const char** nextPositionalArg = &argv[1];
  for (const char** arg = &argv[1]; *arg; ++arg) {
    string fullArg = *arg;

    // Formatted like a command-line flag?
    string flagName = getFlagName(fullArg);
    if (flagName.empty()) {
      // No, it's a positional arg: keep it in argv[].
      *nextPositionalArg = *arg;
      ++nextPositionalArg;
      continue;
    }

    // Yes, this is a --flag arg, look for matching Flag.
    AbstractFlag* flag = getFlag(flagName);
    if (!flag) {
      return false;
    }

    // Initialize it.
    bool wasValid = FlagsInternal::parseValidateAndSet(flag, fullArg);
    if (!wasValid) {
      return false;
    }
  }

  // Terminate argv[] and update argc to count remaining positional args.
  *nextPositionalArg = nullptr;
  *argcPtr = static_cast<int>(nextPositionalArg - &argv[0]);
  return areAllRequiredFlagsSet();
}


void initOrDie(int* argcPtr, const char* argv[]) {
  if (!init(argcPtr, argv)) {
    exit(EXIT_FAILURE);
  }
}


void initOrPrintUsageAndDie(int* argcPtr, const char* argv[],
                            const string& programName,
                            const string& positionalArgs,
                            const string& usageNotes) {
  if (!init(argcPtr, argv)) {
    printUsage(programName, positionalArgs, usageNotes);
    exit(EXIT_FAILURE);
  }
}


void printUsage(const string& programName,
                const string& positionalArgs,
                const string& usageNotes) {
  vector<const AbstractFlag*> requiredFlags = getFlagsByRequiredness(true);
  vector<const AbstractFlag*> optionalFlags = getFlagsByRequiredness(false);

  // Print program name, positional arguments, and usage notes.
  *output << "Usage: " << programName;
  if (!requiredFlags.empty() || !optionalFlags.empty()) {
    *output << (requiredFlags.empty() ? " [flags]" : " flags");
  }
  if (!positionalArgs.empty()) {
    *output << " " << positionalArgs;
  }
  *output << endl;

  if (!usageNotes.empty()) {
    *output << "  " << usageNotes << endl;
  }

  // Print flag info.
  printFlags(requiredFlags, "Required flags:");
  printFlags(optionalFlags, "Optional flags:");
}


void setOutputStream(ostream* outputStream) {
  assert(outputStream);
  output = outputStream;
}


void resetForTest() {
  hasBeenInitialized = false;
  flagMap().clear();
}


ostream& FlagsInternal::outputStream() {
  return *output;
}


void FlagsInternal::registerFlag(AbstractFlag* flag) {
  assert(flag);
  assert(flagMap().count(flag->name()) == 0);

  flagMap()[flag->name()] = flag;
}


bool FlagsInternal::parseValidateAndSet(AbstractFlag* flag,
                                        const string& fullArg) {
  auto equalsIndex = fullArg.find('=');
  string textValue = (equalsIndex != string::npos)
      ? fullArg.substr(equalsIndex + 1)
      : "";

  return flag->parseValidateAndSet(textValue);
}


}  // namespace flags
}  // namespace oomuse
