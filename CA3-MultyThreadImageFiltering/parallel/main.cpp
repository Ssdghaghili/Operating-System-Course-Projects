#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <pthread.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define MAX_NUM_THREADS 8

using namespace std;
using namespace cv;

const int KERNEL_MATRIX[3][3] = {
    {0, 1, 0},
    {1, -4, 1},
    {0, 1, 0}
};

struct ThreadData {
    Mat* srcImage;
    Mat* dstImage;
    int startRow;
    int endRow;
};

void* LaplacianEdgeDetection(void* arg)
{
    ThreadData* data = (ThreadData*)arg;
    Mat& srcImage = *(data->srcImage);
    Mat& dstImage = *(data->dstImage);
    int startRow = data->startRow;
    int endRow = data->endRow;

    for(int row = startRow; row < endRow; ++row)
    {
        for(int col = 1; col < srcImage.cols - 1; ++col)
        {
            int sum = 0;
            for(int i = -1; i <= 1; ++i)
            {
                for(int j = -1; j <= 1; ++j)
                {
                    sum += srcImage.at<uchar>(row + i, col + j) * KERNEL_MATRIX[i + 1][j + 1];
                }
            }
            dstImage.at<uchar>(row, col) = (sum > 255) ? 255 : (sum < 0) ? 0 : sum;
        }
    }

    pthread_exit(NULL);
}

void* grayToBW(void* arg)
{
    ThreadData* data = (ThreadData*)arg;
    Mat& srcImage = *(data->srcImage);
    Mat& dstImage = *(data->dstImage);
    int startRow = data->startRow;
    int endRow = data->endRow;

    for(int row = startRow; row < endRow; ++row)
    {
        for(int col = 0; col < srcImage.cols; ++col)
        {
            dstImage.at<uchar>(row, col) = (srcImage.at<uchar>(row, col) > 127) ? 255 : 0;
        }
    }

    pthread_exit(NULL);
}

void* boxFilter(void* arg)
{
    ThreadData* data = (ThreadData*)arg;
    Mat& srcImage = *(data->srcImage);
    Mat& dstImage = *(data->dstImage);
    int startRow = data->startRow;
    int endRow = data->endRow;

    for(int row = startRow; row < endRow; ++row)
    {
        for(int col = 1; col < srcImage.cols - 1; ++col)
        {
            int sum = 0;
            for(int i = -1; i <= 1; ++i)
            {
                for(int j = -1; j <= 1; ++j)
                {
                    sum += srcImage.at<uchar>(row + i, col + j);
                }
            }
            dstImage.at<uchar>(row, col) = sum / 9;
        }
    }

    pthread_exit(NULL);
}

void* sepiaFilter(void* arg)
{
    ThreadData* data = (ThreadData*)arg;
    Mat& srcImage = *(data->srcImage);
    Mat& dstImage = *(data->dstImage);
    int startRow = data->startRow;
    int endRow = data->endRow;

    for (int row = startRow; row < endRow; ++row)
    {
        for (int col = 0; col < srcImage.cols; ++col)
        {
            Vec3b pixel = srcImage.at<Vec3b>(row, col);

            uchar originalBlue = pixel[0];
            uchar originalGreen = pixel[1];
            uchar originalRed = pixel[2];

            int newRed = static_cast<int>(0.393 * originalRed + 0.769 * originalGreen + 0.189 * originalBlue);
            int newGreen = static_cast<int>(0.349 * originalRed + 0.686 * originalGreen + 0.168 * originalBlue);
            int newBlue = static_cast<int>(0.272 * originalRed + 0.534 * originalGreen + 0.131 * originalBlue);

            dstImage.at<Vec3b>(row, col)[2] = (newRed > 255) ? 255 : newRed;
            dstImage.at<Vec3b>(row, col)[1] = (newGreen > 255) ? 255 : newGreen;
            dstImage.at<Vec3b>(row, col)[0] = (newBlue > 255) ? 255 : newBlue;
        }
    }

    pthread_exit(NULL);
}

