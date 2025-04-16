#include <iostream>
#include <vector>

#include <plog/Formatters/TxtFormatter.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Log.h>

int main(int argc, char const* argv[])
{
  plog::init<plog::TxtFormatter>(plog::debug, plog::streamStdOut);
  auto vec = std::vector<int>{1, 2, 3, 4, 0, 6, 6, 7};
  PLOG_INFO << vec;
  return 0;
}
