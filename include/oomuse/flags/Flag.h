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

#ifndef OOMUSE_FLAGS_FLAG_H
#define OOMUSE_FLAGS_FLAG_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "oomuse/core/Validator.h"
#include "oomuse/core/int_types.h"
#include "oomuse/core/readability_macros.h"
#include "oomuse/core/strings.h"
#include "oomuse/flags/flags.h"

namespace oomuse {


/** Indicates whether a flag must be explicitly set for correctness. */
enum class FlagRequired {YES, NO};


/** Abstract, type-independent base class for a command-line flag. */
class AbstractFlag {
 public:
  virtual ~AbstractFlag() {}

  const std::string& name() const { return name_; }
  const std::string& description() const { return description_; }
  bool isRequired() const { return isRequired_; }

  /** Returns true if flag has a value (default or explicit). */
  virtual bool hasValue() const = 0;

  /** Returns true if this flag was configured with a default value. */
  virtual bool hasDefaultValue() const = 0;

  /** Returns default value as a printable string, "" if none. */
  virtual std::string printableDefaultValue() const = 0;

 protected:
  AbstractFlag(const std::string& name, const std::string& description,
               FlagRequired flagRequired)
      : name_(name), description_(description),
        isRequired_(flagRequired == FlagRequired::YES) {
    assert(!name.empty());
    assert(!description.empty());

    oomuse::flags::FlagsInternal::registerFlag(this);
  }

  /** Parses, validates, sets flag value, and returns true if successful. */
  virtual bool parseValidateAndSet(const std::string& textValue) = 0;

  /** Outputs error message about an invalid value for this flag. */
  void outputError(const std::string& textValue, const std::string& errorMsg) {
    oomuse::flags::FlagsInternal::outputStream()
        << "Invalid value for flag --" << name() << ": " << textValue << ". "
        << errorMsg << std::endl;
  }

 private:
  CANT_COPY(AbstractFlag);

  friend oomuse::flags::FlagsInternal;  // For access to parseValidateAndSet().

  const std::string name_;
  const std::string description_;
  bool isRequired_;
};


/**
 * A command-line flag, specified by the user as --flagName=value. Additionally,
 * bool flags can be set to true with just --flagName. All flags must be
 * initialized in main() before using by calling oomuse::flags::init().
 *
 * Supported types: bool, int32, int64, float, double, string.
 */
template<typename T>
class Flag : public AbstractFlag {
 public:
  /** Type alias for unique pointer to a Validator. */
  using UniqueValidator = std::unique_ptr<oomuse::Validator<T>>;

  /** Creates a new, optional Flag. */
  Flag(const std::string& name, const std::string& description);

  /** Creates a new, optional Flag with a value validator. */
  Flag(const std::string& name, const std::string& description,
       UniqueValidator validator1);

  /** Creates a new, optional Flag with two value validators. */
  Flag(const std::string& name, const std::string& description,
       UniqueValidator validator1, UniqueValidator validator2);

  /** Creates a new Flag. */
  Flag(const std::string& name, const std::string& description,
       FlagRequired flagRequired);

  /** Creates a new Flag with a value validator. */
  Flag(const std::string& name, const std::string& description,
       FlagRequired flagRequired, UniqueValidator validator1);

  /** Creates a new Flag with two value validators. */
  Flag(const std::string& name, const std::string& description,
       FlagRequired flagRequired, UniqueValidator validator1,
       UniqueValidator validator2);

  /** Creates a new Flag with a default value. */
  Flag(const std::string& name, const std::string& description, T defaultValue);

  /** Creates a new Flag with a default value and value validator. */
  Flag(const std::string& name, const std::string& description, T defaultValue,
       UniqueValidator validator1);

  /** Creates a new Flag with a default value and two value validators. */
  Flag(const std::string& name, const std::string& description, T defaultValue,
       UniqueValidator validator1, UniqueValidator validator2);

  virtual bool hasValue() const override { return hasValue_; }

  /** Returns value of this command-line flag; error to call if !hasValue(). */
  const T& value() const {
    assert(hasValue_);
    return value_;
  }

  virtual bool hasDefaultValue() const override { return hasDefaultValue_; }

  /** Returns default value of this command-line flag; error to call if none. */
  const T& defaultValue() const {
    assert(hasDefaultValue_);
    return defaultValue_;
  }

  virtual std::string printableDefaultValue() const override;

 protected:
  virtual bool parseValidateAndSet(const std::string& textValue) override;

 private:
  Flag(const std::string& name, const std::string& description,
       FlagRequired flagRequired, T defaultValue, bool hasDefaultValue,
       UniqueValidator validator1, UniqueValidator validator2);

  CANT_COPY(Flag);

  bool validateAndSet(const T& value);
  bool passesCustomValidators(const T& value);
  bool passesValidator(const oomuse::Validator<T>& validator, const T& value);

  T value_;
  bool hasValue_;

  T defaultValue_;
  bool hasDefaultValue_;

