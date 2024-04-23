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

    //up until here, the code is the same as 2.cpp

    int pipefd[2];
    pid_t Combined_classification;

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe Combined_classification error");
        return 1;
    }

    // Create a child process
    Combined_classification = fork();

    if (Combined_classification == -1) {
        perror("fork error");
        return 1;
    }
    else if (Combined_classification == 0)
    { 
        // Child process (Combined classifier process)
        close(pipefd[1]); // Close the write end of the pipe in the child

        // Read from the pipe
        char buffer[20];
        read(pipefd[0], buffer, sizeof(buffer));
        // std::cout << "Combined classifier process received message: " << buffer << std::endl;

        // Perform some calculation on the received string
        vector<classifier> weights = readClassifierDatas(weights_folder, buffer);
        int x = calculatePrediction(items, weights, index);
        predicted_labels.push_back(x);

        // Pass the result to Voter process using another unnamed pipe
        int pipefd_voter[2];
        if (pipe(pipefd_voter) == -1) {
            perror("pipe error");
            return 1;
        }

        pid_t Voter_process = fork();

        if (Voter_process == -1) {
            perror("fork error");
            return 1;
        } 
        else if (Voter_process == 0) {
            // Child process (Voter process)
            close(pipefd_voter[1]); // Close the write end of the pipe in the child

            // Read from the pipe
            char buffer_voter[20];
            read(pipefd_voter[0], buffer_voter, sizeof(buffer_voter));
            std::cout << "Voter process received message: " << buffer_voter << std::endl;

            // Close the read end of the pipe in the child
            close(pipefd_voter[0]);

            return 0;
        } 
        else {
            // Parent process (Combined classifier process)
            // Write to the pipe
            write(pipefd_voter[1], buffer, strlen(buffer) + 1);

            // Close the write end of the pipe in the parent
            close(pipefd_voter[1]);

            // Wait for the Voter process to finish
            wait(NULL);

            // Close the read end of the pipe in the child
            close(pipefd_voter[0]);

            // Close the read end of the pipe in the parent
            close(pipefd[0]);

            return 0;
        }
    } 
    else 
    { 
        // Parent process (Combined classification process)
        close(pipefd[0]); // Close the read end of the pipe in the parent

        // Write to the pipe
        const char* message = "Hello from parent!";
        write(pipefd[1], message, strlen(message) + 1);

        // Close the write end of the pipe in the parent
        close(pipefd[1]);

        // Wait for the Combined classifier process to finish
        wait(NULL);

        return 0;
    }

    return 0;
}
