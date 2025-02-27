#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"  // Include a JSON library (e.g., nlohmann/json)
#include <string>
#include <map>
#include <regex>
#include <stack>

using json = nlohmann::json;



/*

ZADACI
1. ispraviti bug pri modifikaciji main funkcije                 Uspjesno
2. dodati opciju za ukljucivanje i iskljucivanje profilera
3. prepraviti da se testovi samo jednom ucitaju
4. dodati opciju odabira samo jednog testa, pri cemu se detaljno ispisuje provedeno testiranje, ukljucujuci razlike
5. napraviti da se program izvrsava u beskonacnoj petlji
6. istraziti docker i napraviti projekat u dockeru, tako da se moze izvrsavati na svim operativnim sistemima
7. napraviti da se prvo kompajlira pravi program, radi gresaka, pa tek onda da se pokrene testiranje        Uspjesno
8. napraviti da se samo jednom nadju pozicije main funkcije     Uspjesno
*/

struct Test{
    int testNum;
    std::map<std::string, std::string> patch;
    std::vector<std::string> expected;
    Test(int n, const std::map< std::string, std::string > &p, const std::vector< std::string > &exp) : testNum(n), patch(p), expected(exp){};
};

std::string::const_iterator nadji_main_kraj(const std::string &code_string, std::string::const_iterator pocetak){

    bool UCharu = false, UStringu = false, UJednolinijskomKomentaru = false, UViselinijskomKomentaru = false;
    std::stack<char> otvoreneViticaste;
    otvoreneViticaste.push('{');
    auto it = pocetak-1;
    while(!otvoreneViticaste.empty()) {
        it++;
        if(!UCharu && !UStringu && !UJednolinijskomKomentaru && !UViselinijskomKomentaru){
            if(*it == '/'){
                it++;
                if(*it == '/') UJednolinijskomKomentaru = true;
                if(*it == '*') UViselinijskomKomentaru = true;
            }
        }
        
        if(!UJednolinijskomKomentaru && !UViselinijskomKomentaru){
            if(*it == '\"') UStringu = !UStringu;
            if(*it == '\'') UCharu = !UCharu;
        }

        if(UJednolinijskomKomentaru && *it == '\n') UJednolinijskomKomentaru = false;
        if(UViselinijskomKomentaru && *it == '*'){
            it++;
            if(*it == '/') UViselinijskomKomentaru = false;
        }

        if(!UCharu && !UStringu && !UJednolinijskomKomentaru && !UViselinijskomKomentaru){
            if(*it=='{') otvoreneViticaste.push('{');
            if(*it=='}' && !otvoreneViticaste.empty()) otvoreneViticaste.pop();
        }
    }

    return it;
}