  std::vector<UniqueValidator> validators_;
};


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description)
    : Flag(name, description, FlagRequired::NO, T(), false, nullptr, nullptr) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              UniqueValidator validator1)
    : Flag(name, description, FlagRequired::NO, T(), false,
           std::forward<UniqueValidator>(validator1), nullptr) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              UniqueValidator validator1, UniqueValidator validator2)
    : Flag(name, description, FlagRequired::NO, T(), false,
           std::forward<UniqueValidator>(validator1),
           std::forward<UniqueValidator>(validator2)) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              FlagRequired flagRequired)
    : Flag(name, description, flagRequired, T(), false, nullptr, nullptr) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              FlagRequired flagRequired, UniqueValidator validator1)
    : Flag(name, description, flagRequired, T(), false,
           std::forward<UniqueValidator>(validator1), nullptr) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              FlagRequired flagRequired, UniqueValidator validator1,
              UniqueValidator validator2)
    : Flag(name, description, flagRequired, T(), false,
           std::forward<UniqueValidator>(validator1),
           std::forward<UniqueValidator>(validator2)) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              T defaultValue)
    : Flag(name, description, FlagRequired::NO, defaultValue, true,
           nullptr, nullptr) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              T defaultValue, UniqueValidator validator1)
    : Flag(name, description, FlagRequired::NO, defaultValue, true,
           std::forward<UniqueValidator>(validator1), nullptr) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              T defaultValue, UniqueValidator validator1,
              UniqueValidator validator2)
    : Flag(name, description, FlagRequired::NO, defaultValue, true,
           std::forward<UniqueValidator>(validator1),
           std::forward<UniqueValidator>(validator2)) {
}


template<typename T>
Flag<T>::Flag(const std::string& name, const std::string& description,
              FlagRequired flagRequired, T defaultValue, bool hasDefaultValue,
              UniqueValidator validator1, UniqueValidator validator2)
    : AbstractFlag(name, description, flagRequired),
      value_(hasDefaultValue ? defaultValue : T()), hasValue_(hasDefaultValue),
      defaultValue_(defaultValue), hasDefaultValue_(hasDefaultValue) {
  if (validator1) {
    validators_.emplace_back(std::move(validator1));
  }
  if (validator2) {
    validators_.emplace_back(std::move(validator2));
  }
}


template<typename T>
inline std::string Flag<T>::printableDefaultValue() const {
  if (!hasDefaultValue_) {
    return "";
  }

  std::stringstream ss;
  ss << std::boolalpha << defaultValue_;
  return ss.str();
}


template<>
inline bool Flag<bool>::parseValidateAndSet(const std::string& textValue) {
  std::string lowercaseText =
      oomuse::strings::toLowerCase(oomuse::strings::trimWhitespace(textValue));

  if ((lowercaseText == "true") || (lowercaseText == "")) {
    return validateAndSet(true);
  } else if (lowercaseText == "false") {
    return validateAndSet(false);
  }

  outputError(textValue, "Must be true or false.");
  return false;
}


template<>
inline bool Flag<int32>::parseValidateAndSet(const std::string& textValue) {
  std::stringstream textStream(textValue);
  int32 value;
  textStream >> value;

  // Fail if text couldn't be converted to an int32 or if more text remains.
  if (textStream.fail() || !(textStream >> std::ws).eof()) {
    outputError(textValue, "Must be an int32 number.");
    return false;
  }

  return validateAndSet(value);
}


template<>
inline bool Flag<int64>::parseValidateAndSet(const std::string& textValue) {
  std::stringstream textStream(textValue);
  int64 value;
  textStream >> value;

  // Fail if text couldn't be converted to an int64 or if more text remains.
  if (textStream.fail() || !(textStream >> std::ws).eof()) {
    outputError(textValue, "Must be an int64 number.");
    return false;
  }

  return validateAndSet(value);
}


template<>
inline bool Flag<float>::parseValidateAndSet(const std::string& textValue) {
  std::stringstream textStream(textValue);
  float value;
  textStream >> value;

  // Fail if text couldn't be converted to a float or if more text remains.
  if (textStream.fail() || !(textStream >> std::ws).eof()
      || !std::isfinite(value)) {
    outputError(textValue, "Must be a finite float number.");
    return false;
  }

  return validateAndSet(value);
}


template<>
inline bool Flag<double>::parseValidateAndSet(const std::string& textValue) {
  std::stringstream textStream(textValue);
  double value;
  textStream >> value;

  // Fail if text couldn't be converted to a double or if more text remains.
  if (textStream.fail() || !(textStream >> std::ws).eof()
      || !std::isfinite(value)) {
    outputError(textValue, "Must be a finite double number.");
    return false;
  }

  return validateAndSet(value);
}


template<>
inline bool Flag<std::string>::parseValidateAndSet(
    const std::string& textValue) {
  return validateAndSet(textValue);
}


template<typename T>
bool Flag<T>::validateAndSet(const T& value) {
  // Validate:
  if (!passesCustomValidators(value)) {
    return false;
  }

  // Set:
  value_ = value;
  hasValue_ = true;
  return true;
}


template<typename T>
bool Flag<T>::passesCustomValidators(const T& value) {
  return std::all_of(validators_.begin(), validators_.end(),
      [this, &value](const UniqueValidator& validator) {
        return passesValidator(*validator, value);
      });
}


template<typename T>
bool Flag<T>::passesValidator(const oomuse::Validator<T>& validator,
                              const T& value) {
  std::string validationError = validator.checkValidationErrors(value);
  if (!validationError.empty()) {
    // Convert value to string and output error.
    std::stringstream ss;
    ss << value;

    outputError(ss.str(), validationError);
    return false;
  }

  return true;
}


}  // namespace oomuse

#endif  // OOMUSE_FLAGS_FLAG_H
