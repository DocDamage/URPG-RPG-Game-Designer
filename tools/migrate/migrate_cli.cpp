#include "engine/core/format/canonical_json.h"
#include "engine/core/migrate/migration_runner.h"

#include <fstream>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

namespace {

struct CliArgs {
    std::string input_path;
    std::string migration_path;
    std::string output_path;
};

bool ParseArgs(int argc, char** argv, CliArgs& out) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--input" && i + 1 < argc) {
            out.input_path = argv[++i];
        } else if (arg == "--migration" && i + 1 < argc) {
            out.migration_path = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            out.output_path = argv[++i];
        } else {
            return false;
        }
    }

    return !out.input_path.empty() && !out.migration_path.empty() && !out.output_path.empty();
}

bool LoadJsonFile(const std::string& path, nlohmann::json& out) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    in >> out;
    return true;
}

bool SaveTextFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out << content;
    return true;
}

} // namespace

int main(int argc, char** argv) {
    CliArgs args;
    if (!ParseArgs(argc, argv, args)) {
        std::cerr << "Usage: urpg_migrate --input <project.json> --migration <migration_op.json> --output <out.json>\n";
        return 2;
    }

    nlohmann::json document;
    nlohmann::json migration_spec;

    try {
        if (!LoadJsonFile(args.input_path, document)) {
            std::cerr << "Failed to read input JSON: " << args.input_path << "\n";
            return 3;
        }
        if (!LoadJsonFile(args.migration_path, migration_spec)) {
            std::cerr << "Failed to read migration JSON: " << args.migration_path << "\n";
            return 4;
        }
    } catch (const std::exception& ex) {
        std::cerr << "JSON parse error: " << ex.what() << "\n";
        return 5;
    }

    if (auto error = urpg::MigrationRunner::Apply(migration_spec, document)) {
        std::cerr << "Migration failed: " << error->message << "\n";
        return 6;
    }

    if (!SaveTextFile(args.output_path, urpg::DumpCanonicalJson(document))) {
        std::cerr << "Failed to write output JSON: " << args.output_path << "\n";
        return 7;
    }

    return 0;
}
