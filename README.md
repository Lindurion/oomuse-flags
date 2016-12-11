# oomuse-flags
Simple C++ library for parsing and validating command-line flags.

Default supported flag types are `bool`, `int32`, `int64`, `float`, `double`, and `string`. See more details below about supporting custom types.


## Sample Usage

From the command line:
```
$ your_program_name --verbose --username=slyfox31 --retry_limit=3
```

From your code:
```C++
using oomuse::Flag;
using oomuse::FlagRequired;
using oomuse::Validators;
namspace flags = oomuse::flags;

Flag<int32> retryLimit("retry_limit", "Max # of times to retry",
                       Validators<int32>::greaterOrEqual(1));
Flag<string> username("username", "Username between 3 and 15 characters",
                      FlagRequired::YES,
                      Validators<string>::sizeGreaterOrEqual(3),
                      Validators<string>::sizeLessOrEqual(15));
Flag<bool> verbose("verbose", "Set --verbose to print extra info", false);
Flag<string> specialGreeting("special_greeting", "An optional message",
                             "Hello, this is the default greeting.");

int main(int argc, char* argv[]) {
  // Initialize and validate all command-line flags. If any required flags
  // are missing or any validators fail, this will print out program usage
  // and exit.
  flags::initOrPrintUsageAndDie(
      &argc, argv, "your_program_name", "positional_arg1 positional_arg2",
      "Special notes on usage go here.");

  // Parse any positional command-line args (flags lib updates argc and
  // argv[], removing all parsed flags).
  if (argc >= 3) {
    string positionalArg1 = argv[1];
    string positionalArg2 = argv[2];
    ...
  }

  // Check whether optional flags are set with hasValue().
  if (retryLimit.hasValue()) {
    int32 theRetryLimit = retryLimit.value();
    ...
  }

  // Required flags are guaranteed to be set after initialization.
  string theUsername = username.value();
  ...

  // Flags with a default value will return the default if no user-provided
  // value was passed on the command line.
  bool shouldPrintVerboseInfo = verbose.value();
  string theSpecialGreeting = specialGreeting.value();
  ...
}
```


## Using oomuse-flags in your own projects

See [oomuse-core README](https://github.com/Lindurion/oomuse-core) for build & test instructions. In addition to those, just add a [conan](http://docs.conan.io/en/latest/) requirement on `oomuse-flags/0.1.0@lindurion/stable`.


## Custom Flag Types

To support other types, you can provide a specialized implementation of `Flag<YourType>::parseValidateAndSet(const std::string& textValue)`. See [Flag.h](https://github.com/Lindurion/oomuse-flags/blob/master/include/oomuse/flags/Flag.h) to reference the default implementations.


## License

This open source library is free to use in your own projects, released under the [Apache License Version 2.0](https://github.com/Lindurion/oomuse-core/blob/master/LICENSE).
