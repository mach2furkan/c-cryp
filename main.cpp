//this   is just for the edu purpose not for commercial 
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// Constants
const int DEFAULT_KEY_SIZE = 2048;

// Function to find GCD
template <typename T>
T gcd(T a, T b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

// Miller-Rabin primality test
bool isPrime(long long n, int k = 5) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0) return false;

    long long d = n - 1;
    int s = 0;
    while (d % 2 == 0) {
        d /= 2;
        s++;
    }

    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<long long> dist(2, n - 2);

    for (int i = 0; i < k; i++) {
        long long a = dist(gen);
        long long x = 1;
        long long y = a;
        long long temp = d;

        // Compute a^d % n
        while (temp > 0) {
            if (temp % 2 == 1)
                x = (x * y) % n;
            y = (y * y) % n;
            temp /= 2;
        }

        if (x == 1 || x == n - 1)
            continue;

        for (int j = 0; j < s - 1; j++) {
            x = (x * x) % n;
            if (x == n - 1)
                break;
        }

        if (x != n - 1)
            return false;
    }
    return true;
}

// Generate large prime number
long long generateLargePrime(int bits) {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<long long> dist(1LL << (bits - 1), (1LL << bits) - 1);

    long long num;
    do {
        num = dist(gen);
        num |= 1; // Ensure odd
    } while (!isPrime(num));

    return num;
}

// Extended Euclidean Algorithm for modular inverse
template <typename T>
T modInverse(T a, T m) {
    T m0 = m;
    T y = 0, x = 1;

    if (m == 1)
        return 0;

    while (a > 1) {
        T q = a / m;
        T t = m;

        m = a % m;
        a = t;
        t = y;

        y = x - q * y;
        x = t;
    }

    if (x < 0)
        x += m0;

    return x;
}

// Modular exponentiation (base^exp % mod)
template <typename T>
T modExp(T base, T exp, T mod) {
    T result = 1;
    base %= mod;

    while (exp > 0) {
        if (exp % 2 == 1)
            result = (result * base) % mod;

        exp >>= 1;
        base = (base * base) % mod;
    }

    return result;
}

// Simple XOR cipher for demonstration
string xorCipher(const string& data, const string& key) {
    string output = data;
    for (size_t i = 0; i < data.size(); ++i) {
        output[i] = data[i] ^ key[i % key.size()];
    }
    return output;
}

// Basic SHA-256 implementation (simplified for demonstration)
string sha256(const string& str) {
    // In a real implementation, this would be the actual SHA-256 algorithm
    // For demonstration purposes, we'll return a fixed-length string
    return "simulated_sha256_hash_" + str.substr(0, 10);
}

// Hybrid encryption (RSA + XOR)
string hybridEncrypt(const string& message, long long e, long long n) {
    // Generate random XOR key
    string xorKey(16, 0);
    random_device rd;
    for (char& c : xorKey) {
        c = rd() % 256;
    }

    // Encrypt message with XOR
    string xorEncrypted = xorCipher(message, xorKey);

    // Encrypt XOR key with RSA
    string rsaEncryptedKey;
    for (char c : xorKey) {
        long long encryptedChar = modExp((long long)c, e, n);
        rsaEncryptedKey += to_string(encryptedChar) + " ";
    }

    return rsaEncryptedKey + "|" + xorEncrypted;
}

// Hybrid decryption (RSA + XOR)
string hybridDecrypt(const string& encrypted, long long d, long long n) {
    size_t separator = encrypted.find("|");
    if (separator == string::npos) return "";

    string rsaEncryptedKey = encrypted.substr(0, separator);
    string xorEncrypted = encrypted.substr(separator + 1);

    // Decrypt XOR key with RSA
    string xorKey;
    long long c;
    size_t start = 0, end = 0;
    while ((end = rsaEncryptedKey.find(" ", start)) != string::npos) {
        c = stoll(rsaEncryptedKey.substr(start, end - start));
        xorKey += (char)modExp(c, d, n);
        start = end + 1;
        if (end == string::npos) break;
    }

    // Decrypt message with XOR
    return xorCipher(xorEncrypted, xorKey);
}

// Performance benchmark
void benchmark() {
    cout << "\nPerformance Benchmark:\n";
    cout << "----------------------\n";

    auto start = high_resolution_clock::now();
    long long p = generateLargePrime(512);
    auto end = high_resolution_clock::now();
    cout << "Generated 512-bit prime in: "
         << duration_cast<milliseconds>(end - start).count() << " ms\n";

    start = high_resolution_clock::now();
    long long q = generateLargePrime(512);
    end = high_resolution_clock::now();
    cout << "Generated second 512-bit prime in: "
         << duration_cast<milliseconds>(end - start).count() << " ms\n";

    long long n = p * q;
    long long phi = (p - 1) * (q - 1);
    long long e = 65537;
    long long d = modInverse(e, phi);

    string testMessage(1024, 'A');

    start = high_resolution_clock::now();
    string encrypted;
    for (char c : testMessage) {
        encrypted += to_string(modExp((long long)c, e, n)) + " ";
    }
    end = high_resolution_clock::now();
    cout << "RSA Encryption time: "
         << duration_cast<milliseconds>(end - start).count() << " ms\n";

    start = high_resolution_clock::now();
    string decrypted;
    size_t pos = 0;
    while (pos < encrypted.size()) {
        size_t space_pos = encrypted.find(' ', pos);
        if (space_pos == string::npos) break;
        long long c = stoll(encrypted.substr(pos, space_pos - pos));
        decrypted += (char)modExp(c, d, n);
        pos = space_pos + 1;
    }
    end = high_resolution_clock::now();
    cout << "RSA Decryption time: "
         << duration_cast<milliseconds>(end - start).count() << " ms\n";

    start = high_resolution_clock::now();
    string hybridEnc = hybridEncrypt(testMessage, e, n);
    end = high_resolution_clock::now();
    cout << "Hybrid Encryption time: "
         << duration_cast<milliseconds>(end - start).count() << " ms\n";

    start = high_resolution_clock::now();
    string hybridDec = hybridDecrypt(hybridEnc, d, n);
    end = high_resolution_clock::now();
    cout << "Hybrid Decryption time: "
         << duration_cast<milliseconds>(end - start).count() << " ms\n";
}

