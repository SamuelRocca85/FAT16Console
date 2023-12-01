#include "Console.h"
#include "FileSystem.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

void lsCallback(FileSystem &fs, Param &param) { fs.listFiles(); }
void cdCallback(FileSystem &fs, Param &param) {
  // std::cout << "Hola desde cd\n";
  string value = param.value;
  if (value.length() < 11) {
    value.append(11 - value.length(), ' ');
  }
  fs.changeDir(value.c_str());
}
void catCallback(FileSystem &fs, Param &param) {
  string value = param.value;
  std::transform(value.begin(), value.end(), value.begin(), ::toupper);

  if (param.name.compare(NULL_STRING) == 0) {
    // Mostrar contenido de archivo
    fs.catFile(value.c_str());
    return;
  } else if (param.name == ">") {
    // Crear archivo
    fs.createFile(value.c_str());
    return;
  } else {
    printf("Uso incorecto de cat: parametro %s invalido\n",
           param.value.c_str());
    return;
  }
}
void mkdirCallback(FileSystem &fs, Param &param) {
  string value = param.value;
  std::transform(value.begin(), value.end(), value.begin(), ::toupper);
  if (value.length() < 11) {
    value.append(11 - value.length(), ' ');
  }
  fs.makeDir(value.c_str());
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
    Param param;
    if (split.size() == 2) {
      param.value = split[1];
    } else if (split.size() == 3) {
      param.name = split[1];
      param.value = split[2];
    }
    commands.at(command).callback(fs, param);
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
