#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 
#include <cstring>
#include <dirent.h>


#define CLASSIFICATION_SIZE 3    

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

vector<string> readWeightFiles(const string& folderPath){
    vector<string> filenames;
    DIR* directory;
    struct dirent* entry;

    directory = opendir(folderPath.c_str());
    if (directory != nullptr) {
        while ((entry = readdir(directory)) != nullptr) {
            if (entry->d_type == DT_REG && string(entry->d_name).find(".csv") != string::npos) {
                filenames.push_back(entry->d_name);
            }
        }
        closedir(directory);
    } else {
        cerr << "Error opening directory" << endl;
    }
    return filenames;
}

vector<classifier> readClassifierDatas(const string& folderPath, const string& classifier_file)
{
    int count = 0;
    vector<classifier> weights;

    string filename = folderPath + "/" + classifier_file;

    // cout << filename << endl;

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file!" << endl;
        return weights;
    }

    string line;
    while (getline(file, line)) {
        if (count != 0){

            
            stringstream ss(line);
            classifier c;
            string value;

            if (getline(ss, value, ',')) {
                try {
                    c.betha_0 = stod(value);
                } catch (const invalid_argument& e) {
                    cerr << "Error converting betha_0 to double in file " << endl;
                    continue;
                }
            } else {
                cerr << "Error reading betha_0 in file " << endl;
                continue; 
            }

            // Read betha_1
            if (getline(ss, value, ',')) {
                try {
                    c.betha_1 = stod(value);
                } catch (const invalid_argument& e) {
                    cerr << "Error converting betha_1 to double in file " << endl;
                    continue; 
                }
            } else {
                cerr << "Error reading betha_1 in file " << endl;
                continue; 
            }

            // Read bias
            if (getline(ss, value)) {
                try {
                    c.bias = stod(value);
                } catch (const invalid_argument& e) {
                    cerr << "Error converting bias to double in file " << endl;
                    continue; 
                }
            } else {
                cerr << "Error reading bias in file " << endl;
                continue; 
            }

            weights.push_back(c);
        }
        count++;
    }
    file.close();

    return weights;
}

int findMaxLable(vector<double> predictions)
{
    int maxIndex = 0;
    double max = predictions[0];
    for (int i = 1; i < CLASSIFICATION_SIZE; i++)
    {
        if (predictions[i] > max)
        {
            max = predictions[i];
            maxIndex = i;
        }
    }
    return maxIndex;
}

double calculatePrediction(vector<item> item, vector<classifier> weights, int index)
{
    int label;
    vector<double> predictions;
    double prediction;
    for (int i = 0; i < CLASSIFICATION_SIZE ; i++)
    {
        prediction = weights[i].betha_0 * item[index].length + weights[i].betha_1 * item[index].width + weights[i].bias;
        predictions.push_back(prediction);
    }

    label = findMaxLable(predictions);
    return label;

}



int main(int argc, char *argv[]) {

    // string buffer = "classifier_3.csv";
    // int index = 326;

    string validation_folder = argv[1];
    string weights_folder = argv[2];


    vector<item> items = readDataSet(validation_folder);
    vector<int> main_labels = readLabels(validation_folder);
    vector<string> csvFiles = readWeightFiles(weights_folder);

    vector<double> predicted_labels;
    vector<int> final_item_labels;


    for (int j = 0; j < items.size(); j++)
    {
        int index = j;
        for (int i = 0; i < csvFiles.size(); i++)
        {
            vector<classifier> weights = readClassifierDatas(weights_folder, csvFiles[i]);
            int x = calculatePrediction(items, weights, index);
            predicted_labels.push_back(static_cast<double>(x));
        }

        final_item_labels.push_back(findMaxLable(predicted_labels));
        predicted_labels.clear();
    }

    
    
    
    



    /*                 ------------------- Print ---------------------                  */

    // cout << final_item_labels[0] << endl;

    // for (int j = 0; j < predicted_labels.size(); j++)
    // {
    //     cout << predicted_labels[j] << endl;
    // }

    // cout << "Validation folder: " << validation_folder << endl;
    
    // for (const auto &it : items)
    // {
    //     cout << "Length: " << it.length << ", Width: " << it.width << endl;
    // }

    // for (const auto &it : labels)
    // {
    //     cout << it << endl;
    // }

    // for (int i = 0; i < predicted_labels.size(); i++){
    //     cout << predicted_labels[i] << endl;
    // }


    // // Print the names of CSV files
    // for (const auto& filename : csvFiles) {
    //     cout << filename << endl;
    // }

        return 0;
}





