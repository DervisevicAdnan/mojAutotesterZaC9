#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"  // Include a JSON library (e.g., nlohmann/json)
#include <string>
#include <map>
#include <regex>

using json = nlohmann::json;



/*

ZADACI
1. citanje autotestova                                              uspjesno
2. izvrsavanje komandi u terminalu                                  uspjesno
3. citanje teksta iz terminala (prepravljeno da cita iz datoteke)   uspjesno
4. modifikovanje cpp fajla                                          uspjesno
5. uporedjivanje ocekivanog rezultata
6. pronaci potrebne valgrind komande
7. sastaviti sve
8. dodati da se moze birati path do autotesta i cpp fajla
*/

struct Test{
    int testNum;
    std::map<std::string, std::string> patch;
    std::vector<std::string> expected;
    Test(int n, const std::map< std::string, std::string > &p, const std::vector< std::string > &exp) : testNum(n), patch(p), expected(exp){};
};

std::string replace_main_code(const std::string& code_string, const std::string& new_code) {
    // Find the start and end of the `main` function definition using regular expressions.
    std::regex main_start_regex(R"(\bint\s+main\s*\(([^()]*)\)\s*\{)");
    std::regex main_end_regex(R"(\})");

    std::smatch main_start_match;
    std::smatch main_end_match;

    std::string::const_iterator start_iter = code_string.cbegin();
    std::string::const_iterator end_iter = code_string.cend();

    if (std::regex_search(start_iter, end_iter, main_start_match, main_start_regex)) {
        start_iter = main_start_match[0].second; // Start of main function body
        if (std::regex_search(start_iter, end_iter, main_end_match, main_end_regex)) {
            end_iter = main_end_match[0].first;  // End of main function body

            // Replace the code within the `main` function with the new code.
            return code_string.substr(0, start_iter - code_string.begin()) + "\n" + new_code + "\n"  + code_string.substr(end_iter - code_string.begin());
        } else {
            throw std::runtime_error("Could not find closing brace for `main` function.");
        }
    } else {
        throw std::runtime_error("Could not find `main` function definition in the code.");
    }
}

std::string insert_code_above_main(const std::string& starting_code, const std::string& new_code) {
    // Find the starting position of the `main` function
    std::regex main_start_regex(R"(\bint\s+main\s*\(([^()]*)\)\s*\{)");
    std::smatch main_start_match;

    if (std::regex_search(starting_code, main_start_match, main_start_regex)) {
        // Insert the new code before the `main` function
        std::string modified_code = starting_code;
        modified_code.insert(main_start_match.position(), new_code + "\n");
        return modified_code;
    } else {
        throw std::runtime_error("Could not find `main` function in the code.");
    }
}

void modifikujFajl(Test &test){
    std::string source("/home/ado/fakultet/asp/zadaca4/main2.cpp");
    std::ifstream kod(source);
    std::ofstream izlaz("code.cpp");
    std::string code_string((std::istreambuf_iterator<char>(kod)), std::istreambuf_iterator<char>());
    if(test.patch["main"].length()!=0){
        code_string = replace_main_code(code_string, test.patch["main"]);
    }
    if(test.patch["above_main"].length()!=0){
        code_string = insert_code_above_main(code_string, test.patch["above_main"]);
    }
    izlaz<<code_string;
    kod.close();
    izlaz.close();
}

void izvrsiKomandu(){
        int result = system("ls -l > proba.txt"); // Izvrši komandu "ls -l" za listanje sadržaja direktorija

    if (result == 0) {
        std::cout << "Komanda je uspješno izvršena.\n";
    } else {
        std::cerr << "Greška pri izvođenju komande.\n";
    }
}

void izlistajTestove(){
    std::ifstream file("/home/ado/fakultet/asp/Autotestovi/Zadaca4/autotest2");
    if (!file.is_open()) {
        std::cerr << "Error opening JSON file!\n";
        return;
    }

    json data;
    file >> data;

    // Access and print the "name" parameter:
    std::string name = data["name"];
    std::cout << "Name: " << name << std::endl;

    std::vector<Test> testovi;

    // Access and print the "tests" array, handling multiple tests:
    const json& tests = data["tests"];
    int test_count = 0;
    for (const auto& test : tests) {
        if(test_count==0) {
            test_count++;
            continue;
        }
        std::cout << "\nTest " << test_count++ << ":" << std::endl;

        std::map <std::string, std::string> map;
        
        // Access and print relevant fields within each test:
        int id = test["id"];
        std::cout << "  ID: " << id << std::endl;
        std::cout << "  Patch:"<<std::endl;
        const json& patches = test["patch"];
        for(const auto& patch:patches){
            std::cout<<"    position: "<<patch["position"]<<std::endl;
            std::cout<<"    code: "<<std::endl<<std::string(patch["code"])<<std::endl<<std::endl;
            map[patch["position"]] = std::string(patch["code"]);
        }

        std::cout<<"  Execute:"<<std::endl;
        const json& execute = test["execute"];
        const json& expections = execute["expect"];

        std::vector<std::string> v;
        for(const auto& expect:expections){
            std::cout<<"    expect: "<<std::endl<<std::string(expect)<<std::endl<<std::endl;
            v.push_back(std::string(expect));
        }
        testovi.push_back(Test(test_count, map, v));
        // Access and print other fields as needed, e.g., "patch", "execute", etc.
    }
    modifikujFajl(testovi[0]);
}

void citajIzDatoteke(){
    std::ifstream ulaz("proba.txt");
    while(ulaz) std::cout<<char(ulaz.get());
}



int main() {
    izlistajTestove();
    return 0;
}
