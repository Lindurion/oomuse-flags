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

#include "oomuse/flags/Flag.h"
#include "oomuse/flags/flags.h"

#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "oomuse/core/Validators.h"
#include "oomuse/core/int_types.h"

using oomuse::Flag;
using oomuse::FlagRequired;
using oomuse::Validators;
using std::string;
using std::stringstream;
using testing::Test;

namespace flags = oomuse::flags;

namespace {


/** A sufficiently small double used for floating-point comparison. */
static const double DOUBLE_EPSILON = 0.000000000000001;

/** A sufficiently small float used for floating-point comparison. */
static const float FLOAT_EPSILON = 0.00001F;


/** Test fixture for common flags test setup. */
class FlagTest : public Test {
 protected:
  /** Reset flags library global state before every test. */
  FlagTest() {
    flags::resetForTest();
    flags::setOutputStream(&outputStream_);
  }

  /** Returns text that has been ouput to the configured output stream. */
  string output() const { return outputStream_.str(); }

 private:
  stringstream outputStream_;
};


TEST_F(FlagTest, parsesOptionalIntFlag) {
  Flag<int32> intFlag("intFlag", "A test flag with integer value");

  int argc = 4;
  const char* argv[] = {"App", "arg1", "--intFlag=42", "arg2", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));
  ASSERT_TRUE(intFlag.hasValue());
  EXPECT_EQ(42, intFlag.value());

  // Verify flag accessors.
  EXPECT_EQ("intFlag", intFlag.name());
  EXPECT_EQ("A test flag with integer value", intFlag.description());
  EXPECT_FALSE(intFlag.isRequired());
  EXPECT_FALSE(intFlag.hasDefaultValue());

