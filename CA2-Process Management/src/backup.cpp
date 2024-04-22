#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <filesystem>

using namespace std;

struct item
{
    double length;
    double width;
};

struct classifier
{
    double betha_0;
    double betha_1;
    double bias;
};



vector<item> readDataSet(const string& folderPath)
{
    int count = 0;
    vector<item> data;
    string filename = folderPath + "/dataset.csv";

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file!" << endl;
        return data;
    }

    string line;
    while (getline(file, line)) {
        if(count != 0) {
            stringstream ss(line);
            item i;
            string value;

            if (getline(ss, value, ',')) {
                try {
                    i.length = stod(value);
                } catch (const invalid_argument& e) {
                    cerr << "Error converting length to float: " << e.what() << endl;
                    continue; 
                }
            } else {
                cerr << "Error reading length!" << endl;
                continue; 
            }

            if (getline(ss, value)) {
                try {
                    i.width = stod(value);
                } catch (const invalid_argument& e) {
                    cerr << "Error converting width to float: " << e.what() << endl;
                    continue; 
                }
            } else {
                cerr << "Error reading width!" << endl;
                continue; 
            }

            data.push_back(i);
        }
        count++;
    }

    file.close();

    return data;
}

vector<int> readLabels(const string& folderPath)
{
    int count = 0;
    vector<int> labels;
    string filename = folderPath + "/labels.csv";

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file!" << endl;
        return labels;
    }

    string line;
    while (getline(file, line)) {
        if (count != 0){
            stringstream ss(line);
            string value;

            if (getline(ss, value)) {
                try {
                    labels.push_back(stoi(value));
                } catch (const invalid_argument& e) {
                    cerr << "Error converting label to int: " << e.what() << endl;
                    continue; 
                }
            } else {
                cerr << "Error reading label!" << endl;
                continue; 
            }
        }
        count++;
    }

    file.close();

    return labels;

}



vector<classifier> readClassifier(const string& folderPath)
{
    int count = 0;
    vector<classifier> weights;

    for (int i = 0; i < 10; i++) {
        string filename = folderPath + "/classifier_" + to_string(i) + ".csv";

        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error opening file!" << endl;
            return weights;
        }

    }
}

int main(int argc, char *argv[]) {

    string validation_folder = argv[1];
    string weights_folder = argv[2];

    vector<int> labels;

    vector<item> items = readDataSet(validation_folder);
    vector<int> labels = readLabels(validation_folder);

    for (int i = 0; i < 10; i++) {
        string filename = weights_folder + "/classifier_" + to_string(i) + ".csv";
        // labels.push_back(readClassifier(filename));
    }


    /*                 ------------------- Print ---------------------                  */


    // cout << "Validation folder: " << validation_folder << endl;
    
    // for (const auto &it : items)
    // {
    //     cout << "Length: " << it.length << ", Width: " << it.width << endl;
    // }

    // for (const auto &it : labels)
    // {
    //     cout << it << endl;
    // }


    return 0;
}