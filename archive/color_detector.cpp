#include <iostream>
#include <opencv2/opencv.hpp>

cv::Mat image_hsv;
cv::Mat image_mask;

int h_min = 0, s_min = 0, v_min = 0;
int h_max = 179, s_max = 255, v_max = 255;


int h_min_prev = h_min, s_min_prev = s_min, v_min_prev = v_min;
int h_max_prev = h_max, s_max_prev = s_max, v_max_prev = v_max;


int main()
{
    cv::Mat image;
    cv::VideoCapture cap(0);

    if (!cap.isOpened())
    {
        std::cout << "Камера не открылась!" << std::endl;
        return -1;
    }

    cv::namedWindow("Color limits", cv::WINDOW_NORMAL);
    cv::resizeWindow("Color limits", 640, 200);

    cv::createTrackbar("Hue min", "Color limits", &h_min, 179);
    cv::createTrackbar("Hue max", "Color limits", &h_max, 179);
    cv::createTrackbar("Sat min", "Color limits", &s_min, 255);
    cv::createTrackbar("Sat max", "Color limits", &s_max, 255);
    cv::createTrackbar("Val min", "Color limits", &v_min, 255);
    cv::createTrackbar("Val max", "Color limits", &v_max, 255);

    while (true)
    {
        cap.read(image);

        if (image.empty()) break;

        // Обработка на каждом кадре
        cv::cvtColor(image, image_hsv, cv::COLOR_BGR2HSV);

        cv::Scalar lower_color(h_min, s_min, v_min);
        cv::Scalar upper_color(h_max, s_max, v_max);

        cv::inRange(image_hsv, lower_color, upper_color, image_mask);

        cv::imshow("Image", image);
        cv::imshow("Image mask", image_mask);


        if (h_min != h_min_prev || h_max != h_max_prev ||
            s_min != s_min_prev || s_max != s_max_prev ||
            v_min != v_min_prev || v_max != v_max_prev)
        {
            h_min_prev = h_min;
            h_max_prev = h_max;
            s_min_prev = s_min;
            s_max_prev = s_max;
            v_min_prev = v_min;
            v_max_prev = v_max;

            std::cout << h_min << "," << h_max << ","
            << s_min << "," << s_max << ","
            << v_min << "," << v_max << std::endl;
        }

        if (cv::waitKey(1) == 27) break; // ESC для выхода
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
