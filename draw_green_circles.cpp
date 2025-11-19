#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <random>
#include <cmath>

using namespace cv;
using namespace std;

// 结构体存储圆形信息
struct Circle {
    Point center;
    int radius;
    int brightness;
};

// 检查两个圆是否重叠
bool isOverlapping(const Circle& c1, const Circle& c2) {
    double distance = sqrt(pow(c1.center.x - c2.center.x, 2) + 
                          pow(c1.center.y - c2.center.y, 2));
    return distance < (c1.radius + c2.radius + 10);  // 加10像素间距
}

// 检查圆是否与已有圆重叠
bool checkOverlap(const Circle& newCircle, const vector<Circle>& circles) {
    for (const auto& circle : circles) {
        if (isOverlapping(newCircle, circle)) {
            return true;
        }
    }
    return false;
}

int main(int argc, char** argv) {
    // 设置随机数生成器
    random_device rd;
    mt19937 gen(rd());
    
    // 创建画布
    int width = 1280;
    int height = 720;
    Mat canvas(height, width, CV_8UC3, Scalar(40, 40, 40));
    
    // 目标生成圆形数量
    int target_circles = 8 + (gen() % 8);  // 8-15个圆形
    
    cout << "尝试生成 " << target_circles << " 个不重叠的绿色圆形..." << endl;
    
    // 存储已生成的圆形
    vector<Circle> circles;
    
    uniform_int_distribution<> radius_dist(25, 70);
    uniform_int_distribution<> brightness_dist(80, 255);
    
    int max_attempts = 1000;  // 最大尝试次数
    int attempts = 0;
    
    while (circles.size() < target_circles && attempts < max_attempts) {
        attempts++;
        
        // 生成随机半径
        int radius = radius_dist(gen);
        
        // 确保圆在画布内
        uniform_int_distribution<> x_dist(radius + 20, width - radius - 20);
        uniform_int_distribution<> y_dist(radius + 20, height - radius - 20);
        
        // 生成随机位置
        Circle newCircle;
        newCircle.center = Point(x_dist(gen), y_dist(gen));
        newCircle.radius = radius;
        newCircle.brightness = brightness_dist(gen);
        
        // 检查是否与已有圆重叠
        if (!checkOverlap(newCircle, circles)) {
            circles.push_back(newCircle);
            cout << "圆形 " << circles.size() << ": 中心(" << newCircle.center.x 
                 << ", " << newCircle.center.y << "), 半径=" << newCircle.radius
                 << ", 亮度=" << newCircle.brightness << endl;
        }
    }
    
    cout << "\n成功生成 " << circles.size() << " 个不重叠的圆形" << endl;
    
    // 绘制所有圆形
    uniform_real_distribution<> angle_dist(0, 2 * M_PI);
    uniform_real_distribution<> glow_dist(0.3, 0.8);
    
    for (const auto& c : circles) {
        // 1. 绘制外层光晕（模拟发光扩散）
        int glow_radius = c.radius + 15;
        for (int layer = 0; layer < 3; layer++) {
            int current_radius = glow_radius - layer * 5;
            int alpha = c.brightness / (3 + layer * 2);
            Scalar glow_color(alpha / 5, alpha, alpha / 5);
            cv::circle(canvas, c.center, current_radius, glow_color, -1, LINE_AA);
        }
        
        // 2. 添加不规则光斑效果
        uniform_int_distribution<> spot_count_dist(8, 15);
        int num_spots = spot_count_dist(gen);
        
        for (int i = 0; i < num_spots; i++) {
            double angle = angle_dist(gen);
            double distance = c.radius * glow_dist(gen);
            int spot_x = c.center.x + static_cast<int>(distance * cos(angle));
            int spot_y = c.center.y + static_cast<int>(distance * sin(angle));
            
            uniform_int_distribution<> spot_size_dist(2, 6);
            int spot_size = spot_size_dist(gen);
            int spot_brightness = c.brightness * (0.4 + glow_dist(gen) * 0.4);
            
            Scalar spot_color(spot_brightness / 4, spot_brightness, spot_brightness / 4);
            cv::circle(canvas, Point(spot_x, spot_y), spot_size, spot_color, -1, LINE_AA);
        }
        
        // 3. 绘制主圆形
        int green = c.brightness;
        int red = c.brightness / 4;
        int blue = c.brightness / 4;
        Scalar color(blue, green, red);
        cv::circle(canvas, c.center, c.radius, color, -1, LINE_AA);
        
        // 4. 添加不规则边缘（模拟灯罩纹理）
        uniform_int_distribution<> edge_points_dist(20, 30);
        int num_edge_points = edge_points_dist(gen);
        
        for (int i = 0; i < num_edge_points; i++) {
            double angle = (2 * M_PI * i) / num_edge_points;
            uniform_real_distribution<> noise_dist(-3, 3);
            int noise = static_cast<int>(noise_dist(gen));
            
            int edge_radius = c.radius + noise;
            int x = c.center.x + static_cast<int>(edge_radius * cos(angle));
            int y = c.center.y + static_cast<int>(edge_radius * sin(angle));
            
            int edge_brightness = c.brightness * 0.7;
            Scalar edge_color(edge_brightness / 5, edge_brightness, edge_brightness / 5);
            cv::circle(canvas, Point(x, y), 2, edge_color, -1, LINE_AA);
        }
        
        // 5. 添加中心高光（最亮区域）
        int highlight_brightness = min(255, c.brightness + 50);
        Scalar highlight(highlight_brightness / 3, highlight_brightness, highlight_brightness / 3);
        
        // 不规则高光
        uniform_int_distribution<> highlight_offset(-3, 3);
        Point highlight_center(c.center.x + highlight_offset(gen), 
                              c.center.y + highlight_offset(gen));
        cv::circle(canvas, highlight_center, c.radius / 3, highlight, -1, LINE_AA);
        
        // 6. 添加光线效果（模拟光源辐射）
        uniform_int_distribution<> ray_count_dist(4, 8);
        int num_rays = ray_count_dist(gen);
        
        for (int i = 0; i < num_rays; i++) {
            double ray_angle = angle_dist(gen);
            int ray_length = c.radius + 10 + (gen() % 15);
            
            Point ray_end(
                c.center.x + static_cast<int>(ray_length * cos(ray_angle)),
                c.center.y + static_cast<int>(ray_length * sin(ray_angle))
            );
            
            int ray_brightness = c.brightness * 0.3;
            Scalar ray_color(ray_brightness / 6, ray_brightness, ray_brightness / 6);
            cv::line(canvas, c.center, ray_end, ray_color, 1, LINE_AA);
        }
        
        // 7. 添加外圈轮廓（略微不规则）
        Scalar outline(0, min(255, c.brightness + 30), 0);
        cv::circle(canvas, c.center, c.radius, outline, 2, LINE_AA);
    }
    
    
    // 添加标题
    string title = "Green Circle Lights - " + to_string(circles.size()) + " circles";
    putText(canvas, title, 
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
