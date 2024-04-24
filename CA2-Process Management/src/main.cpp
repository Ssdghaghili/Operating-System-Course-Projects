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

vector<int> finalPrediction(vector<int> main_labels, vector<int> predicted_labels)
{
    vector<int> result;
    int correct = 0;
    int wrong = 0;
    for (int i = 0; i < main_labels.size(); i++)
    {
        if (main_labels[i] == predicted_labels[i])
        {
            correct++;
        }
        else
        {
            wrong++;
        }
    }
    result.push_back(correct);
    result.push_back(wrong);

    return result;
}



int main(int argc, char *argv[]) {

    string validation_folder = argv[1];
    string weights_folder = argv[2];

    vector<item> items = readDataSet(validation_folder);
    vector<int> main_labels = readLabels(validation_folder);
    vector<string> csvFiles = readWeightFiles(weights_folder);

    vector<int> predicted_labels;
    vector<int> final_item_labels;
    vector<int> result;

    int pipefd[2];
    pid_t Combined_classification;

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe Combined_classification error");
        return 1;
    }

    // Fork the Combined classification process
    Combined_classification = fork();

    if (Combined_classification < 0) {
        perror("fork Combined_classification error");
        return 1;
    }

    if (Combined_classification == 0) {

        close(pipefd[0]);

        // Pass csvFiles and items to the Linear classification process
        for (int i = 0; i < csvFiles.size(); ++i) {
            write(pipefd[1], csvFiles[i].c_str(), csvFiles[i].size() + 1);
        }

        // Pass the index of items to the Linear classification process
        for (int i = 0; i < items.size(); ++i) {
            write(pipefd[1], &i, sizeof(int));
        }

        close(pipefd[1]); 

        wait(NULL);

    } else { 

        close(pipefd[1]); 

        pid_t Linear_classification;
        Linear_classification = fork();

        if (Linear_classification < 0) {
            perror("fork Linear_classification error");
            return 1;
        }

        if (Linear_classification == 0) { //Linear classification process
            close(pipefd[1]); 

            vector<string> csvFiles;
            vector<item> items;

            // Read csvFiles
            char buffer[1024];
            int bytesRead;
            while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
                string filename(buffer, bytesRead);
                csvFiles.push_back(filename);
            }

            // Read item indexes 
            int index;
            while (read(pipefd[0], &index, sizeof(int)) > 0) {
                items.push_back(items[index]);
            }

            // Call the Linear classification process function
            for (int i = 0; i < csvFiles.size(); i++)
            {
                vector<classifier> weights = readClassifierDatas(weights_folder, csvFiles[i]);
                int x = calculatePrediction(items, weights, index);
                predicted_labels.push_back(static_cast<double>(x));
            }

            exit(0);
        }
    else { 
        close(pipefd[0]);

        vector<int> final_item_labels;

        while (true) {

            // Read final_item_labels from the pipe
            int label;
            while (read(pipefd[1], &label, sizeof(int)) > 0) {
                final_item_labels.push_back(label);
            }

            // Check if there are any more indexes left
            if (final_item_labels.size() < items.size()) {
                int next_index = final_item_labels.size();
                write(pipefd[1], &next_index, sizeof(int));
            
            } else {
                // All indexes have been processed
                break;
            }
        }

        result = finalPrediction(main_labels, final_item_labels);
        cout << "Correct predictions: " << result[0] << ", Wrong predictions: " << result[1] << endl;

        wait(NULL);

        close(pipefd[1]);
    }
    }
    return 0;
}