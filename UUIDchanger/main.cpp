#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
#include <bcrypt.h>
#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string version = "1.4.7";

using json = nlohmann::ordered_json;

// Generate new UUIDs
boost::uuids::random_generator generator;
boost::uuids::uuid new_uuid_header = generator();
boost::uuids::uuid new_uuid_module = generator();

std::string generateRandomUUID() {
    boost::uuids::random_generator generator;
    boost::uuids::uuid new_uuid = generator();
    std::stringstream ss;
    ss << new_uuid;
    return ss.str();
}

void helpcommand() {
    std::cout << "uuid <command>" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << std::endl;
    std::cout << std::endl;
    std::cout << "uuid <JSON-File>      Changes the UUID's in the specified file." << std::endl;
    std::cout << "uuid --random         Copies random UUID to the clipboard." << std::endl;
    std::cout << "uuid --alias          Get all aliases." << std::endl;
    std::cout << "uuid --version        Get current Version." << std::endl;
}

void aliascommand() {
    std::cout << "Aliases:" << std::endl;
    std::cout << std::endl;
    std::cout << "uuid --help       --h / ?" << std::endl;
    std::cout << "uuid --random     --r" << std::endl;
    std::cout << "uuid --alias      --a" << std::endl;
    std::cout << "uuid --version    --v" << std::endl;
}

int filecommand(std::string filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "The file could not be opened." << std::endl;
        return 1;
    }

    std::string json_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Parse JSON data as ordered_json
    json data = json::parse(json_data);

    // Store the original "subpacks" in JSON
    json subpacks;
    if (data.contains("subpacks")) {
        subpacks = data["subpacks"];
    }

    // Save the original "name" in the header
    std::string name = "";
    if (data.contains("header") && data["header"].contains("name")) {
        name = data["header"]["name"];
    }

    // Replace UUIDs in header
    if (data.contains("header") && data["header"].contains("uuid")) {
        data["header"]["uuid"] = boost::uuids::to_string(new_uuid_header);
    }

    // Replace UUIDs in modules
    if (data.contains("modules")) {
        for (auto& module : data["modules"]) {
            if (module.contains("uuid")) {
                module["uuid"] = boost::uuids::to_string(new_uuid_module);
            }
        }
    }

    // Create an ordered JSON object and set the fields in the desired order
    json ordered_data;
    ordered_data["format_version"] = data["format_version"];
    json& header = ordered_data["header"];
    header["name"] = name;
    header["description"] = data["header"]["description"];
    header["uuid"] = data["header"]["uuid"];
    header["version"] = data["header"]["version"];
    header["min_engine_version"] = data["header"]["min_engine_version"];
    ordered_data["modules"] = data["modules"];
    ordered_data["subpacks"] = subpacks;

    // Convert back to JSON string
    std::string new_json_data = ordered_data.dump(4);

    // Save updated JSON file
    std::ofstream outfile(filename);
    if (!outfile) {
        std::cout << "The file could not be saved." << std::endl;
        return 1;
    }
    outfile << new_json_data;
    outfile.close();

    std::cout << "The UUIDs have been successfully updated." << std::endl;
    std::cout << std::endl;
    std::cout << "UUID 1: " << new_uuid_header << std::endl;
    std::cout << "UUID 2: " << new_uuid_module << std::endl;

    return 0;
}

int main(int argc, char* argv[]) {

    SetConsoleOutputCP(CP_UTF8);

    if (argc < 2) {
        helpcommand();
        return 1;
    }

    if (argc > 1) {
        std::string option = argv[1];
        if (option == "--version" || option == "--v") {
            std::cout << "UUIDchanger " << version << std::endl;
            return 0;
        }
        else if (option == "--help" || option == "--h" || option == "?") {
            helpcommand();
            return 0;
        }
        else if (option == "--alias" || option == "--a") {
            aliascommand();
            return 0;
        }
        else if (option == "--random" || option == "--r")
        {
            std::string new_uuid = generateRandomUUID();

            // Copy UUID to clipboard
            std::string clipboard_command = "echo " + new_uuid + " | clip";
            system(clipboard_command.c_str());

            std::cout << "A random UUID was generated and copied to the clipboard." << std::endl;
            std::cout << std::endl;
            std::cout << "UUID: " << new_uuid << std::endl;

            return 0;
        }
    }

    filecommand(argv[1]);

    return 0;
}
