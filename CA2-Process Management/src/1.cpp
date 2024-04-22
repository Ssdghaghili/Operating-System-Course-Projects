#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem> // for std::filesystem
#include <stdexcept>  // for std::invalid_argument

using namespace std;
namespace fs = std::filesystem;

struct classifier
{
    double betha_0;
    double betha_1;
    double bias;
};

vector<classifier> readWeightsFromCSV(const string& folderPath) {
    vector<classifier> weights; // Vector to store the classifier weights

    // Iterate over files in the folder
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) { // Check if it's a regular file
            ifstream file(entry.path());
            if (!file.is_open()) {
                cerr << "Error opening file: " << entry.path() << endl;
                continue; // Skip this file
            }

            string line;
            while (getline(file, line)) { // Read each line
                stringstream ss(line);
                classifier c;
                string value;

                // Read betha_0
                if (getline(ss, value, ',')) {
                    try {
                        c.betha_0 = stod(value);
                    } catch (const invalid_argument& e) {
                        cerr << "Error converting betha_0 to double in file " << entry.path() << ": " << e.what() << endl;
                        continue; // Skip this line
                    }
                } else {
                    cerr << "Error reading betha_0 in file " << entry.path() << endl;
                    continue; // Skip this line
                }

                // Read betha_1
                if (getline(ss, value, ',')) {
                    try {
                        c.betha_1 = stod(value);
                    } catch (const invalid_argument& e) {
                        cerr << "Error converting betha_1 to double in file " << entry.path() << ": " << e.what() << endl;
                        continue; // Skip this line
                    }
                } else {
                    cerr << "Error reading betha_1 in file " << entry.path() << endl;
                    continue; // Skip this line
                }

                // Read bias
                if (getline(ss, value)) {
                    try {
                        c.bias = stod(value);
                    } catch (const invalid_argument& e) {
                        cerr << "Error converting bias to double in file " << entry.path() << ": " << e.what() << endl;
                        continue; // Skip this line
                    }
                } else {
                    cerr << "Error reading bias in file " << entry.path() << endl;
                    continue; // Skip this line
                }

                weights.push_back(c); // Add classifier to vector
            }

            file.close(); // Close the file
        }
    }

    return weights; // Return the vector with classifier weights
}

int main() {
    string folderPath = "weights";
    vector<classifier> weights = readWeightsFromCSV(folderPath);

    // Print the loaded weights (optional)
    for (size_t i = 0; i < weights.size(); ++i) {
        cout << "Classifier " << i+1 << ":" << endl;
        cout << "Betha_0: " << weights[i].betha_0 << endl;
        cout << "Betha_1: " << weights[i].betha_1 << endl;
        cout << "Bias: " << weights[i].bias << endl;
        cout << endl;
    }

    return 0;
}
