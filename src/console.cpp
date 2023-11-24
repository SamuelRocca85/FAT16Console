#include "Console.h"
#include "FileSystem.h"
#include <iostream>
#include <sstream>

void lsCallback(FileSystem &fs) { fs.listFiles(); }
void cdCallback(FileSystem &fs) {
  // std::cout << "Hola desde cd\n";
  fs.changeDir("PRUEBA     ");
}
void catCallback(FileSystem &fs) { std::cout << "Hola desde cat\n"; }
void mkdirCallback(FileSystem &fs) {
  // std::cout << "Hola desde mkdir\n";
  fs.makeDir("Hola");
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
    std::cout << ">> ";
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
    commands.at(command).callback(fs);
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