  // Should strip flags out of argc, argv[].
  EXPECT_EQ(3, argc);
  EXPECT_STREQ("App", argv[0]);
  EXPECT_STREQ("arg1", argv[1]);
  EXPECT_STREQ("arg2", argv[2]);
  EXPECT_STREQ(nullptr, argv[3]);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, parsesRequiredStringFlag) {
  Flag<string> nameFlag("name", "Your first name", FlagRequired::YES);

  int argc = 2;
  const char* argv[] = {"App", "--name=Donald", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));
  ASSERT_TRUE(nameFlag.hasValue());
  EXPECT_EQ("Donald", nameFlag.value());

  // Verify flag accessors.
  EXPECT_EQ("name", nameFlag.name());
  EXPECT_EQ("Your first name", nameFlag.description());
  EXPECT_TRUE(nameFlag.isRequired());
  EXPECT_FALSE(nameFlag.hasDefaultValue());

  // Should strip flags out of argc, argv[].
  EXPECT_EQ(1, argc);
  EXPECT_STREQ("App", argv[0]);
  EXPECT_STREQ(nullptr, argv[1]);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, canSetNoFlagsIfNoneAreRequired) {
  // Testing explicit FlagRequired::NO value (which could instead be omitted).
  Flag<string> nameFlag("name", "Your first name", FlagRequired::NO);

  int argc = 3;
  const char* argv[] = {"App", "arg1", "arg2", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));
  EXPECT_FALSE(nameFlag.hasValue());

  // Verify flag accessors.
  EXPECT_EQ("name", nameFlag.name());
  EXPECT_EQ("Your first name", nameFlag.description());
  EXPECT_FALSE(nameFlag.isRequired());
  EXPECT_FALSE(nameFlag.hasDefaultValue());

  // Should leave argc, argv[] alone.
  EXPECT_EQ(3, argc);
  EXPECT_STREQ("App", argv[0]);
  EXPECT_STREQ("arg1", argv[1]);
  EXPECT_STREQ("arg2", argv[2]);
  EXPECT_STREQ(nullptr, argv[3]);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, canSetSomeOptionalFlags) {
  Flag<string> searchQueryFlag("searchQuery", "Optional search restrict");
  Flag<float> timeoutSecsFlag("timeoutSecs", "Optional timeout in seconds");

  int argc = 3;
  const char* argv[] = {"App", "--timeoutSecs=10", "arg1", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify searchQueryFlag state.
  EXPECT_FALSE(searchQueryFlag.hasValue());
  EXPECT_EQ("searchQuery", searchQueryFlag.name());
  EXPECT_EQ("Optional search restrict", searchQueryFlag.description());
  EXPECT_FALSE(searchQueryFlag.isRequired());
  EXPECT_FALSE(searchQueryFlag.hasDefaultValue());

  // Verify timeoutSecsFlag state.
  ASSERT_TRUE(timeoutSecsFlag.hasValue());
  EXPECT_NEAR(10.0F, timeoutSecsFlag.value(), FLOAT_EPSILON);
  EXPECT_EQ("timeoutSecs", timeoutSecsFlag.name());
  EXPECT_EQ("Optional timeout in seconds", timeoutSecsFlag.description());
  EXPECT_FALSE(timeoutSecsFlag.isRequired());
  EXPECT_FALSE(timeoutSecsFlag.hasDefaultValue());

  // Should strip flags out of argc, argv[].
  EXPECT_EQ(2, argc);
  EXPECT_STREQ("App", argv[0]);
  EXPECT_STREQ("arg1", argv[1]);
  EXPECT_STREQ(nullptr, argv[2]);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, defaultValuesApplyIffUnset) {
  Flag<bool> enableFeature1Flag("enableFeature1", "Enable feature 1?", false);
  Flag<bool> enableFeature2Flag("enableFeature2", "Enable feature 2?", true);

  int argc = 2;
  const char* argv[] = {"App", "--enableFeature2=false", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify enableFeature1Flag state.
  ASSERT_TRUE(enableFeature1Flag.hasValue());
  EXPECT_FALSE(enableFeature1Flag.value());
  EXPECT_EQ("enableFeature1", enableFeature1Flag.name());
  EXPECT_EQ("Enable feature 1?", enableFeature1Flag.description());
  EXPECT_FALSE(enableFeature1Flag.isRequired());
  EXPECT_TRUE(enableFeature1Flag.hasDefaultValue());
  EXPECT_FALSE(enableFeature1Flag.defaultValue());

  // Verify enableFeature2Flag state.
  ASSERT_TRUE(enableFeature2Flag.hasValue());
  EXPECT_FALSE(enableFeature2Flag.value());
  EXPECT_EQ("enableFeature2", enableFeature2Flag.name());
  EXPECT_EQ("Enable feature 2?", enableFeature2Flag.description());
  EXPECT_FALSE(enableFeature2Flag.isRequired());
  EXPECT_TRUE(enableFeature2Flag.hasDefaultValue());
  EXPECT_TRUE(enableFeature2Flag.defaultValue());

  // Should strip flags out of argc, argv[].
  EXPECT_EQ(1, argc);
  EXPECT_STREQ("App", argv[0]);
  EXPECT_STREQ(nullptr, argv[1]);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, validBoolValuesParseCorrectly) {
  Flag<bool> flag1("flag1", "Enable flag1?");
  Flag<bool> flag2("flag2", "Enable flag2?");
  Flag<bool> flag3("flag3", "Enable flag3?");

  int argc = 4;
  const char* argv[] =
      {"App", "--flag2=tRUe", "--flag1", "--flag3=false", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag values.
  EXPECT_TRUE(flag1.value());
  EXPECT_TRUE(flag2.value());
  EXPECT_FALSE(flag3.value());

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, invalidBoolValuesFailValidation) {
  Flag<bool> boolFlag("boolFlag", "Enable boolFlag?");

  int argc = 2;
  const char* argv[] = {"App", "--boolFlag=1", nullptr};

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for invalid bool value.
  EXPECT_EQ("Invalid value for flag --boolFlag: 1. Must be true or false.\n",
            output());
}


TEST_F(FlagTest, validNegativeNumbersParseCorrectly) {
  Flag<int32> int32Flag("int32Flag", "An int32 number");
  Flag<int64> int64Flag("int64Flag", "An int64 number");
  Flag<float> floatFlag("floatFlag", "A float number");
  Flag<double> doubleFlag("doubleFlag", "A double number");

  int argc = 5;
  const char* argv[] = {
    "App", "--int32Flag=-804", "--floatFlag=-0.65", "--doubleFlag=-42.37",
    "--int64Flag=-10000", nullptr
  };

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag values.
  EXPECT_EQ(-804, int32Flag.value());
  EXPECT_EQ(-10000L, int64Flag.value());
  EXPECT_NEAR(-0.65F, floatFlag.value(), FLOAT_EPSILON);
  EXPECT_NEAR(-42.37, doubleFlag.value(), DOUBLE_EPSILON);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, validZeroNumbersParseCorrectly) {
  Flag<int32> int32Flag("int32Flag", "An int32 number");
  Flag<int64> int64Flag("int64Flag", "An int64 number");
  Flag<float> floatFlag("floatFlag", "A float number");
  Flag<double> doubleFlag("doubleFlag", "A double number");

  int argc = 5;
  const char* argv[] = {
    "App", "--int64Flag=0", "--doubleFlag=0.0", "--floatFlag=0.000",
    "--int32Flag=0", nullptr
  };

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag values.
  EXPECT_EQ(0, int32Flag.value());
  EXPECT_EQ(0L, int64Flag.value());
  EXPECT_NEAR(0.0F, floatFlag.value(), FLOAT_EPSILON);
  EXPECT_NEAR(0.0, doubleFlag.value(), DOUBLE_EPSILON);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, validPositiveNumbersParseCorrectly) {
  Flag<int32> int32Flag("int32Flag", "An int32 number");
  Flag<int64> int64Flag("int64Flag", "An int64 number");
  Flag<float> floatFlag("floatFlag", "A float number");
  Flag<double> doubleFlag("doubleFlag", "A double number");

  int argc = 5;
  const char* argv[] = {
    "App", "--int32Flag=804", "--floatFlag=0.65", "--doubleFlag=42.37",
    "--int64Flag=10000", nullptr
  };

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag values.
  EXPECT_EQ(804, int32Flag.value());
  EXPECT_EQ(10000L, int64Flag.value());
  EXPECT_NEAR(0.65F, floatFlag.value(), FLOAT_EPSILON);
  EXPECT_NEAR(42.37, doubleFlag.value(), DOUBLE_EPSILON);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, validMinIntegersParseCorrectly) {
  Flag<int32> int32Flag("int32Flag", "An int32 number");
  Flag<int64> int64Flag("int64Flag", "An int64 number");

  int argc = 3;
  const char* argv[] = {
    "App", "--int64Flag=-9223372036854775808", "--int32Flag=-2147483648",
    nullptr
  };

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag values.
  EXPECT_EQ(-2147483647 - 1, int32Flag.value());
  EXPECT_EQ(-9223372036854775807L - 1L, int64Flag.value());

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, validMaxIntegersParseCorrectly) {
  Flag<int32> int32Flag("int32Flag", "An int32 number");
  Flag<int64> int64Flag("int64Flag", "An int64 number");

  int argc = 3;
  const char* argv[] = {
    "App", "--int64Flag=9223372036854775807", "--int32Flag=2147483647",
    nullptr
  };

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag values.
  EXPECT_EQ(2147483647, int32Flag.value());
  EXPECT_EQ(9223372036854775807, int64Flag.value());

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, tooSmallNumberValuesFailValidation) {
  Flag<int32> int32Flag("int32Flag", "An int32 number");

  int argc = 2;
  const char* argv[] = {"App", "--int32Flag=-2147483649", nullptr};

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for out-of-range int32 value.
  EXPECT_EQ(
      "Invalid value for flag --int32Flag: -2147483649."
          " Must be an int32 number.\n",
      output());
}


TEST_F(FlagTest, tooLargeNumberValuesFailValidation) {
  Flag<float> floatFlag("floatFlag", "A float number");

  static_assert(sizeof(float) == 4, "float isn't 32 bits");
  string tooBigForFloat = "10000000000000000000000000000000000000000";
  string floatFlagValue = "--floatFlag=" + tooBigForFloat;

  int argc = 2;
  const char* argv[] = {"App", floatFlagValue.c_str(), nullptr};

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for out-of-range float value.
  EXPECT_EQ(
      "Invalid value for flag --floatFlag: " + tooBigForFloat + "."
          " Must be a finite float number.\n",
      output());
}


TEST_F(FlagTest, nonNumericValueFailsValidation) {
  Flag<int64> int64Flag("int64Flag", "An int64 number");

  int argc = 2;
  const char* argv[] = {"App", "--int64Flag=deadbeef", nullptr};

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for out-of-range float value.
  EXPECT_EQ(
      "Invalid value for flag --int64Flag: deadbeef."
          " Must be an int64 number.\n",
      output());
}


TEST_F(FlagTest, emptyStringAndWhitespaceParseCorrectly) {
  Flag<string> flag1("flag1", "First string flag");
  Flag<string> flag2("flag2", "Second string flag");

  int argc = 3;
  const char* argv[] = {"App", "--flag1=", "--flag2=     ", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag values.
  EXPECT_EQ("", flag1.value());
  EXPECT_EQ("     ", flag2.value());

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, parsesValidatedValueCorrectly) {
  Flag<int32> nonNegativeFlag("nonNegative", "A non-negative number",
                              Validators<int32>::greaterOrEqual(0));

  int argc = 2;
  const char* argv[] = {"App", "--nonNegative=0", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag value.
  EXPECT_EQ(0, nonNegativeFlag.value());

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, failsIfValueValidationFails) {
  Flag<int64> starRatingFlag("starRating", "A star rating in [1, 5]",
                             Validators<int64>::greater(0),
                             Validators<int64>::lessOrEqual(5));

  int argc = 2;
  const char* argv[] = {"App", "--starRating=6", nullptr};

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for failing custom validators.
  EXPECT_EQ(
      "Invalid value for flag --starRating: 6."
          " Must be less than or equal to 5.\n",
      output());
}


TEST_F(FlagTest, defaultValueDoesNotHaveToPassValidators) {
  // Default flag to 0, which can't be specified from the command-line.
  Flag<int32> starRatingFlag("starRating", "A star rating in [1, 5]", 0,
                             Validators<int32>::greater(0),
                             Validators<int32>::lessOrEqual(5));

  // Don't specify any flag values from the command-line.
  int argc = 1;
  const char* argv[] = {"App", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag value.
  ASSERT_TRUE(starRatingFlag.hasValue());
  EXPECT_EQ(0, starRatingFlag.value());

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, canOverrideDefaultValueIfItPassesValidators) {
  // Default flag to 0, which can't be specified from the command-line.
  Flag<float> positiveFlag("positive", "A positive number", -1.0F,
                           Validators<float>::greater(0.0F));

  int argc = 2;
  const char* argv[] = {"App", "--positive=0.000001", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag value.
  ASSERT_TRUE(positiveFlag.hasValue());
  EXPECT_NEAR(0.000001F, positiveFlag.value(), FLOAT_EPSILON);

  // Verify flag default value is -1 (which wouldn't validate).
  ASSERT_TRUE(positiveFlag.hasDefaultValue());
  EXPECT_NEAR(-1.0F, positiveFlag.defaultValue(), FLOAT_EPSILON);

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, failsIfRequiredValueValidationFails) {
  Flag<string> nameFlag("name", "Your name (at most 16 characters)",
                        FlagRequired::YES,
                        Validators<string>::sizeLessOrEqual(16));

  int argc = 2;
  const char* argv[] = {
    "App",
    "--name=Tikki Tikki Tembo-no Sa Rembo-chari Bari Ruchi-pip Peri Pembo",
    nullptr
  };

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for failing custom validators.
  EXPECT_EQ(
      "Invalid value for flag --name:"
          " Tikki Tikki Tembo-no Sa Rembo-chari Bari Ruchi-pip Peri Pembo."
          " Size/length must be less than or equal to 16.\n",
      output());
}


TEST_F(FlagTest, parsesRequiredValidatedValueCorrectly) {
  Flag<string> usernameFlag("username", "Username between 3 and 15 characters",
                            FlagRequired::YES,
                            Validators<string>::sizeGreaterOrEqual(3),
                            Validators<string>::sizeLessOrEqual(15));

  int argc = 2;
  const char* argv[] = {"App", "--username=coolkid99", nullptr};

  // Should parse successfully.
  ASSERT_TRUE(flags::init(&argc, argv));

  // Verify flag value.
  EXPECT_EQ("coolkid99", usernameFlag.value());

  // No errors should have been output.
  EXPECT_EQ("", output());
}


TEST_F(FlagTest, initFailsIfMissingRequiredFlags) {
  Flag<string> requiredFlag1("requiredFlag1", "First required flag",
                             FlagRequired::YES);
  Flag<string> requiredFlag2("requiredFlag2", "Second required flag",
                             FlagRequired::YES);
  Flag<string> requiredFlag3("requiredFlag3", "Third required flag",
                             FlagRequired::YES);
  Flag<string> requiredFlag4("requiredFlag4", "Fourth required flag",
                             FlagRequired::YES);

  int argc = 3;
  const char* argv[] =
      {"App", "--requiredFlag1=A", "--requiredFlag3=C", nullptr};

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for missing required flags.
  EXPECT_EQ(
      "Missing required command-line flag --requiredFlag2.\n"
          "Missing required command-line flag --requiredFlag4.\n",
      output());
}


TEST_F(FlagTest, initFailsIfGivenUnrecognizedFlagName) {
  Flag<bool> flag1("flag1", "First flag");
  Flag<bool> flag2("flag2", "Second flag");
  Flag<bool> flag3("flag3", "Third flag");

  int argc = 3;
  const char* argv[] = {"App", "--flag2", "--flag33=false" /* typo */, nullptr};

  // Trying to parse flags should fail.
  EXPECT_FALSE(flags::init(&argc, argv));

  // Verify validation error for unrecognized flag name.
  EXPECT_EQ("Unrecognized command-line flag: --flag33\n", output());
}


TEST_F(FlagTest, printUsageNoFlags) {
  flags::printUsage("App", "first_arg second_arg", "Some extra notes.");

  EXPECT_EQ(
      "Usage: App first_arg second_arg\n"
          "  Some extra notes.\n",
      output());
}


TEST_F(FlagTest, printUsageOneOptionalFlag) {
  Flag<bool> fastModeFlag("fastMode", "True to enable fast mode");

  flags::printUsage("App");

  EXPECT_EQ(
      "Usage: App [flags]\n"
          "\n"
          "Optional flags:\n"
          "  --fastMode: True to enable fast mode\n",
      output());
}


TEST_F(FlagTest, printUsageManyFlags) {
  Flag<bool> strictModeFlag("strictMode", "True to enable strict mode", false);
  Flag<bool> fastModeFlag("fastMode", "True to enable fast mode");
  Flag<string> usernameFlag("username", "Username for login", FlagRequired::YES);
  Flag<string> authFileFlag("authFile", "Path to file with auth info",
                            FlagRequired::YES);
  Flag<int32> favoriteNumberFlag("favoriteNumber", "Favorite positive int", 42,
                                 Validators<int32>::greater(0));

  flags::printUsage("App", "pos_arg1 pos_arg2");

  EXPECT_EQ(
      "Usage: App flags pos_arg1 pos_arg2\n"
          "\n"
          "Required flags:\n"
          "  --authFile: Path to file with auth info\n"
          "  --username: Username for login\n"
          "\n"
          "Optional flags:\n"
          "  --fastMode: True to enable fast mode\n"
          "  --favoriteNumber: Favorite positive int (default: 42)\n"
          "  --strictMode: True to enable strict mode (default: false)\n",
      output());
}


TEST_F(FlagTest, printUsageRequiredFlagsAndNotes) {
  Flag<bool> required1("required1", "First required flag", FlagRequired::YES);
  Flag<bool> required2("required2", "Second required flag", FlagRequired::YES);

  flags::printUsage("App", "", "Some usage notes");

  EXPECT_EQ(
      "Usage: App flags\n"
          "  Some usage notes\n"
          "\n"
          "Required flags:\n"
          "  --required1: First required flag\n"
          "  --required2: Second required flag\n",
      output());
}


}  // namespace