void* verticalMirror(void* arg) 
{
    ThreadData* data = (ThreadData*)arg;
    Mat& srcImage = *(data->srcImage);
    Mat& dstImage = *(data->dstImage);
    int startRow = data->startRow;
    int endRow = data->endRow;

    for(int row = startRow; row < endRow; ++row)
    {
        for(int col = 0; col < srcImage.cols; ++col)
        {
            dstImage.at<Vec3b>(row, col) = srcImage.at<Vec3b>(srcImage.rows - 1 - row, col);
        }
    }

    pthread_exit(NULL);
}

void* horizontalMirror(void* arg) 
{
    ThreadData* data = (ThreadData*)arg;
    Mat& srcImage = *(data->srcImage);
    Mat& dstImage = *(data->dstImage);
    int startRow = data->startRow;
    int endRow = data->endRow;

    for(int row = startRow; row < endRow; ++row)
    {
        for(int col = 0; col < srcImage.cols; ++col)
        {
            dstImage.at<Vec3b>(row, col) = srcImage.at<Vec3b>(row, srcImage.cols - 1 - col);
        }
    }

    pthread_exit(NULL);
}

void parallelProcess(Mat& srcImage, Mat& dstImage, void* (*filterFunc)(void*), int numThreads)
{
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    int rowsPerThread = srcImage.rows / numThreads;

    for (int i = 0; i < numThreads; ++i)
    {
        threadData[i].srcImage = &srcImage;
        threadData[i].dstImage = &dstImage;
        threadData[i].startRow = i * rowsPerThread;
        threadData[i].endRow = (i == numThreads - 1) ? srcImage.rows : (i + 1) * rowsPerThread;
        pthread_create(&threads[i], NULL, filterFunc, (void*)&threadData[i]);
    }

    for (int i = 0; i < numThreads; ++i)
    {
        pthread_join(threads[i], NULL);
    }
}

pair<Mat, Mat> passToThread(Mat& img_color, Mat& img_gray)
{
    // Horizontal Mirror
    Mat img_horizontal_mirror = img_color.clone();
    parallelProcess(img_color, img_horizontal_mirror, horizontalMirror, MAX_NUM_THREADS);

    // Vertical Mirror
    Mat img_vertical_mirror = img_horizontal_mirror.clone();
    parallelProcess(img_horizontal_mirror, img_vertical_mirror, verticalMirror, MAX_NUM_THREADS);

    // Sepia 
    Mat img_sepia = img_vertical_mirror.clone();
    parallelProcess(img_vertical_mirror, img_sepia, sepiaFilter, MAX_NUM_THREADS);

    // Box
    Mat img_box = img_gray.clone();
    parallelProcess(img_gray, img_box, boxFilter, MAX_NUM_THREADS);

    // Gray to BW
    Mat img_bw = img_box.clone();
    parallelProcess(img_box, img_bw, grayToBW, MAX_NUM_THREADS);

    // Edge Detection
    Mat img_edge = img_bw.clone();
    parallelProcess(img_bw, img_edge, LaplacianEdgeDetection, MAX_NUM_THREADS);

    return make_pair(img_sepia, img_edge);
}

int main(int argc, char* argv[])
{
    auto start = std::chrono::high_resolution_clock::now();
    
    if(argc != 2)
    {
        cout << "Bad arguments!" << endl;
        exit(EXIT_FAILURE);
    }

    const string FILE_PATH(argv[1]);
    Mat img_color, img_gray; 
    img_color = imread(FILE_PATH, ImreadModes::IMREAD_ANYCOLOR);
    cvtColor(img_color, img_gray, 
        ColorConversionCodes::COLOR_BGR2GRAY);

    if(img_color.empty() || img_gray.empty())
    {
        cout << "input image empty" << endl;
        exit(EXIT_FAILURE);
    }

    pair<Mat, Mat> result = passToThread(img_color, img_gray);
    Mat img_sepia = result.first;
    Mat img_edge = result.second;

    imwrite("First_Parallel.bmp", img_sepia);
    imwrite("Second_Parallel.bmp", img_edge);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << "Execution Time: " << duration.count() / 1000.0 << endl;

    return 0;
}