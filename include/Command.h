#pragma once
#include "FileSystem.h"
#include <iostream>
#include <string>
#include <vector>

#define NULL_STRING " "

struct Param;

typedef void (*function)(FileSystem &, Param &param);

using std::string;
using std::vector;

struct Param {
  string name;
  string value;

  Param() : value(NULL_STRING), name(NULL_STRING) {}
  Param(string _value) : name(NULL_STRING), value(_value) {}
  Param(string _name, string _value) : name(_name), value(_value) {}
};

struct Command {
  friend std::ostream &operator<<(std::ostream &out, const Command &com) {
    out << "--- Comando " << com.name << " ---\n";
    return out;
  }
  string name;
  Param param;
  function callback;

  Command(string _name) : name(_name) {}
  Command(string _name, function _callback)
      : name(_name), callback(_callback) {}
};
