#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

const int KERNEL_MATRIX[3][3] = {
    {0, 1, 0},
    {1, -4, 1},
    {0, 1, 0}
};


void LaplacianEdgeDetection(Mat image)
{
    Mat edge_img = image.clone();
    for(int row = 1; row < image.rows - 1; ++row)
    {
        for(int col = 1; col < image.cols - 1; ++col)
        {
            int sum = 0;
            for(int i = -1; i <= 1; ++i)
            {
                for(int j = -1; j <= 1; ++j)
                {
                    sum += image.at<uchar>(row + i, col + j) * KERNEL_MATRIX[i + 1][j + 1];
                    edge_img.at<uchar>(row, col) = (sum > 255) ? 255 : (sum < 0) ? 0 : sum;
                }
            }
        }
    }
    imwrite("Second_Serial.bmp", edge_img);
    edge_img.release();
}

void grayToBW(Mat image)
{
    Mat bw_img = image.clone();
    for(int row = 0; row < image.rows; ++row)
    {
        for(int col = 0; col < image.cols; ++col)
        {
            bw_img.at<uchar>(row, col) = (image.at<uchar>(row, col) > 127) ? 255 : 0;
        }
    }
    LaplacianEdgeDetection(bw_img);
}

void boxFilter(Mat image)
{
    Mat box_img = image.clone();
    for(int row = 1; row < image.rows - 1; ++row)
    {
        for(int col = 1; col < image.cols - 1; ++col)
        {
            int sum = 0;
            for(int i = -1; i <= 1; ++i)
            {
                for(int j = -1; j <= 1; ++j)
                {
                    sum += image.at<uchar>(row + i, col + j);
                }
            }
            box_img.at<uchar>(row, col) = sum / 9;
        }
    }
    grayToBW(box_img);
}

void sepiaFilter(Mat image)
{
    Mat sepia_img = image.clone();

    for (int row = 0; row < image.rows; ++row)
    {
        for (int col = 0; col < image.cols; ++col)
        {
            Vec3b pixel = image.at<Vec3b>(row, col);

            uchar originalBlue = pixel[0];
            uchar originalGreen = pixel[1];
            uchar originalRed = pixel[2];

            int newRed = static_cast<int>(0.393 * originalRed + 0.769 * originalGreen + 0.189 * originalBlue);
            int newGreen = static_cast<int>(0.349 * originalRed + 0.686 * originalGreen + 0.168 * originalBlue);
            int newBlue = static_cast<int>(0.272 * originalRed + 0.534 * originalGreen + 0.131 * originalBlue);

            image.at<Vec3b>(row, col)[2] = (newRed > 255) ? 255 : newRed;
            image.at<Vec3b>(row, col)[1] = (newGreen > 255) ? 255 : newGreen;
            image.at<Vec3b>(row, col)[0] = (newBlue > 255) ? 255 : newBlue;
        }
    }
    imwrite("First_Serial.bmp", image);
    image.release();
}

void verticalMirror(Mat image) 
{
    Mat vertical_img = image.clone();
    for(int row = 0; row < image.rows; ++row)
    {
        for(int col = 0; col < image.cols; ++col)
        {
            vertical_img.at<Vec3b>(row, col) = image.at<Vec3b>(-row + image.rows - 1, col);
        }
    }
    sepiaFilter(vertical_img);
}

void horizontalMirror(Mat image) 
{
    Mat horizontal_img = image.clone();
    for(int row = 0; row < image.rows; ++row)
    {
        for(int col = 0; col < image.cols; ++col)
        {
            horizontal_img.at<Vec3b>(row, col) = image.at<Vec3b>(row, -col + image.cols - 1);
        }
    }
    verticalMirror(horizontal_img);
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
    Mat imgcpy = img_color.clone();
    cvtColor(img_color, img_gray,
        ColorConversionCodes::COLOR_BGR2GRAY);

    if(img_color.empty() || img_gray.empty())
    {
        cout << "input image empty" << endl;
        exit(EXIT_FAILURE);
    }

    horizontalMirror(imgcpy);
    boxFilter(img_gray);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << "Execution Time: " << duration.count() / 1000.0 << endl;
}