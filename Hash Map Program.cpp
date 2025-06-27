#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
unordered_map<int, string> hashmap;
ofstream logfile("log_hash.txt", ios::app);

string green(const string& text)  { return "\033[1;32m" + text + "\033[0m"; }
string red(const string& text)    { return "\033[1;31m" + text + "\033[0m"; }
string blue(const string& text)   { return "\033[1;34m" + text + "\033[0m"; }
string yellow(const string& text) { return "\033[1;33m" + text + "\033[0m"; }
string pink(const string& text)   { return "\033[1;35m" + text + "\033[0m"; } 
string purple(const string& text) { return "\033[0;35m" + text + "\033[0m"; } 

void log(const string& message) {
    logfile << message << endl;
}

void insert(int key, const string& value) {
    auto start = chrono::high_resolution_clock::now();
    hashmap[key] = value;
    auto end = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, micro>(end - start).count();
    log("Insert: key=" + to_string(key) + ", time=" + to_string(duration) + " ms");
    cout << green("Inserted in ") << duration << " ms\n";
}

void remove(int key) {
    auto start = chrono::high_resolution_clock::now();
    bool found = hashmap.erase(key);
    auto end = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, micro>(end - start).count();
    if (found) {
        log("Delete: key=" + to_string(key) + ", time=" + to_string(duration) + " ms");
        cout << green("Deleted") << " in " << duration << " ms\n";
    }
    else {
        log("Delete failed: key=" + to_string(key) + ", time=" + to_string(duration) + " ms");
        cout << red("Key not found.\n");
    }
}

void update(int key, const string& new_value) {
    auto start = chrono::high_resolution_clock::now();
    bool found = hashmap.find(key) != hashmap.end();
    if (found) hashmap[key] = new_value;
    auto end = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, micro>(end - start).count();
    if (found) {
        log("Update: key=" + to_string(key) + ", time=" + to_string(duration) + " ms");
        cout << green("Updated") << " in " << duration << " ms!\n";
    }
    else {
        log("Update failed: key=" + to_string(key) + ", time=" + to_string(duration) + " ms");
        cout << red("Key not found.\n");
    }
}

void find(int key) {
    auto start = chrono::high_resolution_clock::now();
    auto i = hashmap.find(key);
    auto end = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, micro>(end - start).count();
    if (i != hashmap.end()) {
        cout << green("Found: ") << i->second << endl;
        log("Find: key=" + to_string(key) + ", time=" + to_string(duration) + " ms");
    } 
    else {
        cout << red("Key not found\n");
        log("Find failed: key=" + to_string(key) + ", time=" + to_string(duration) + " ms");
    }
    cout << "Lookup time: " << duration << " ms\n";
}

void display() {
    cout << yellow("\n=== Current Data in Hash Map ===\n");
    vector<pair<int, string>> sorted(hashmap.begin(), hashmap.end());
    sort(sorted.begin(), sorted.end());
    for (auto& [k, v] : sorted) {
        cout << blue(to_string(k)) << yellow(" -> ") << pink(v) << endl;
    }
    log("Displayed all entries.");
}

void loadFromFile(const string& filename) {
    ifstream file(filename);
    string line;
    int key;
    string value;

    auto start = chrono::high_resolution_clock::now();
    while (getline(file, line)) {
        istringstream iss(line);
        iss >> key;
        getline(iss, value);
        if (!value.empty() && value[0] == ' ') value.erase(0, 1);
        if (!value.empty() && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2); 
        }
        hashmap[key] = value;
    }
    file.close();
    auto end = chrono::high_resolution_clock::now();
    double loadTime = chrono::duration<double, milli>(end - start).count();
    cout << green("Loaded in ") << loadTime << " ms\n";
    log("Loaded from file: " + filename + ", time: " + to_string(loadTime) + " ms");
}

void showMenu() {
    cout << blue("\n==============================");
    cout << blue("\nHash Map Menu >.<\n");
    cout << pink("1. Insert Data\n");
    cout << pink("2. Delete Data\n");
    cout << pink("3. Update Data\n");
    cout << pink("4. Find Data\n");
    cout << pink("5. Display All\n");
    cout << pink("6. Exit\n");
    cout << blue("==============================\n");
    cout << blue("Your choice: ");
}

int main() {
    int choice, key;
    string value, filename;

    cout << yellow("Load data from file (ex: 100_movies.txt): ");
    cin >> filename;
    loadFromFile(filename);

    while (true) {
        showMenu();
        cin >> choice;
        switch (choice) {
        case 1:
            cout << purple("Key: ");
            cin >> key;
            cin.ignore();
            cout << purple("Value: ");
            getline(cin, value);
            insert(key, value);
            break;
        case 2:
            cout << purple("Key to delete: ");
            cin >> key;
            remove(key);
            break;
        case 3:
            cout << purple("Key to update: ");
            cin >> key;
            cin.ignore();
            cout << purple("New value: ");
            getline(cin, value);
            update(key, value);
            break;
        case 4:
            cout << purple("Key to find: ");
            cin >> key;
            find(key);
            break;
        case 5:
            display();
            break;
        case 6:
            cout << green("Exiting. Thank you!\n");
            log("Program Terminated\n");
            logfile.close();
            return 0;
        default:
            cout << red("Invalid choice. Try again.\n");
        }
    }
}
