#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <utility>
#include <limits>
#include <list>
#include <string>
#include <sstream>
#include <limits>
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

inline void batch_process(
    map<unsigned int, pair<string, shared_ptr<vector<double>>>> &datamap,
    int &numthread, vector<unique_ptr<SegAlg>> &algvec,
    map<unsigned int, pair<string, vector<Segment>>> &res) {
  vector<thread> threadvec;
  mutex read_mutex, write_mutex;
  auto iter = datamap.begin();

  for (int i = 0; i < numthread; i++) {
    threadvec.emplace_back(thread([&](int idx) -> void {
      decltype(iter) localiter;
      while (true) {
        {
          lock_guard<mutex> guard(read_mutex);
          if (iter == datamap.end())
            return;
          localiter = iter;
          iter++;
        }
        algvec[idx]->set_string(localiter->second.second);
        algvec[idx]->run();
        auto tmpres = algvec[idx]->get_result();
        {
          lock_guard<mutex> guard(write_mutex);
          res[localiter->first] = pair<string, vector<Segment>>(
              localiter->second.first, *(algvec[idx]->get_result()));
        }
      }
    }, i));
  }
  for (auto &t : threadvec) {
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

  if (batchsize < 1)
    LOG_ERROR("batchsize is too small");
  if (batchsize >= numeric_limits<unsigned int>::max())
    LOG_ERROR("batchsize is too big");

  if (numthread < 1)
    LOG_ERROR("nthread should be >0");

  if (numseg < 1)
    LOG_ERROR("numseg should be >0");

  /* prepare algorithm */
  SegAlgFactory factory;
  vector<unique_ptr<SegAlg>> algvec;
  algvec.reserve(numthread);
  for (int i = 0; i < numthread; i++) {
    algvec.emplace_back(factory.make(algtype));
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
  map<unsigned int, pair<string, shared_ptr<vector<double>>>> datamap;
  bool isfirst = true;
  int length = 0;
  map<unsigned int, pair<string, vector<Segment>>> result;
  unsigned int idxbatch = 0;
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
    if (id.size() == 0 or id[0] == '#')
      continue;

    // load the numbers;
    shared_ptr<vector<double>> seq(new vector<double>);
    seq->reserve(length - 1);
    while (!ss.eof()) {
      ss >> num;
      seq->push_back(num);
    }
    datamap[idxbatch] = pair<string, shared_ptr<vector<double>>>(id, seq);
    idxbatch++;

    // process the batch
    if (datamap.size() >= batchsize or infileobj.eof()) {
      batch_process(datamap, numthread, algvec, result);
      /* clear data */
      datamap.clear();
      /* output result */
      for (auto &kv : result) {
        outfileobj << kv.second.first << "\t[";
        for (auto &seg : kv.second.second) {
          outfileobj << '(' << seg.headIndex << ',' << seg.tailIndex << ','
                     << seg.a << ',' << seg.b << ',' << seg.c << ',' << seg.loss
                     << ',' << seg.order << "),";
        }
        outfileobj << "]\n";
      }
      outfileobj.flush();
      result.clear();
      idxbatch = 0;
    }
  }

  return 0;
}
