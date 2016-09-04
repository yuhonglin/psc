#include <fstream>
#include <iostream>
#include <vector>
#include <limits>
#include <list>
#include <string>
#include <sstream>

#include <cstdlib>
#include <cmath>

#ifdef DEBUG
#include <fenv.h>
#endif

#include "option.hpp"
#include "logger.hpp"
#include "SegAlgFactory.hpp"
#include "SegAlg.hpp"

using namespace std;

int main(int argc, char *argv[]) {

#ifdef DEBUG
  /**
   * function (log, pow) in cmath may lead to FE_INEXACT exceptions
   */
  feenableexcept(-1 xor FE_INEXACT);
#endif
  
  // LOGGER->enable_exception = false;

  // /* parse options */
  // dms::Option opt;

  // opt.add_string("-alg", "algorithm type", "dp",
  //                set<string>({ "dp", "topdown" }));

  // opt.add_string("-fit", "fitting type", "powerabc",
  //                set<string>({ "powerabc", "abline", "const" }));

  // opt.add_string("-i", "input file");

  // opt.add_string("-o", "output file");

  // opt.add_string("-nthread", "number of threads to use", 1);

  // switch (opt.parse(argc, argv)) {
  // case dms::Option::PARSE_HELP: { return 0; }
  // case dms::Option::PARSE_ERROR: {
  //   LOG_ERROR("option parsing error, use --help to for usage");
  //   return 1;
  // }
  // default:
  //   break;
  // }

  // string algtype = opt.get_string("-alg");
  // string fittype = opt.get_string("-fit");
  // string infile = opt.get_string("-i");
  // string outfile = opt.get_string("-o");
  // int numthread = opt.get_int("-nthread");

  // ifstream infileobj(infile);
  // for (string line; getline(infileobj, line);) {
  //   cout << line << endl;
  // }

  return 0;
}
