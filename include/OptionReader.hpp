#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Parameter.hpp"

using std::string;
using std::vector;

namespace gethin {
class OptionReader {
 public:
  OptionReader() {}
  OptionReader(vector<Parameter *> opts) : m_opts(opts) {}
  OptionReader &with(Parameter *p) {
    m_opts.push_back(p);
    return *this;
  }
  OptionReader &with(vector<Parameter *> ps) {
    m_opts.insert(end(m_opts), begin(ps), end(ps));
    return *this;
  }

  void read(int argc, char *argv[]) {
    vector<string> args(argv, argv + argc);
    std::cout << "Before: " << args.size() << std::endl;
    preProcessInput(args);

    std::cout << "After: " << args.size() << std::endl;
    processInput(args);
  }

 private:
  vector<Parameter *> m_opts;

  static void preProcessInput(vector<string> &args) {
    for (int i = 0; i < args.size(); ++i) {
      string val = args[i];

      size_t p = val.find('=');
      if (p != string::npos) {
        args[i] = val.substr(0, p);
        size_t secondLength = val.size() - p - 1;
        string second = val.substr(p + 1, secondLength);
        second.erase(remove(second.begin(), second.end(), '"'), second.end());
        args.insert(args.begin() + ++i, second);
      }

      else if (val[0] == '-' && val[1] != '-' && val.length() > 2) {
        args.erase(args.begin() + i--);
        for (size_t j = 1; j < val.size(); j++) {
          string extracted = "-";
          extracted += string(1, val[j]);
          args.insert(args.begin() + ++i, extracted);
        }
      }
    }
  }

  void processInput(vector<string> args) {
    for (size_t i = 0; i < args.size(); i++) {
      string str = args[i];
      if (isUsageOpt(str)) {
        printUsage();
        return;
      } else {
        parseInput(args, i);
      }
    }
  }

  void printUsage() {
    std::cout << "Usage: <program> [OPTION]" << std::endl;
    for (Parameter *p : m_opts) {
      std::cout << p->usage() << std::endl;
    }
  }

  static bool isUsageOpt(string str) {
    if (str == "-h" || str == "--help" || str == "-u" || str == "--usage") {
      return true;
    }
    return false;
  }

  void parseInput(vector<string> args, size_t i) {
    string val(args[i]);
    std::cout << "Val: " << val << std::endl;
    try {
      if (isOption(val)) {
        string data = "";
        size_t nextOptIdx = findNextOptIdx(args, i);
        std::cout << "\t" << i << " : " << nextOptIdx << " : " << nextOptIdx - i
                  << std::endl;
        if (nextOptIdx - i > 2) {
          throw std::invalid_argument("Two arguments given instead of one.");
        } else if (nextOptIdx - i == 2) {
          data = args[i + 1];
        }
        handleOpt(val, data);
      }
    } catch (const std::invalid_argument &e) {
      throw std::invalid_argument("Failed to parse option: '" + val +
                                  "': " + e.what());
    }
  }
  static bool isOption(string val) { return val[0] == '/' || val[0] == '-'; }

  static size_t findNextOptIdx(vector<string> args, size_t currentIdx) {
    for (size_t i = currentIdx + 1; i < args.size(); i++) {
      string candidate(args[i]);
      if (isOption(candidate)) {
        std::cout << "\t" << candidate << "@" << i << std::endl;
        return i;
      }
    }
    return args.size();
  }

  void handleOpt(const string &enteredOpt, const string &arg) {
    bool found = false;
    for (Parameter *opt : m_opts) {
      if (enteredOpt.substr(2, string::npos) == opt->longOpt() ||
          (enteredOpt.length() == 2 && enteredOpt[1] == opt->shortOpt())) {
        std::cout << "\tentered:\t'" << enteredOpt << "'" << std::endl;
        std::cout << "\tmatched:\t'" << opt->longOpt() << "' or '"
                  << opt->shortOpt() << "'" << std::endl;
        std::cout << "\tdata:\t'" << arg << "'" << std::endl;
        opt->set(arg);
        found = true;
      }
    }
    if (!found) {
      throw std::invalid_argument("Unknown option: '" + enteredOpt + "'");
    }
  }
};
}  // namespace gethin