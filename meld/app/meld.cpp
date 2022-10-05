#include "boost/program_options.hpp"
#include "libjsonnet++.h"
#include "meld/app/run_meld.hpp"
#include "meld/app/version.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace std::string_literals;
using namespace boost;
namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
  std::ostringstream descstr;
  descstr << "\nUsage: " << std::filesystem::path(argv[0]).filename().native()
          << " -c <config-file> [other-options]\n\n"
          << "Basic options";
  bpo::options_description desc{
    descstr.str()}; //{"meld is a framework to explore processing DUNE data"};

  std::string config_file;
  // clang-format off
  desc.add_options()
    ("help,h", "Produce help message")
    ("version", ("Print meld version ("s + meld::version() + ")").c_str())
    ("config,c", bpo::value<std::string>(&config_file), "Configuration file.");
  // clang-format on

  // Parse the command line.
  bpo::variables_map vm;
  try {
    bpo::store(
      bpo::command_line_parser(argc, argv)
        .options(desc)
        .style(bpo::command_line_style::default_style & ~bpo::command_line_style::allow_guessing)
        .run(),
      vm);
    bpo::notify(vm);
  }
  catch (bpo::error const& e) {
    std::cerr << "Exception from command line processing in " << argv[0] << ": " << e.what()
              << '\n';
    return 1;
  }

  if (vm.count("help")) {
    std::cout << desc << '\n';
    return 0;
  }

  if (vm.count("version")) {
    std::cout << "meld " << meld::version() << '\n';
    return 0;
  }

  if (not vm.count("config")) {
    std::cerr << "Error: No configuration file given.\n";
    return 2;
  }

  jsonnet::Jsonnet j;
  if (not j.init()) {
    std::cerr << "Error: Could not initialize Jsonnet parser.\n";
    return 2;
  }

  std::cout << "Using configuration file: " << config_file << '\n';

  std::string config_str;
  auto rc = j.evaluateFile(config_file, &config_str);
  if (not rc) {
    std::cerr << j.lastError() << '\n';
    return 2;
  }

  meld::run_it(json::parse(config_str));
}
