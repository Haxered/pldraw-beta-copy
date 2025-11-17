#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "interpreter.hpp"
#include "interpreter_semantic_error.hpp"

#include <exception>

//
// postlisp - postlisp.cpp (GUIDANCE)
// -----------------------------------------------------------------------------
// Implement the three modes in main() per the spec:
//
//   • REPL (no args):
//       - Prompt exactly: "postlisp>"
//       - Read lines until EOF; ignore empty lines
//       - parse() each line; on false → print "Error: parse error"
//       - On success → eval() and print the result via operator<<
//       - On InterpreterSemanticError → print "Error: <msg>" and reset env
//
//   • -e "<program>" (two args):
//       - Parse/eval the quoted program once
//       - Success: print "(value)" and exit 0
//       - Failure: print "Error: ..." and exit non-zero
//
//   • <file.slp> (one arg):
//       - Read the file, parse/eval once
//       - Same output/exit rules as -e mode
//
// • Do NOT print extra whitespace or banners: Keep output minimal and predictable. No welcome messages, extra newlines, tabs, headers, decorative lines, etc. The autograder often does exact string matches.
// • All errors start with Error: .”: When anything goes wrong (parse fail, semantic error, file open fail, bad args), print one line that begins exactly with Error: followed by a short message.
//

static void prompt() {
    // REPL prompt must match the spec exactly (no trailing spaces besides single one).
    std::cout << "\npostlisp> ";
}

static void error(const std::string &err_str) {
    // All error messages funnel through here for consistent formatting.
    std::cerr << "Error: " << err_str << std::endl;
}

static bool parse_and_eval(Interpreter &interp, std::istream &in, Expression &out) {
    if (!interp.parse(in)) {
        return false;
    }
    out = interp.eval();
    return true;
}

static int run_single_expression_mode(const std::string &filename) {
    Interpreter interp;
    std::istringstream iss{filename};
    try {
        Expression result;
        if (!parse_and_eval(interp, iss, result)) {
            error("parse error");
            return EXIT_FAILURE;
        }
        std::cout << result << std::endl;
        return EXIT_SUCCESS;
    } catch (const InterpreterSemanticError &e) {
        error(e.what());
        return EXIT_FAILURE;
    } catch (const std::exception &e) {
        error(e.what());
        return EXIT_FAILURE;
    }
}

static int run_file_mode(const std::string &filename) {
    std::ifstream infile(filename);
    if (!infile.good()) {
        error("could not open file");
        return EXIT_FAILURE;
    }

    Interpreter interp;
    try {
        Expression result;
        if (!parse_and_eval(interp, infile, result)) {
            error("parse error");
            return EXIT_FAILURE;
        }
        std::cout << result << std::endl;
        return EXIT_SUCCESS;
    } catch (const InterpreterSemanticError &e) {
        error(e.what());
        return EXIT_FAILURE;
    } catch (const std::exception &e) {
        error(e.what());
        return EXIT_FAILURE;
    }
}

static int run_interactive_mode() {
    Interpreter interp;

    // initial prompt
    prompt();

    std::string line;
    while (true) {
        if (!std::getline(std::cin, line)) {
            return EXIT_SUCCESS;
        }

        // ignore empty/whitespace lines
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
            prompt();
            continue;
        }

        std::istringstream iss(line);
        try {
            Expression result;
            if (!parse_and_eval(interp, iss, result)) {
                error("parse error");
            } else {
                std::cout << result << std::endl;
            }
        } catch (const InterpreterSemanticError &e) {
            error(e.what());
            // reset env on semantic error
            interp = Interpreter();
        } catch (const std::exception &e) {
            // any other error: throw and reset
            error(e.what());
            interp = Interpreter();
        }

        prompt();
    }
}

// A REPL is a repeated read-eval-print loop
int main(int argc, char *argv[]) {
    // (M6): Implement command-line handling and modes per the spec.
    //   - REPL (no args)
    //   - -e "<program>"
    //   - <file.slp>
    //
    // IMPORTANT:
    //   - Do not call eval() unless parse() returned true.
    //   - On parse failure:       Error: parse error
    //   - On semantic exception:  Error: <message>
    //   - In REPL, reset the environment after a semantic error.
    //   - Exit codes:
    //       * success   → EXIT_SUCCESS
    //       * any error → EXIT_FAILURE

    // Interactive REPL mode
    if (argc == 1) {
        return run_interactive_mode();
    }

    // Single Expression mode
    if (argc == 3 && std::string(argv[1]) == "-e") {
        return run_single_expression_mode(std::string(argv[2]));
    }

    // File mode
    if (argc == 2) {
        return run_file_mode(argv[1]);
    }

    // otherwise, throw invalid args
    error("invalid arguments");
    return EXIT_FAILURE;
}