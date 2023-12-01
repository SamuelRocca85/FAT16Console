#pragma once
#include "Command.h"
#include "FileSystem.h"
#include <map>
#include <string>

using std::map;
using std::string;

class Console {

  friend std::ostream &operator<<(std::ostream &out, const Console &con) {
    out << "--- Commands ---\n";
    for (const auto &[key, value] : con.commands) {
      out << value;
    }
    return out;
  }

private:
  FileSystem &fs;
  map<string, Command> commands;

public:
  Console(FileSystem &_fs);
  void start();
  vector<string> splitCommand(string command);
};
