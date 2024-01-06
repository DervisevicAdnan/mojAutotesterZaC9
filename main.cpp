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
5. uporedjivanje ocekivanog rezultata                               uspjesno
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
        int result = system("g++ code.cpp -o output.out && ./output.out > izlaz.txt");

    if (result == 0) {
        std::cout << "Komanda je uspješno izvršena.\n";
    } else {
        std::cerr << "Greška pri izvođenju komande.\n";
    }
}

std::string _longest_common_substring(const std::string& str1, const std::string& str2) {
    int m = str1.length();
    int n = str2.length();

    // Create a table to store lengths of common substrings
    int dp[m + 1][n + 1];
    memset(dp, 0, sizeof(dp));  // Initialize all elements to 0

    // Fill the table in a bottom-up manner
    int maxLength = 0;
    int endPosition = 0;  // Index of the last character in the longest substring
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (str1[i - 1] == str2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;

                if (dp[i][j] > maxLength) {
                    maxLength = dp[i][j];
                    endPosition = i - 1;  // Track the end position for extraction
                }
            }
        }
    }

    // Extract the longest common substring from the table
    return str1.substr(endPosition - maxLength + 1, maxLength);
}


void compare_strings(const std::string& str1, const std::string& str2) {
    int differences = 0;

    // Find the longest common substring to align the output
    std::string lcs = _longest_common_substring(str1, str2);

    // Print the differences, highlighting them
    for (int i = 0; i < std::max(str1.length(), str2.length()); ++i) {
        char c1 = (i < str1.length()) ? str1[i] : ' ';
        char c2 = (i < str2.length()) ? str2[i] : ' ';

        if (c1 != c2) {
            std::cout << "\033[1;31m" << c1 << "\033[0m";  // Red for differences
            differences++;
        } else if (lcs.find(c1) != std::string::npos) {
            std::cout << "\033[1m" << c1 << "\033[0m";  // Bold for common parts
        } else {
            std::cout << c1;  // Normal for other characters
        }
    }

    std::cout << "\nNumber of differences: " << differences << std::endl;
}

std::string trim_newlines(const std::string& str) {
    size_t start = str.find_first_not_of('\n');
    size_t end = str.find_last_not_of('\n');

    if (start == std::string::npos) {
        return "";
    }

    return str.substr(start, end - start + 1);
}

std::string trim_whitespaces(const std::string& str) {
    size_t start = str.find_first_not_of(' ');
    size_t end = str.find_last_not_of(' ');

    if (start == std::string::npos) {  // All whitespace
        return "";
    }

    return str.substr(start, end - start + 1);
}

bool provjeriRezultat(const std::vector<std::string> &v){
    std::ifstream ulaz("izlaz.txt");
    std::string rezultat((std::istreambuf_iterator<char>(ulaz)), std::istreambuf_iterator<char>());
    rezultat = trim_whitespaces(trim_newlines(rezultat));

    for(const std::string &s:v){
        //std::cout<<"                "<<s.length()<<" : "<<rezultat.length()<<std::endl;
        if(trim_whitespaces(trim_newlines(s))==rezultat) return true;
        //compare_strings(rezultat,trim_whitespaces(trim_newlines(s)));
    }
    std::cout<<"Result: "<<std::endl<<rezultat<<std::endl;
    return false;
}


void testiraj(std::vector<Test>& testovi){
    for(Test &t:testovi){
        modifikujFajl(t);
        izvrsiKomandu();
        if(provjeriRezultat(t.expected)){
            std::cout<<"Test "<<t.testNum<<": Correct"<<std::endl;
        }else{
            std::cout<<"Test "<<t.testNum<<": Not correct"<<std::endl;
        }
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
        //std::cout << "\nTest " << test_count++ << ":" << std::endl;

        std::map <std::string, std::string> map;
        
        // Access and print relevant fields within each test:
        int id = test["id"];
        //std::cout << "  ID: " << id << std::endl;
        //std::cout << "  Patch:"<<std::endl;
        const json& patches = test["patch"];
        for(const auto& patch:patches){
            //std::cout<<"    position: "<<patch["position"]<<std::endl;
            //std::cout<<"    code: "<<std::endl<<std::string(patch["code"])<<std::endl<<std::endl;
            map[patch["position"]] = std::string(patch["code"]);
        }

        //std::cout<<"  Execute:"<<std::endl;
        const json& execute = test["execute"];
        const json& expections = execute["expect"];

        std::vector<std::string> v;
        for(const auto& expect:expections){
            //std::cout<<"    expect: "<<std::endl<<std::string(expect)<<std::endl<<std::endl;
            v.push_back(std::string(expect));
        }
        testovi.push_back(Test(test_count++, map, v));
        // Access and print other fields as needed, e.g., "patch", "execute", etc.
    }
    testiraj(testovi);
}



int main() {
    izlistajTestove();
    return 0;
}
