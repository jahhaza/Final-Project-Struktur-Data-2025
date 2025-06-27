#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace std;

const int ORDER = 4;

string green(const string& text)  { return "\033[1;32m" + text + "\033[0m"; }
string red(const string& text)    { return "\033[1;31m" + text + "\033[0m"; }
string blue(const string& text)   { return "\033[1;34m" + text + "\033[0m"; }
string yellow(const string& text) { return "\033[1;33m" + text + "\033[0m"; }
string pink(const string& text)   { return "\033[1;35m" + text + "\033[0m"; } 
string purple(const string& text) { return "\033[0;35m" + text + "\033[0m"; } 


class BPlusNode {
public:
    bool isLeaf;
    vector<int> keys;
    vector<string> values;
    vector<BPlusNode*> children;
    BPlusNode* next;

    BPlusNode(bool leaf) : isLeaf(leaf), next(nullptr) {}
};

class BPlusTree {
private:
    BPlusNode* root;
    ofstream logfile;

    void log(const string& msg) {
        logfile << msg << endl;
    }

    void insertInternal(int key, BPlusNode* leftChild, BPlusNode* rightChild, BPlusNode* parent) {
        if (!parent) {
            root = new BPlusNode(false);
            root->keys.push_back(key);
            root->children.push_back(leftChild);
            root->children.push_back(rightChild);
            return;
        }

        auto pos = upper_bound(parent->keys.begin(), parent->keys.end(), key) - parent->keys.begin();
        parent->keys.insert(parent->keys.begin() + pos, key);
        parent->children.insert(parent->children.begin() + pos + 1, rightChild);

        if (parent->keys.size() >= ORDER) {
            int mid = parent->keys.size() / 2;
            int midKey = parent->keys[mid];

            BPlusNode* sibling = new BPlusNode(false);
            sibling->keys.assign(parent->keys.begin() + mid + 1, parent->keys.end());
            sibling->children.assign(parent->children.begin() + mid + 1, parent->children.end());

            parent->keys.resize(mid);
            parent->children.resize(mid + 1);

            insertInternal(midKey, parent, sibling, getParent(root, parent));
        }
    }

    BPlusNode* getParent(BPlusNode* curr, BPlusNode* child) {
        if (!curr || curr->isLeaf) return nullptr;
        for (auto c : curr->children) {
            if (c == child) return curr;
            BPlusNode* parent = getParent(c, child);
            if (parent) return parent;
        }
        return nullptr;
    }

public:
    BPlusTree() : root(nullptr) {
        logfile.open("log.txt", ios::app);
    }

    ~BPlusTree() {
        logfile.close();
    }

    void insert(int key, const string& value) {
        if (!root) {
            root = new BPlusNode(true);
            root->keys.push_back(key);
            root->values.push_back(value);
            log("Inserted: " + to_string(key) + " -> " + value);
            return;
        }

        BPlusNode* curr = root;
        BPlusNode* parent = nullptr;

        while (!curr->isLeaf) {
            parent = curr;
            auto it = upper_bound(curr->keys.begin(), curr->keys.end(), key);
            curr = curr->children[it - curr->keys.begin()];
        }

        auto pos = lower_bound(curr->keys.begin(), curr->keys.end(), key) - curr->keys.begin();
        if (pos < curr->keys.size() && curr->keys[pos] == key) {
            curr->values[pos] = value;
            log("Updated: " + to_string(key) + " -> " + value);
            return;
        }

        curr->keys.insert(curr->keys.begin() + pos, key);
        curr->values.insert(curr->values.begin() + pos, value);
        log("Inserted: " + to_string(key) + " -> " + value);

        if (curr->keys.size() >= ORDER) {
            int mid = curr->keys.size() / 2;
            BPlusNode* sibling = new BPlusNode(true);
            sibling->keys.assign(curr->keys.begin() + mid, curr->keys.end());
            sibling->values.assign(curr->values.begin() + mid, curr->values.end());

            curr->keys.resize(mid);
            curr->values.resize(mid);

            sibling->next = curr->next;
            curr->next = sibling;

            insertInternal(sibling->keys[0], curr, sibling, parent);
        }
    }

