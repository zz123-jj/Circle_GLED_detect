#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    // 检查命令行参数
    if (argc != 2) {
        cout << "使用方法: " << argv[0] << " <视频文件路径>" << endl;
        return -1;
    }

    // 打开视频文件
    VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        cout << "错误: 无法打开视频文件 " << argv[1] << endl;
        return -1;
    }

    // 获取视频属性
    int frame_width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(CAP_PROP_FPS);
    
    cout << "视频分辨率: " << frame_width << "x" << frame_height << endl;
    cout << "帧率: " << fps << " FPS" << endl;

    // 创建视频写入器
    string output_filename = "detected_output.avi";
    int fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G');
    VideoWriter writer(output_filename, fourcc, fps, Size(frame_width, frame_height));
    
    if (!writer.isOpened()) {
        cout << "错误: 无法创建输出视频文件" << endl;
        return -1;
    }
    
    cout << "输出视频将保存为: " << output_filename << endl;

    // 创建窗口
    namedWindow("原始视频", WINDOW_AUTOSIZE);
    namedWindow("检测结果", WINDOW_AUTOSIZE);

    Mat frame, hsv, mask, result;
    
    while (true) {
        // 读取帧
        cap >> frame;
        if (frame.empty()) {
            cout << "视频播放完毕" << endl;
            break;
        }

        // 转换到HSV色彩空间
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // 定义绿色的HSV范围
        // 绿色通常在HSV中H值范围为35-85
        Scalar lower_green1 = Scalar(35, 40, 40);
        Scalar upper_green1 = Scalar(85, 255, 255);
        
        // 创建绿色掩码
        inRange(hsv, lower_green1, upper_green1, mask);

        // 形态学操作去除噪声
        Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
        morphologyEx(mask, mask, MORPH_OPEN, kernel);
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);

        // 高斯模糊减少噪声
        GaussianBlur(mask, mask, Size(9, 9), 2, 2);

        // 使用霍夫圆变换检测圆形
        vector<Vec3f> circles;
        HoughCircles(mask, circles, HOUGH_GRADIENT, 1,
                     mask.rows / 8,  // 圆心之间的最小距离
                     100,            // Canny边缘检测的高阈值
                     30,             // 累加器阈值
                     10, 200);       // 最小和最大半径

        // 复制原始帧用于绘制结果
        result = frame.clone();

        // 绘制检测到的圆形和边界框
        for (size_t i = 0; i < circles.size(); i++) {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            
            // 绘制圆心
            circle(result, center, 3, Scalar(0, 255, 255), -1, LINE_AA);
            
            // 绘制圆形轮廓
            circle(result, center, radius, Scalar(0, 255, 0), 3, LINE_AA);
            
            // 绘制矩形边界框
            Rect boundingBox(center.x - radius, center.y - radius, 
                           radius * 2, radius * 2);
            rectangle(result, boundingBox, Scalar(255, 0, 0), 2, LINE_AA);
            
            // 添加文字标签
            string label = "Green Light";
            putText(result, label, 
                   Point(center.x - radius, center.y - radius - 10),
                   FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
        }

        // 显示检测到的圆形数量
        string count_text = "Detected: " + to_string(circles.size());
        putText(result, count_text, Point(10, 30),
               FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);

        // 显示结果
        imshow("原始视频", frame);
        imshow("检测结果", result);

        // 将标注后的帧写入输出视频
        writer.write(result);

        // 按ESC或q键退出
        int key = waitKey(30);
        if (key == 27 || key == 'q') {
            cout << "用户停止播放" << endl;
            break;
        }
    }

    // 释放资源
    cap.release();
    writer.release();
    destroyAllWindows();

    cout << "检测完成！标注后的视频已保存到: " << output_filename << endl;

    return 0;
}
