#include <getopt.h>

#include "ASTDumper.h"
#include "Parser.h"

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char *argv[]) {
  int C = 0;
  int DumpAST = 0;

  opterr = 0;
  while (true) {
    static struct option long_options[] = {
        {"dump-ast", no_argument, &DumpAST, 1},
        {nullptr, 0, nullptr, 0},
    };

    int option_index = 0;
    C = getopt_long(argc, argv, "", long_options, &option_index);
    if (C == -1)
      break;

    switch (C) {
    // case 0: break;
    case 0:
      if (long_options[option_index].flag != nullptr) {
        // This option set a flag, do nothing else.
        break;
      }
      if (optarg != nullptr)
        printError("option {} with arg {}", long_options[option_index].name,
                   optarg);
      else
        printError("option {}", long_options[option_index].name);
      break;
    case '?': printError("Invalid option \"-{}\"", (char)optopt); exit(1);
    default: printError("Invalid option \"-{}\"", (char)C); exit(1);
    }
  }

  if (argv[optind] == nullptr) {
    printError("No input file");
    exit(1);
  }

  FILE *Stream = fopen(argv[optind], "r");
  if (Stream == nullptr) {
    printError("{}: \"{}\"", strerror(errno), argv[optind]);
    exit(2);
  }

  CompilationUnit Unit;

  Parser P(Stream);

  // Run the main "interpreter loop" now.
  P.Parse(Unit);

  fmt::println("DumpAST {}", DumpAST);
  if (DumpAST) {
    ASTDumper(stdout, Unit);
  }

  return 0;
}
