#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include<stdio.h>
#include <fstream>

// Namespaces.
using namespace cv;
using namespace std;
using namespace cv::dnn;


// vector<int,int> box1 = [(786,670),(1113,670),(1111,994),(782,996)];
// vector<int,int> box2 = [(1330,697),(1732,697),(1734,997),(1382,991)];

vector<Mat> detect(Mat frame , Net &net ){

    Mat blob;
    blobFromImage(frame, blob, 1/255.0, Size(416, 416), Scalar(), true, false);

    net.setInput(blob);

    vector<String> layerNames = net.getUnconnectedOutLayersNames();

    vector<Mat> outs;
    net.forward(outs, layerNames);


    return outs;


}


vector<string> loadClassNames(const string& filename) {
    vector<string> class_names;
    ifstream ifs(filename.c_str());
    string line;
    while (getline(ifs, line)) {
        class_names.push_back(line);
    }
    return class_names;
}



Mat getbox(Mat frame, vector<Mat> &outs ){

    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;
    float confThreshold = 0.5; 
    float nmsThreshold = 0.4;  

    vector<string> class_name =  loadClassNames("Models/yolov4.names");
   

    for (int i = 0; i < outs.size(); ++i) {
    
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;

            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);

            if (confidence > confThreshold) {
            
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

            
                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(Rect(left, top, width, height));
            }
        }
    }

    // Apply non-maxima suppression to eliminate redundant overlapping boxes
    vector<int> indices;
    dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
     
    for(int i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        Rect box = boxes[idx];
        rectangle(frame, box, Scalar(0, 255, 0), 2);

        String label = format("%.2f", confidences[idx]);
        label = format("%d: %s", classIds[idx], label.c_str());

        int baseLine;
        Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        int top = max(box.y, labelSize.height);
        rectangle(frame, Point(box.x, top - labelSize.height),
                    Point(box.x + labelSize.width, top + baseLine),
                    Scalar(255, 255, 255), FILLED);
        putText(frame, label, Point(box.x, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1);
    }

    return frame; 

}


int main()
{

auto net = readNet("Models/yolov4-tiny.cfg", "Models/yolov4-tiny.weights");

net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

VideoCapture cap("Models/Video.mp4");  

if (!cap.isOpened()) {
    cout << "Error: Could not open video." << endl;
    return -1;
}

while (cap.isOpened()) {
    Mat frame;
    cap >> frame; 

    cout<<frame.size();    

    if (frame.empty()) {
        cout << "End of video." << endl;
        break;
    }


    auto detection = detect(frame,net);
    Mat annotatedframe = getbox(frame,detection);


    resize(annotatedframe, annotatedframe, cv::Size(600, 600));
    imshow("Vehicle Monitoring", annotatedframe);   


    int key = cv::waitKey(1); 
    if (key == 'q') {
        break; 
    }

    }

cap.release();
destroyAllWindows();


return 0;


}