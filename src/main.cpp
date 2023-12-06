#include "Console.h"
#include "FileSystem.h"
#include <string>

using std::string;

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "Mal uso: ./fatconsole [archivo de imagen fat]\n";
    return 1;
  }

  Disk disk(argv[1]);
  FileSystem fs(disk);
  Console console(fs);

  console.start();
  return 0;
}