void main_code_position(const std::string& code_string, std::string::const_iterator &start_iter, std::string::const_iterator &end_iter) {
    // Find the start and end of the `main` function definition using regular expressions.
    std::regex main_start_regex(R"(\bint\s+main\s*\(([^()]*)\)\s*\{)");

    std::smatch main_start_match;

    start_iter = code_string.cbegin();
    end_iter = code_string.cend();

    if (std::regex_search(start_iter, end_iter, main_start_match, main_start_regex)) {
        start_iter = main_start_match[0].first; 
        end_iter = main_start_match[0].second; // Start of main function body
        //end_iter = nadji_main_kraj(code_string,start_iter);
        // return code_string.substr(0, start_iter - code_string.begin()) + "\n" + new_code + "\n"  + code_string.substr(end_iter - code_string.begin());
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

void modifikujFajl(Test &test, const std::string &code, std::string::const_iterator ispred_maina, 
        std::string::const_iterator pocetak_maina, std::string::const_iterator kraj_maina){

    std::ofstream izlaz("code.cpp");

    izlaz<<code.substr(0, ispred_maina - code.begin());

    if(test.patch["above_main"].length() != 0){
        izlaz<<test.patch["above_main"];
    }

    izlaz<<code.substr(ispred_maina - code.begin(), pocetak_maina-ispred_maina);

    if(test.patch["main"].length() != 0){
        izlaz<<test.patch["main"];
    }

    izlaz<<code.substr(kraj_maina - code.begin(), code.end()-kraj_maina);
    
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
    ulaz.close();

    for(const std::string &s:v){
        //std::cout<<"                "<<s.length()<<" : "<<rezultat.length()<<std::endl;
        if(trim_whitespaces(trim_newlines(s))==rezultat) return true;
        //compare_strings(rezultat,trim_whitespaces(trim_newlines(s)));
    }
    std::cout<<"Result: "<<std::endl<<rezultat<<std::endl;
    return false;
}

bool check_valgrind_output() {
    // Run Valgrind on the file, redirecting output to a temporary file
    std::string temp_output_file = "valgrind_output.tmp";
    std::string command = "valgrind --tool=memcheck --leak-check=full --log-file=" + temp_output_file + " " + "./output.out > izlaz.txt";
    system(command.c_str());

    // Read the Valgrind output file
    std::ifstream output_file(temp_output_file);
    std::string output((std::istreambuf_iterator<char>(output_file)),
                        std::istreambuf_iterator<char>());
    output_file.close();

    // Check for errors
    std::regex error_regex(R"(==(\d+)== ERROR SUMMARY: (\d+) errors from (\d+) contexts)");
    std::smatch error_match;
    if (std::regex_search(output, error_match, error_regex) && error_match[2] != "0") {
        std::cout << "Valgrind found " << error_match[2] << " errors." << std::endl;
        return false;
    }

    // Check for memory leaks
    std::regex leak_regex(R"(definitely lost: (\d+) bytes in (\d+) blocks)");
    std::smatch leak_match;
    if (std::regex_search(output, leak_match, leak_regex)) {
        std::cout << "Valgrind found " << leak_match[1] << " bytes in " << leak_match[2] << " memory leaks." << std::endl;
        return false;
    }

    std::cout << "Valgrind found no errors or memory leaks." << std::endl;
    return true;
}


void testiraj(std::vector<Test>& testovi){

    bool zanemariProfiler = true;

    std::string source("/home/ado/fakultet/semestar3/asp/PZ3/zatester.cpp");
    std::ifstream kod(source);
    std::string code_string((std::istreambuf_iterator<char>(kod)), std::istreambuf_iterator<char>());

    std::string::const_iterator ispred_maina;
    std::string::const_iterator pocetak_maina;
    std::string::const_iterator kraj_maina;

    main_code_position(code_string, ispred_maina, pocetak_maina);
    kraj_maina = nadji_main_kraj(code_string, pocetak_maina);

    // dodati inicijalno kompajliranje

    std::cout<<"Inicijalno kompajliranje: ";
    std::ofstream izlaz("code.cpp");
    izlaz<<code_string<<std::endl;
    izlaz.close();
    int result = system("g++ code.cpp -o output.out");

    if (result == 0) {
        std::cout << "\033[1;32m" <<"Uspjesno"<< "\033[0m"<<std::endl;
        for(Test &t:testovi){
            modifikujFajl(t, code_string, ispred_maina, pocetak_maina, kraj_maina);


            izvrsiKomandu();
            if(provjeriRezultat(t.expected) && (zanemariProfiler || check_valgrind_output())){
                std::cout<<"Test "<<t.testNum<<": " << "\033[1;32m" <<"Correct"<< "\033[0m"<<std::endl;
            }else{
                std::cout<<"Test "<<t.testNum<<": " << "\033[1;31m" <<"Not correct"<< "\033[0m"<<std::endl;
            }
        }
    } else {
        std::cerr << "\033[1;31m" <<"Program se ne kompajlira"<< "\033[0m"<<std::endl;
    }
    

    
    kod.close();
}

void izlistajTestove(){
    std::ifstream file("/home/ado/fakultet/semestar3/asp/Autotestovi/PripremnaZadaca3/autotest2");
    if (!file.is_open()) {
        std::cerr << "Error opening JSON file!\n";
        return;
    }

    json data;
    file >> data;
    file.close();
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
