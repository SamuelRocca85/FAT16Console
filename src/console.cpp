#include "Console.h"
#include "FileSystem.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

void lsCallback(FileSystem &fs, string param) { fs.listFiles(); }
void cdCallback(FileSystem &fs, string param) {
  // std::cout << "Hola desde cd\n";
  if (param.length() < 11) {
    param.append(11 - param.length(), ' ');
  }
  fs.changeDir(param.c_str());
}
void catCallback(FileSystem &fs, string param) {
  // param = fs.parseFileName(param);
  // fs.catFile(param);
  fs.print();
}
void mkdirCallback(FileSystem &fs, string param) {
  std::transform(param.begin(), param.end(), param.begin(), ::toupper);
  if (param.length() < 11) {
    param.append(11 - param.length(), ' ');
  }
  fs.makeDir(param.c_str());
}

Console::Console(FileSystem &_fs) : fs(_fs) {
  Command ls("ls", lsCallback);
  Command cd("cd", cdCallback);
  Command cat("cat", catCallback);
  Command mkdir("mkdir", mkdirCallback);

  commands.insert({"ls", ls});
  commands.insert({"cd", cd});
  commands.insert({"cat", cat});
  commands.insert({"mkdir", mkdir});
}

void Console::start() {
  string input = "";

  while (input != "exit") {
    std::cout << fs.getPath() << " >> ";
    std::getline(std::cin, input);

    vector<string> split = splitCommand(input);
    string command = split[0];
    if (command == "exit") {
      std::cout << "Saliendo de la consola...\n";
      return;
    } else if (commands.find(command) == commands.end()) {
      std::cout << "Comando invalido!\n";
      continue;
    }
    string value = "";
    for (int i = 1; i < split.size(); i++) {
      value = split[i];
    }
    commands.at(command).callback(fs, value);
  }
}

vector<string> Console::splitCommand(string command) {
  string s;

  std::stringstream ss(command);

  vector<string> v;

  while (getline(ss, s, ' '))
    v.push_back(s);

  return v;
}
