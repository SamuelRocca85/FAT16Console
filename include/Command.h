#pragma once
#include "FileSystem.h"
#include <iostream>
#include <string>
#include <vector>

typedef void (*function)(FileSystem &);

using std::string;
using std::vector;

// Crear una clase que guarde un map de <string, Command> donde el string es el
// nombre del comando
// Crear una funcion que cree todos los comandos

struct Flag {
  string name;

  Flag(string _name) : name(_name) {}
};

struct Command {
  friend std::ostream &operator<<(std::ostream &out, const Command &com) {
    out << "--- Comando " << com.name << " ---\n";
    for (Flag flag : com.flags) {
      out << "Flag: " << flag.name << "\n";
    }
    return out;
  }
  vector<Flag> flags;
  string name;
  function callback;

  Command(string _name) : name(_name) {}
  Command(string _name, function _callback)
      : name(_name), callback(_callback) {}
  void addFlag(string name) { flags.push_back(Flag{name}); }
  void checkFlag(string flag) {
    // Check if flag is valid
  }
};
