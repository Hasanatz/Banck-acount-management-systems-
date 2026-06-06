#include <bits/stdc++.h>
using namespace std;
struct Account {
    string id;
    string name;
    string fname;
    string address;
    string phone;
    string pin;
    string password;
    double balance = 0.0;
};

static const string ACCOUNTS_FILE = "bank.txt";
static const string TRANS_FILE = "transactions.txt";
string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
string serializeAccount(const Account &a) {
    ostringstream oss;
    oss << a.id << '|' << a.name << '|' << a.fname << '|' << a.address << '|' << a.phone
        << '|' << a.pin << '|' << a.password << '|' << fixed << setprecision(2) << a.balance;
    return oss.str();
}

bool deserializeAccount(const string &line, Account &a) {
    vector<string> parts;
    string cur;
    for (char c : line) {
        if (c == '|') { parts.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    parts.push_back(cur);
    if (parts.size() != 8) return false;
    a.id = trim(parts[0]);
    a.name = trim(parts[1]);
    a.fname = trim(parts[2]);
    a.address = trim(parts[3]);
    a.phone = trim(parts[4]);
    a.pin = trim(parts[5]);
    a.password = trim(parts[6]);
    try {
        a.balance = stod(trim(parts[7]));
    } catch (...) {
        a.balance = 0.0;
    }
    return true;
}
vector<Account> loadAccounts() {
    vector<Account> out;
    ifstream fin(ACCOUNTS_FILE);
    if (!fin.is_open()) return out;
    string line;
    while (getline(fin, line)) {
        if (trim(line).empty()) continue;
        Account a;
        if (deserializeAccount(line, a)) out.push_back(a);
    }
    fin.close();
    return out;
}
bool saveAccounts(const vector<Account> &accounts) {
    string tmp = ACCOUNTS_FILE + ".tmp";
    ofstream fout(tmp, ios::trunc);
    if (!fout.is_open()) return false;
    for (const auto &a : accounts) {
        fout << serializeAccount(a) << '\n';
    }
    fout.close();
    // rename
    if (rename(tmp.c_str(), ACCOUNTS_FILE.c_str()) != 0) {
        // try remove and rename
        remove(ACCOUNTS_FILE.c_str());
        if (rename(tmp.c_str(), ACCOUNTS_FILE.c_str()) != 0) {
            return false;
        }
    }
    return true;
}
string nowStr() {
    auto t = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&tt));
    return string(buf);
}

void logTransaction(const string &id, const string &type, double amount, double balanceAfter) {
    ofstream fout(TRANS_FILE, ios::app);
    if (!fout.is_open()) return;
    fout << id << '|' << nowStr() << '|' << type << '|' << fixed << setprecision(2) << amount << '|' << balanceAfter << '\n';
    fout.close();
}

// Helpers
int findAccountIndex(const vector<Account> &accounts, const string &id) {
    for (size_t i = 0; i < accounts.size(); ++i)
        if (accounts[i].id == id) return (int)i;
    return -1;
}

