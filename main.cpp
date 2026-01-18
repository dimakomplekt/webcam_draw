#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>


#define TRAILS_LIFETIME 1.0


// Цвета реальных маркеров
std::vector<std::vector<int>> my_irl_marker_colors {

    // h_min, h_max, s_min, s_max, v_min, v_max

    {0, 20, 180, 255, 0, 255}, // Orange
    {16, 179, 142, 255, 112, 255} // Yellow

};


// Цвета маркеров на цифровой доске
std::vector<cv::Scalar> my_digital_colors {

    {0, 165, 255}, // Orange (B, G, R)
    {0, 255, 255}  // Yellow (B, G, R)

};


// Цифровая доска
cv::Mat desk;


// Следы движения маркера
std::vector<std::vector<cv::Point>> trails; 
// Время жизни точек
std::vector<std::vector<int64>> trail_times;


// redraw_on_desk читает trails[] и рисует след
void redraw_on_desk(cv::Mat &current_desk, cv::Mat frame);

// Находит точки, кладёт их в trails[]
void find_color(cv::Mat frame);


int main()
{
    // Захват вебкамеры
    cv::VideoCapture cap(0);

    if (!cap.isOpened())
    {
        std::cout << "Камера не открылась!" << std::endl;
        return -1;
    }

    // Окно для рендера изображения
    cv::Mat image;


    // Ресайз векторов
    trails.resize(my_irl_marker_colors.size());
    trail_times.resize(my_irl_marker_colors.size());

    // Перенос изображения на окно
    cap.read(image);
    if (image.empty()) return -1;

    // Создание доски
    desk = cv::Mat::zeros(image.size(), image.type());


    // Цикл показа
    while (true)
    {
        cap.read(image);
        if (image.empty()) break; 

        // Рендер
        redraw_on_desk(desk, image);


        // Выход из лупа
        if (cv::waitKey(1) == 27) break;
    }


    // Закрытие программы
    cap.release();
    cv::destroyAllWindows();

    return 0;
}



void redraw_on_desk(cv::Mat &current_desk, cv::Mat frame)
{
    /*
    
    Хранит историю точек на 3 секунды,

    перерисовывает стол каждый кадр,

    рисует красивый след.
        
    */


    /*
        Очистка стола на каждом кадре

        0.95 — оставить 95% старого рисунка

        0.05 — подмешать 5% чёрного

        След не исчезает мгновенно, а плавно тает.
    */

    cv::Mat fade = cv::Mat::zeros(current_desk.size(), current_desk.type());
    cv::addWeighted(current_desk, 0.95, fade, 0.05, 0, current_desk);

    // Поиск цветов в фрейме
    find_color(frame);

    // Время захвата и частота кадров
    int64 now = cv::getTickCount();
    double freq = cv::getTickFrequency();

    // Рисование следа для всех цветов
    for (int i = 0; i < trails.size(); i++)
    {
        // Для каждого следа по его точкам, зная время его фиксации
        auto &pts = trails[i];
        auto &times = trail_times[i];

        // Берем цвет
        cv::Scalar color = my_digital_colors[i];

        // Берем все его точки
        for (int k = 0; k < pts.size(); )
        {
            // Чек прошедшего времени
            double elapsed = (now - times[k]) / freq;

            // Удаление старых следов
            if (elapsed > TRAILS_LIFETIME)
            {
                pts.erase(pts.begin() + k);
                times.erase(times.begin() + k);
                continue;
            }

            // Рисование новых следов
            cv::circle(current_desk, pts[k], 5, color, -1);
            k++;
        }


        // Если точек несколько - рисуем между ними линию
        if (pts.size() > 1)
        {
            for (int t = 1; t < pts.size(); t++)
            {
                cv::line(current_desk, pts[t - 1], pts[t], color, 5);
            }
        }
    }

    // Рендер изображения

    cv::Mat desk_scaled; // Увеличенная доска

    cv::resize(current_desk, desk_scaled, cv::Size(), 2.0, 2.0); // Увеличение доски относительно веб-камеры

    cv::flip(frame, frame, 1); // Переворот первичного изображения - чтобы рисовалось, как рисуем, а не перевернуто
    
    cv::imshow("Webcam", frame); // Изображение с вебкамеры
    // cv::imshow("Desk (raw)", current_desk); // Тест маски
    
    cv::imshow("Desk (scaled)", desk_scaled); // Наша цифровая доска
}


void find_color(cv::Mat frame)
{
    /*

        Переводит кадр в HSV,

        Ищет каждый цвет из my_irl_marker_colors,

        Находит центр маркера (точку), для каждой найденной точки вызывает redraw_on_desk.
        
    */

    // Первичный поворот с отображением в hsv
    cv::flip(frame, frame, 1);

    cv::Mat hsv_image;

    cv::cvtColor(frame, hsv_image, cv::COLOR_BGR2HSV);

    // Детекция для всех цветов из контейнера с цветами
    for (int i = 0; i < my_irl_marker_colors.size(); i++)
    {
        auto &color = my_irl_marker_colors[i];

        // Настройки маски по выделенным цветам
        cv::Scalar lower(color[0], color[2], color[4]);
        cv::Scalar upper(color[1], color[3], color[5]);

        cv::Mat mask;
        
        cv::inRange(hsv_image, lower, upper, mask);

        // Поиск контуров заданного цвета
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        if (contours.empty()) continue;

        // Фильтрация
        int max_idx = 0;
        double max_area = 100;

        for (int j = 0; j < contours.size(); j++)
        {
            double area = cv::contourArea(contours[j]);

            if (area > max_area)
            {
                max_area = area;
                max_idx = j;
            }
        }


        // Ищем центр маркера используя моментс

        cv::Moments m = cv::moments(contours[max_idx]);

        // m.m00 — это площадь пятна.
        // Если она ноль — значит ничего не найдено → пропускаем.
        if (m.m00 == 0) continue;

        // Центр по X:
        // сумма (x * масса) / общая масса
        // То есть «среднее x всех пикселей пятна».
        int cx = int (m.m10 / m.m00);
        // Так же
        int cy = int(m.m01 / m.m00);

        
        // Добавляем точку в след и инициируем её время жизни
        if (trails[i].empty() || cv::norm(trails[i].back() - cv::Point(cx, cy)) > 2) // Защита от дребезга
        {
            trails[i].push_back(cv::Point(cx, cy));
            trail_times[i].push_back(cv::getTickCount());
        }
    }
}
