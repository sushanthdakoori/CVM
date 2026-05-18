#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"
#include "Serializer.h"
#include "VM.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// ── Helpers ──────────────────────────────────────────────────

static std::string replaceExtension(const std::string& path, const std::string& ext) {
    size_t dot = path.rfind('.');
    std::string base = (dot == std::string::npos) ? path : path.substr(0, dot);
    return base + ext;
}

// ── Compile .cvm → .bytecode ─────────────────────────────────

static void compileFile(const std::string& srcPath, bool debug) {
    std::ifstream file(srcPath);
    if (!file) { std::cerr << "Cannot open: " << srcPath << "\n"; return; }
    std::ostringstream ss; ss << file.rdbuf();

    try {
        Lexer lexer(ss.str());
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        auto ast = parser.parse();

        Compiler compiler;
        auto bytecode = compiler.compile(ast);

        if (debug) Compiler::disassemble(bytecode);

        std::string outPath = replaceExtension(srcPath, ".bytecode");
        Serializer::write(bytecode, outPath);
        std::cout << "Compiled -> " << outPath << "  (" << bytecode.size() << " instructions)\n";

    } catch (const std::exception& e) {
        std::cerr << "[Compile Error] " << e.what() << "\n";
    }
}

// ── Run .bytecode ─────────────────────────────────────────────

static void runFile(const std::string& path, bool debug) {
    try {
        std::vector<Instruction> bytecode;

        if (path.size() > 9 && path.substr(path.size() - 9) == ".bytecode") {
            // load pre-compiled bytecode
            bytecode = Serializer::read(path);
        } else {
            // compile .cvm on the fly and run directly (no file written)
            std::ifstream file(path);
            if (!file) { std::cerr << "Cannot open: " << path << "\n"; return; }
            std::ostringstream ss; ss << file.rdbuf();

            Lexer lexer(ss.str());
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();
            Compiler compiler;
            bytecode = compiler.compile(ast);
        }

        if (debug) Compiler::disassemble(bytecode);

        VM vm;
        vm.run(bytecode);

    } catch (const std::exception& e) {
        std::cerr << "[Runtime Error] " << e.what() << "\n";
    }
}

// ── REPL ──────────────────────────────────────────────────────

static void runRepl(bool debug) {
    std::cout << "CVM++ REPL  (type 'exit' to quit, 'debug' to toggle)\n";
    std::string line;
    VM vm; // one VM for the whole session — variables persist
    while (true) {
        std::cout << ">> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit") break;
        if (line == "debug") { debug = !debug; std::cout << "Debug " << (debug?"ON":"OFF") << "\n"; continue; }
        if (line.empty()) continue;
        try {
            Lexer lexer(line);
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();
            Compiler compiler;
            auto bytecode = compiler.compile(ast);
            if (debug) Compiler::disassemble(bytecode);
            vm.runLine(bytecode); // use runLine to keep state
        } catch (const std::exception& e) {
            std::cerr << "[Error] " << e.what() << "\n";
        }
    }
}

// ── Usage ─────────────────────────────────────────────────────

static void printUsage() {
    std::cout << "Usage:\n"
              << "  cvm compile <file.cvm> [--debug]   compile to .bytecode\n"
              << "  cvm run <file.cvm|file.bytecode> [--debug]   run a file\n"
              << "  cvm                                start REPL\n";
}

// ── Entry point ───────────────────────────────────────────────

int main(int argc, char* argv[]) {
    if (argc == 1) { runRepl(false); return 0; }

    std::string cmd = argv[1];
    bool debug = false;
    std::string filepath;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") debug = true;
        else filepath = arg;
    }

    if (cmd == "compile") {
        if (filepath.empty()) { std::cerr << "No input file.\n"; printUsage(); return 1; }
        compileFile(filepath, debug);
    } else if (cmd == "run") {
        if (filepath.empty()) { std::cerr << "No input file.\n"; printUsage(); return 1; }
        runFile(filepath, debug);
    } else {
        printUsage();
    }

    return 0;
}

