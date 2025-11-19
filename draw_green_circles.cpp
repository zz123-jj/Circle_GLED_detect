#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <random>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    // 设置随机数生成器
    random_device rd;
    mt19937 gen(rd());
    
    // 创建画布
    int width = 1280;
    int height = 720;
    Mat canvas(height, width, CV_8UC3, Scalar(40, 40, 40));
    
    // 生成随机绿色圆形
    int num_circles = 5 + (gen() % 11);  // 5-15个圆形
    
    cout << "生成 " << num_circles << " 个绿色圆形..." << endl;
    
    uniform_int_distribution<> x_dist(100, width - 100);
    uniform_int_distribution<> y_dist(100, height - 100);
    uniform_int_distribution<> radius_dist(20, 80);
    uniform_int_distribution<> green_dist(100, 255);
    uniform_int_distribution<> brightness_dist(150, 255);
    
    for (int i = 0; i < num_circles; i++) {
        // 随机位置和大小
        Point center(x_dist(gen), y_dist(gen));
        int radius = radius_dist(gen);
        
        // 随机绿色色调（更偏绿色）
        int green = brightness_dist(gen);
        int red = green_dist(gen) / 3;
        int blue = green_dist(gen) / 3;
        Scalar color(blue, green, red);
        
        // 绘制实心圆
        circle(canvas, center, radius, color, -1, LINE_AA);
        
        // 添加高光效果，使其更像灯
        Scalar highlight(brightness_dist(gen), 255, brightness_dist(gen));
        circle(canvas, center, radius / 3, highlight, -1, LINE_AA);
        
        // 添加外圈
        circle(canvas, center, radius, Scalar(0, 255, 0), 2, LINE_AA);
        
        cout << "圆形 " << (i + 1) << ": 中心(" << center.x << ", " << center.y 
             << "), 半径=" << radius << endl;
    }
    
    // 添加标题
    putText(canvas, "Green Circle Lights Test Image", 
            Point(50, 50), FONT_HERSHEY_SIMPLEX, 1.2, 
            Scalar(255, 255, 255), 2, LINE_AA);
    
    // 添加时间戳
    time_t now = time(0);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    putText(canvas, timestamp, Point(50, height - 30), 
            FONT_HERSHEY_SIMPLEX, 0.7, Scalar(200, 200, 200), 1, LINE_AA);
    
    // 保存图像
    string filename = "green_circles_test.jpg";
    bool saved = imwrite(filename, canvas);
    
    if (saved) {
        cout << "\n图像已保存到: " << filename << endl;
        cout << "图像尺寸: " << width << "x" << height << endl;
    } else {
        cout << "错误: 无法保存图像" << endl;
        return -1;
    }
    
    // 显示图像
    namedWindow("绿色圆形测试图像", WINDOW_AUTOSIZE);
    imshow("绿色圆形测试图像", canvas);
    
    cout << "\n按任意键关闭窗口..." << endl;
    waitKey(0);
    destroyAllWindows();
    
    return 0;
}