// Key management system
class KeyManager {
private:
    struct KeyPair {
        long long e, d, n;
        string name;
        time_t created;
    };

    vector<KeyPair> keyPairs;

public:
    void generateKeyPair(int bits, const string& name = "") {
        KeyPair kp;
        kp.name = name.empty() ? "KeyPair_" + to_string(keyPairs.size() + 1) : name;

        auto start = high_resolution_clock::now();
        long long p = generateLargePrime(bits / 2);
        long long q = generateLargePrime(bits / 2);
        kp.n = p * q;
        long long phi = (p - 1) * (q - 1);
        kp.e = 65537;
        kp.d = modInverse(kp.e, phi);
        kp.created = time(nullptr);

        keyPairs.push_back(kp);

        auto end = high_resolution_clock::now();
        cout << "Generated " << bits << "-bit key pair '" << kp.name
             << "' in " << duration_cast<milliseconds>(end - start).count()
             << " ms\n";
    }

    void listKeyPairs() const {
        cout << "\nStored Key Pairs:\n";
        cout << "-----------------\n";
        for (const auto& kp : keyPairs) {
            cout << "Name: " << kp.name << "\n";
            cout << "Public Key (e, n): (" << kp.e << ", " << kp.n << ")\n";
            cout << "Created: " << ctime(&kp.created);
            cout << "-----------------\n";
        }
    }

    bool getKeyPair(const string& name, long long& e, long long& d, long long& n) const {
        for (const auto& kp : keyPairs) {
            if (kp.name == name) {
                e = kp.e;
                d = kp.d;
                n = kp.n;
                return true;
            }
        }
        return false;
    }
};

int main() {
    cout << "Advanced RSA Encryption/Decryption System\n";
    cout << "========================================\n\n";

    KeyManager keyManager;
    int choice;
    do {
        cout << "\nMain Menu:\n";
        cout << "1. Generate Key Pair\n";
        cout << "2. List Key Pairs\n";
        cout << "3. RSA Encrypt/Decrypt\n";
        cout << "4. Hybrid Encrypt/Decrypt\n";
        cout << "5. Benchmark Performance\n";
        cout << "6. Exit\n";
        cout << "Enter your choice (1-6): ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                int bits;
                string name;
                cout << "Enter key size in bits (1024, 2048, 4096): ";
                cin >> bits;
                cin.ignore();
                cout << "Enter name for key pair (optional): ";
                getline(cin, name);
                keyManager.generateKeyPair(bits, name);
                break;
            }
            case 2:
                keyManager.listKeyPairs();
                break;
            case 3: {
                string keyName;
                cout << "Enter key pair name to use: ";
                getline(cin, keyName);

                long long e, d, n;
                if (!keyManager.getKeyPair(keyName, e, d, n)) {
                    cout << "Key pair not found!\n";
                    break;
                }

                cout << "\nUsing key pair: " << keyName << "\n";
                cout << "Public Key (e, n): (" << e << ", " << n << ")\n";

                string message;
                cout << "Enter message to encrypt: ";
                getline(cin, message);

                string encrypted;
                for (char c : message) {
                    encrypted += to_string(modExp((long long)c, e, n)) + " ";
                }
                cout << "\nEncrypted: " << encrypted << endl;

                string decrypted;
                size_t pos = 0;
                while (pos < encrypted.size()) {
                    size_t space_pos = encrypted.find(' ', pos);
                    if (space_pos == string::npos) break;
                    long long c = stoll(encrypted.substr(pos, space_pos - pos));
                    decrypted += (char)modExp(c, d, n);
                    pos = space_pos + 1;
                }
                cout << "Decrypted: " << decrypted << endl;
                break;
            }
            case 4: {
                string keyName;
                cout << "Enter key pair name to use: ";
                getline(cin, keyName);

                long long e, d, n;
                if (!keyManager.getKeyPair(keyName, e, d, n)) {
                    cout << "Key pair not found!\n";
                    break;
                }

                cout << "\nUsing key pair: " << keyName << "\n";
                cout << "Public Key (e, n): (" << e << ", " << n << ")\n";

                string message;
                cout << "Enter message to encrypt: ";
                getline(cin, message);

                string encrypted = hybridEncrypt(message, e, n);
                cout << "\nHybrid Encrypted (first part is RSA-encrypted XOR key):\n"
                     << encrypted.substr(0, 100) << "...\n";

                string decrypted = hybridDecrypt(encrypted, d, n);
                cout << "Hybrid Decrypted: " << decrypted << endl;
                break;
            }
            case 5:
                benchmark();
                break;
            case 6:
                cout << "Exiting program...\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 6);

    return 0;
}
