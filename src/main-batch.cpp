#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <limits>
#include <list>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>

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

inline void batch_process(const vector<shared_ptr<vector<double>>> &data,
                          const vector<string> &id, const int &numthread,
                          vector<unique_ptr<SegAlg>> &algvec,
                          map<string, shared_ptr<vector<Segment>>> &res) {
  vector<thread> threadvec;
  mutex read_mutex, write_mutex;
  auto iter = data.begin();
  for (int i = 0; i < numthread; i++) {
    threadvec.push_back(thread([&](int idx) -> void {
      shared_ptr<vector<double>> str;
      while (true) {
        {
          lock_guard<mutex> guard(read_mutex);
          if (iter == data.end())
            return;
          str = *iter;
          iter++;
        }
        algvec[idx]->set_string(str);
        algvec[idx]->run();
        {
          lock_guard<mutex> guard(write_mutex);
          res[id[idx]] = algvec[idx]->get_result();
        }
      }
    }, i));
  }
  for (auto& t : threadvec) {
    t.join();
  }
}

int main(int argc, char *argv[]) {

#ifdef DEBUG
  /**
   * function (log, pow) in cmath may lead to FE_INEXACT exceptions
   */
  feenableexcept(-1 xor FE_INEXACT);
#endif

  LOGGER->enable_exception = false;

  /* parse options */
  dms::Option opt;

  opt.add_string("-alg", "algorithm type", "dp",
                 set<string>({"dp", "topdown"}));
  opt.add_string("-fit", "fitting type", "powerabc",
                 set<string>({"powerabc", "abline", "const"}));
  opt.add_string("-i", "input file");
  opt.add_string("-o", "output file");
  opt.add_int("-nthread", "number of threads to use", 1);
  opt.add_int("-batchsize", "max number of segment to process in one batch",
              1e3);
  opt.add_int("-numseg", "number of segment", 1);

  switch (opt.parse(argc, argv)) {
  case dms::Option::PARSE_HELP: {
    return 0;
  }
  case dms::Option::PARSE_ERROR: {
    LOG_ERROR("option parsing error, use --help to for usage");
    return 1;
  }
  default:
    break;
  }

  string algtype = opt.get_string("-alg");
  string fittype = opt.get_string("-fit");
  string infile = opt.get_string("-i");
  string outfile = opt.get_string("-o");
  int numthread = opt.get_int("-nthread");
  int batchsize = opt.get_int("-batchsize");
  int numseg = opt.get_int("-numseg");

  /* prepare algorithm */
  SegAlgFactory factory;
  vector<unique_ptr<SegAlg>> algvec(numthread);
  for (int i = 0; i < numthread; i++) {
    algvec[i] = factory.make(algtype);
    algvec[i]->set_parameter("numseg", numseg);
    algvec[i]->set_fitalg(fittype);
  }

  /* open files */
  ifstream infileobj(infile);
  ofstream outfileobj(outfile);
  if (infileobj.fail()) {
    LOG_ERROR("Cannot open input file.");
  }
  if (outfileobj.fail()) {
    LOG_ERROR("Cannot open output file.");
  }

  /* load and process input file */
  int idlength = 0;
  string id;
  double num = 0.0;
  vector<shared_ptr<vector<double>>> datavec(batchsize);
  vector<string> idvec(batchsize);
  bool isfirst = true;
  int length = 0;
  map<string, shared_ptr<vector<Segment>>> result;

  for (string line; getline(infileobj, line);) {
    // prepare for ss to read
    // and at the same time count sequence length
    length = 0;
    for (unsigned int i = 0; i < line.size(); i++) {
      if (line[i] == ',' or line[i] == '\t') {
        line[i] = ' ';
        isfirst = true;
      } else {
        if (isfirst) {
          length++;
          isfirst = false;
        }
      }
    }
    if (length < 2)
      continue;

    // load the id of data
    stringstream ss(line);
    ss >> id;

    // allow/skip comments
    if (id[0] == '#')
      continue;

    // load the numbers;
    vector<double> seq(length - 1);
    while (!ss.eof()) {
      ss >> num;
      seq.push_back(num);
    }
    datavec.push_back(make_shared<vector<double>>(seq));
  }
  idvec.push_back(id);

  // process the batch
  if (datavec.size() >= batchsize or infileobj.eof()) {
    batch_process(datavec, idvec, numthread, algvec, result);
    /* clear data */
    datavec.clear();
    idvec.clear();
    /* output result */
    for (auto &kv : result) {
      outfileobj << kv.first << "\t[";
      for (auto &seg : (*kv.second)) {
        outfileobj << '(' << seg.headIndex << ',' << seg.tailIndex << ','
                   << seg.a << ',' << seg.b << ',' << seg.c << ',' << seg.loss
                   << ',' << seg.order << "),";
      }
      outfileobj << "]\n";
    }
    outfileobj.flush();
  }

  return 0;
}
