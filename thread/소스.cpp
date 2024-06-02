#include <iostream>
#include <thread>
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>
#include <functional>
#include <map>

using namespace std;
mutex io_mutex;

int gcd(int x, int y) {
    return y == 0 ? x : gcd(y, x % y);
}

int countPrimes(int n) {
    std::vector<bool> prime(n + 1, true);
    prime[0] = prime[1] = false;
    int count = 0;

    for (int i = 2; i * i <= n; ++i) {
        if (prime[i]) {
            for (int j = i * i; j <= n; j += i) {
                prime[j] = false;
            }
        }
    }

    for (int i = 2; i <= n; ++i) {
        if (prime[i]) ++count;
    }

    return count;
}

int sum(int n) {
    return n * (n + 1) / 2;
}

void parallelSum(int n, int m) {
    int totalSum = 0;
    vector<thread> threads;
    int part = n / m;

    auto partialSum = [&](int start, int end) {
        int partial = 0;
        for (int i = start; i <= end; ++i) {
            partial += i;
        }
        lock_guard<mutex> guard(io_mutex);
        totalSum += partial;
        };

    for (int i = 0; i < m; ++i) {
        int start = i * part + 1;
        int end = (i == m - 1) ? n : (i + 1) * part;
        threads.emplace_back(partialSum, start, end);
    }

    for (auto& th : threads) {
        th.join();
    }

    lock_guard<mutex> guard(io_mutex);
    cout << "parallel sum: " << totalSum << endl;
}

void executeCommand(const string& command, const vector<string>& params) {
    if (command == "echo") {
        for (const auto& param : params) {
            cout << param << " ";
        }
        cout << endl;
    }
    else if (command == "gcd") {
        int x = stoi(params[0]);
        int y = stoi(params[1]);
        cout << "gcd: " << gcd(x, y) << endl;
    }
    else if (command == "prime") {
        int x = stoi(params[0]);
        cout << "prime count: " << countPrimes(x) << endl;
    }
    else if (command == "sum") {
        int x = stoi(params[0]);
        cout << "sumall: " << sum(x) << endl;
    }
}

void docommands(const string& command, const vector<string>& params) {
    map<string, string> options;
    vector<string> actualParams;
    for (const auto& param : params) {
        if (param[0] == '-') { // 옵션인 경우
            if (param.size() > 2 && (param[1] == 'n' || param[1] == 'd' || param[1] == 'p' || param[1] == 'm')) {
                options[param.substr(0, 2)] = param.substr(2);
            }
        }
        else {
            actualParams.push_back(param);
        }
    }

    int repeat = 1;
    int delay = 0;
    int interval = 0; // 실행 간격
    int threads = 1;

    if (options.find("-n") != options.end()) {
        threads = stoi(options["-n"]);
    }

    if (options.find("-d") != options.end()) {
        delay = stoi(options["-d"]);
    }

    if (options.find("-p") != options.end()) {
        interval = stoi(options["-p"]);
    }

    auto task = [&]() {
        if (delay > 0) {
            this_thread::sleep_for(chrono::seconds(delay));
        }
        executeCommand(command, actualParams);
        if (interval > 0) {
            this_thread::sleep_for(chrono::seconds(interval));
        }
        };

    vector<thread> threadPool;
    for (int i = 0; i < threads; ++i) {
        threadPool.emplace_back(task);
    }
    for (auto& th : threadPool) {
        th.join();
    }
}

int main() {
    ifstream file("commands.txt");
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string command;
        vector<string> params;
        iss >> command;
        string param;
        while (iss >> param) {
            params.push_back(param);
        }

        docommands(command, params);
    }

    return 0;
}