    void find(int key) {
        BPlusNode* curr = root;
        while (curr && !curr->isLeaf) {
            auto it = upper_bound(curr->keys.begin(), curr->keys.end(), key);
            curr = curr->children[it - curr->keys.begin()];
        }

        if (curr) {
            for (int i = 0; i < curr->keys.size(); ++i) {
                if (curr->keys[i] == key) {
                    cout << green("Found: ") << curr->values[i] << endl;
                    log("Found: " + to_string(key) + " -> " + curr->values[i]);
                    return;
                }
            }
        }
        cout << red("Key not found\n");
        log("Find failed: " + to_string(key));
    }

    void update(int key, const string& new_value) {
        BPlusNode* curr = root;
        while (curr && !curr->isLeaf) {
            auto it = upper_bound(curr->keys.begin(), curr->keys.end(), key);
            curr = curr->children[it - curr->keys.begin()];
        }

        if (curr) {
            for (int i = 0; i < curr->keys.size(); ++i) {
                if (curr->keys[i] == key) {
                    curr->values[i] = new_value;
                    cout << green("Updated.\n");
                    log("Updated: " + to_string(key) + " -> " + new_value);
                    return;
                }
            }
        }
        cout << red("Key not found\n");
        log("Update failed: " + to_string(key));
    }

    void remove(int key) {
        BPlusNode* curr = root;
        while (curr && !curr->isLeaf) {
            auto it = upper_bound(curr->keys.begin(), curr->keys.end(), key);
            curr = curr->children[it - curr->keys.begin()];
        }

        if (curr) {
            auto it = std::find(curr->keys.begin(), curr->keys.end(), key);
            if (it != curr->keys.end()) {
                int idx = it - curr->keys.begin();
                curr->keys.erase(it);
                curr->values.erase(curr->values.begin() + idx);
                cout << green("Deleted.\n");
                log("Deleted: " + to_string(key));
                return;
            }
        }
        cout << red("Key not found\n");
        log("Delete failed: " + to_string(key));
    }

    void display() {
        cout << yellow("\n=== Current Data in B+ Tree ===\n");
        BPlusNode* curr = root;
        while (curr && !curr->isLeaf) curr = curr->children[0];

        if (!curr) {
            cout << red("No data available.\n");
            return;
        }

        while (curr) {
            for (int i = 0; i < curr->keys.size(); ++i) {
                stringstream line;
                line << left << setw(6) << curr->keys[i] << " => " << curr->values[i];
                cout << blue(to_string(curr->keys[i])) << yellow(" => ") << pink(curr->values[i]) << endl;
            }
            curr = curr->next;
        }
        log("Displayed data.");
    }

    void loadFromFile(const string& filename) {
        ifstream file(filename);
        string line;
        int key;
        string value;

        while (getline(file, line)) {
            istringstream iss(line);
            iss >> key;
            getline(iss, value);
            if (!value.empty() && value[0] == ' ') value.erase(0, 1);
            if (!value.empty() && value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.length() - 2);
            insert(key, value);
        }

        file.close();
        log("Loaded from file: " + filename);
    }
};

void showMenu() {
    cout << blue("\n==============================");
    cout << blue("\nB+ Tree Menu >.<\n");
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
    BPlusTree tree;
    int choice, key;
    string value, filename;

    cout << yellow("Load data from file (e.g. 100.txt): ");
    cin >> filename;

    auto start = chrono::high_resolution_clock::now();
    tree.loadFromFile(filename);
    auto end = chrono::high_resolution_clock::now();
    cout << green("Loaded in ") << chrono::duration<double, milli>(end - start).count() << " ms\n";

    while (true) {
        showMenu();
        cin >> choice;

        switch (choice) {
        case 1:
            cout << purple("Key: ");
            cin >> key;
            cin.ignore();
            cout << ("Value: ");
            getline(cin, value);
            tree.insert(key, value);
            break;
        case 2:
            cout << purple("Key to delete: ");
            cin >> key;
            tree.remove(key);
            break;
        case 3:
            cout << purple("Key to update: ");
            cin >> key;
            cin.ignore();
            cout << purple("New value: ");
            getline(cin, value);
            tree.update(key, value);
            break;
        case 4:
            cout << purple("Key to find: ");
            cin >> key;
            tree.find(key);
            break;
        case 5:
            tree.display();
            break;
        case 6:
            cout << green("Exiting. Thank you!\n");
            return 0;
        default:
            cout << red("Invalid choice. Try again.\n");
        }
    }
}