void pressEnter() {
    cout << "\nPress ENTER to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// UI actions
void addAccount(vector<Account> &accounts) {
    Account a;
    cout << "Enter User ID: ";
    cin >> ws;
    getline(cin, a.id);
    if (findAccountIndex(accounts, a.id) != -1) {
        cout << "Account with this ID already exists.\n";
        return;
    }
    cout << "Name: "; getline(cin, a.name);
    cout << "Father's name: "; getline(cin, a.fname);
    cout << "Address: "; getline(cin, a.address);
    cout << "Phone: "; getline(cin, a.phone);
    cout << "PIN (4-6 digits): "; getline(cin, a.pin);
    cout << "Password: "; getline(cin, a.password);
    cout << "Initial balance: "; cin >> a.balance; cin.ignore(numeric_limits<streamsize>::max(), '\n');

    accounts.push_back(a);
    if (saveAccounts(accounts)) {
        cout << "Account created successfully.\n";
        logTransaction(a.id, "CREATE", 0.0, a.balance);
    } else {
        cout << "Error saving account.\n";
    }
}

void showAll(const vector<Account> &accounts) {
    if (accounts.empty()) { cout << "No accounts.\n"; return; }
    cout << "All accounts:\n";
    for (const auto &a : accounts) {
        cout << "ID: " << a.id << " | Name: " << a.name << " | Phone: " << a.phone << " | Balance: " << fixed << setprecision(2) << a.balance << '\n';
    }
}

void searchAccount(const vector<Account> &accounts) {
    cout << "Enter User ID to search: ";
    string id; cin >> id;
    int idx = findAccountIndex(accounts, id);
    if (idx == -1) { cout << "Not found.\n"; return; }
    const Account &a = accounts[idx];
    cout << "ID: " << a.id << "\nName: " << a.name << "\nFather: " << a.fname << "\nAddress: " << a.address
         << "\nPhone: " << a.phone << "\nBalance: " << fixed << setprecision(2) << a.balance << '\n';
}

void deleteAccount(vector<Account> &accounts) {
    cout << "Enter User ID to delete: ";
    string id; cin >> id;
    int idx = findAccountIndex(accounts, id);
    if (idx == -1) { cout << "Not found.\n"; return; }
    accounts.erase(accounts.begin() + idx);
    if (saveAccounts(accounts)) {
        cout << "Account deleted.\n";
        logTransaction(id, "DELETE", 0.0, 0.0);
    } else {
        cout << "Error saving.\n";
    }
}

void editAccount(vector<Account> &accounts) {
    cout << "Enter User ID to edit: ";
    string id; cin >> id; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int idx = findAccountIndex(accounts, id);
    if (idx == -1) { cout << "Not found.\n"; return; }
    Account &a = accounts[idx];
    cout << "Editing (leave blank to keep existing)\n";
    cout << "Name [" << a.name << "]: "; string tmp; getline(cin, tmp); if (!trim(tmp).empty()) a.name = tmp;
    cout << "Father's name [" << a.fname << "]: "; getline(cin, tmp); if (!trim(tmp).empty()) a.fname = tmp;
    cout << "Address [" << a.address << "]: "; getline(cin, tmp); if (!trim(tmp).empty()) a.address = tmp;
    cout << "Phone [" << a.phone << "]: "; getline(cin, tmp); if (!trim(tmp).empty()) a.phone = tmp;
    cout << "PIN [" << a.pin << "]: "; getline(cin, tmp); if (!trim(tmp).empty()) a.pin = tmp;
    cout << "Password [hidden]: "; getline(cin, tmp); if (!trim(tmp).empty()) a.password = tmp;

    if (saveAccounts(accounts)) cout << "Saved.\n"; else cout << "Error saving.\n";
}

void deposit(vector<Account> &accounts) {
    cout << "User ID: "; string id; cin >> id;
    int idx = findAccountIndex(accounts, id);
    if (idx == -1) { cout << "Not found.\n"; return; }
    cout << "Amount to deposit: "; double amt; cin >> amt;
    if (amt <= 0) { cout << "Invalid amount.\n"; return; }
    accounts[idx].balance += amt;
    if (saveAccounts(accounts)) {
        cout << "Deposited. New balance: " << fixed << setprecision(2) << accounts[idx].balance << '\n';
        logTransaction(id, "DEPOSIT", amt, accounts[idx].balance);
    } else cout << "Error saving.\n";
}

void withdraw(vector<Account> &accounts) {
    cout << "User ID: "; string id; cin >> id;
    int idx = findAccountIndex(accounts, id);
    if (idx == -1) { cout << "Not found.\n"; return; }
    cout << "Amount to withdraw: "; double amt; cin >> amt;
    if (amt <= 0) { cout << "Invalid amount.\n"; return; }
    if (amt > accounts[idx].balance) { cout << "Insufficient funds. Current balance: " << accounts[idx].balance << '\n'; return; }
    accounts[idx].balance -= amt;
    if (saveAccounts(accounts)) {
        cout << "Withdrawn. New balance: " << fixed << setprecision(2) << accounts[idx].balance << '\n';
        logTransaction(id, "WITHDRAW", amt, accounts[idx].balance);
    } else cout << "Error saving.\n";
}

void transfer(vector<Account> &accounts) {
    cout << "From User ID: "; string from; cin >> from;
    cout << "To User ID: "; string to; cin >> to;
    int ifrom = findAccountIndex(accounts, from);
    int ito = findAccountIndex(accounts, to);
    if (ifrom == -1 || ito == -1) { cout << "One or both accounts not found.\n"; return; }
    cout << "Amount to transfer: "; double amt; cin >> amt;
    if (amt <= 0) { cout << "Invalid amount.\n"; return; }
    if (amt > accounts[ifrom].balance) { cout << "Insufficient funds.\n"; return; }
    accounts[ifrom].balance -= amt;
    accounts[ito].balance += amt;
    if (saveAccounts(accounts)) {
        cout << "Transfer complete.\n";
        logTransaction(from, "TRANSFER_OUT", amt, accounts[ifrom].balance);
        logTransaction(to, "TRANSFER_IN", amt, accounts[ito].balance);
    } else cout << "Error saving.\n";
}

void printStatement() {
    cout << "Enter User ID to print statement: ";
    string id; cin >> id;
    ifstream fin(TRANS_FILE);
    if (!fin.is_open()) { cout << "No transaction log found.\n"; return; }
    string line;
    cout << "Transactions for " << id << ":\n";
    bool any = false;
    while (getline(fin, line)) {
        if (trim(line).empty()) continue;
        // format: id|timestamp|type|amount|balanceAfter
        if (line.rfind(id + "|", 0) == 0) {
            // starts with id|
            vector<string> parts;
            string cur;
            for (char c : line) {
                if (c == '|') { parts.push_back(cur); cur.clear(); }
                else cur.push_back(c);
            }
            parts.push_back(cur);
            if (parts.size() >= 5) {
                cout << parts[1] << " | " << parts[2] << " | " << parts[3] << " | Balance: " << parts[4] << '\n';
                any = true;
            }
        }
    }
    if (!any) cout << "No transactions for this user.\n";
    fin.close();
}

void adminLogin() {
    // For simplicity: one admin email and pass (in production, don't hardcode)
    cout << "Admin Email: ";
    string email; cin >> email;
    cout << "Password: ";
    string pass; cin >> pass;
    if (email == "admin@example.com" && pass == "admin123") {
        cout << "Admin login successful.\n";
    } else {
        cout << "Invalid admin credentials.\n";
        throw runtime_error("auth failed");
    }
}

int mainMenu() {
    cout << "\n=== Bank/ATM Management ===\n";
    cout << "1. Admin login (required for bank management)\n";
    cout << "2. ATM Management (user operations)\n";
    cout << "3. Exit\n";
    cout << "Choice: ";
    int ch; cin >> ch; return ch;
}

int adminMenu() {
    cout << "\n--- Bank Management ---\n";
    cout << "1. Add new account\n";
    cout << "2. Edit account\n";
    cout << "3. Delete account\n";
    cout << "4. Show all accounts\n";
    cout << "5. Show user statement\n";
    cout << "6. Back\n";
    cout << "Choice: ";
    int ch; cin >> ch; return ch;
}

int atmMenu() {
    cout << "\n--- ATM Management ---\n";
    cout << "1. Deposit\n";
    cout << "2. Withdraw\n";
    cout << "3. Transfer\n";
    cout << "4. Search account\n";
    cout << "5. Print statement\n";
    cout << "6. Back\n";
    cout << "Choice: ";
    int ch; cin >> ch; return ch;
}

int main() {
    try {
        vector<Account> accounts = loadAccounts();
        while (true) {
            int m = mainMenu();
            if (m == 1) {
                try {
                    adminLogin();
                } catch (...) { continue; }
                // admin area
                while (true) {
                    int a = adminMenu();
                    if (a == 1) addAccount(accounts);
                    else if (a == 2) editAccount(accounts);
                    else if (a == 3) deleteAccount(accounts);
                    else if (a == 4) showAll(accounts);
                    else if (a == 5) printStatement();
                    else if (a == 6) break;
                    else cout << "Invalid.\n";
                }
            } else if (m == 2) {
                // ATM-like user operations
                while (true) {
                    int t = atmMenu();
                    if (t == 1) deposit(accounts);
                    else if (t == 2) withdraw(accounts);
                    else if (t == 3) transfer(accounts);
                    else if (t == 4) searchAccount(accounts);
                    else if (t == 5) printStatement();
                    else if (t == 6) break;
                    else cout << "Invalid.\n";
                }
            } else if (m == 3) {
                cout << "Goodbye.\n";
                break;
            } else cout << "Invalid option.\n";
        }
    } catch (const exception &ex) {
        cout << "Error: " << ex.what() << '\n';
    }
    return 0;
}
