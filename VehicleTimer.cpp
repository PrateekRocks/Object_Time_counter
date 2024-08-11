#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include<stdio.h>
#include <ctime>
#include <chrono>
#include <fstream>

// Namespaces.
using namespace cv;
using namespace std;
using namespace cv::dnn;

clock_t timeStart, timeStop, oldtime;

clock_t TimeStart, TimeStop;
double LastTimmer = 0.0;

int timmer = 0;
int lastTimmer = 0;

struct Inzone{
    clock_t Intime;
    int id;


    bool operator==(const Inzone& other) const {
    return (Intime == other.Intime && id == other.id);
    }    

    

};

vector<Inzone> InzoneVehicle;

vector<Point> box1 = {Point(789, 465), Point(1089, 462), Point(1101, 781), Point(784, 781)};
// vector<Point> box2 = {Point(1330, 697), Point(1732, 697), Point(1734, 997), Point(1382, 991)};


int distanceThreshold = 100;
map<int, pair<int,int>> object;
int objectcount = 0;

vector<Mat> detect(Mat frame , Net &net ){

    Mat blob;
    blobFromImage(frame, blob, 1/255.0, Size(416, 416), Scalar(), true, false);

    net.setInput(blob);

    vector<String> layerNames = net.getUnconnectedOutLayersNames();

    vector<Mat> outs;
    net.forward(outs, layerNames);


    return outs;


}

float caldistance(int x1, int y1, int x2, int y2){
    int distance = sqrt(pow((x1-x2), 2) + pow((y1 - y2), 2));
    return distance;
}


pair<int , pair<int, int>> getid(Rect box){

    
    int X = (box.x + box.x + box.width) / 2;
    int Y = (box.y + box.y + box.height) / 2;

    for (const auto& pair : object) {
          if (pair.second == make_pair(X, Y)) {           
         
            return pair;
        }

    }


    return {}; 
}

void TrackInZone(Inzone target){
   
   
    if (InzoneVehicle.empty()){
        InzoneVehicle.push_back(target);
    }else
    {

        auto it = find(InzoneVehicle.begin(), InzoneVehicle.end(), target);

        if (it != InzoneVehicle.end()) {
            it->Intime = clock();
                    

        } else {
            InzoneVehicle.push_back(target);
        }



    }
}

void Track(Rect box){

    int CenterX = (box.x + box.x + box.width) / 2;
    int CenterY = (box.y + box.y + box.height) / 2;
    
    bool updated = false;
    if (object.empty()){
        object[objectcount] = make_pair(CenterX,CenterY);
        objectcount++;

    }
    else{

        for(int i=0; i < object.size(); i++){

            float distance = caldistance(CenterX,CenterY,object[i].first,object[i].second);
            if(distance < distanceThreshold){
                object[i] = make_pair(CenterX,CenterY);
                updated = true;
            }

        }
        if(!updated){
            ++objectcount;
            object[objectcount] = make_pair(CenterX,CenterY);
        }

    }
    if(  pointPolygonTest(box1,Point(CenterX,CenterY), false) > 0){
        // TrackInZone({clock(),objectcount});
       
    }



}


string formatTime(int seconds) {
    int minutes = seconds / 60;
    seconds = seconds % 60;
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
    return string(buffer);
}



void checkInZone(Mat frame, pair<int, pair<int, int>> result) {
    Point center = Point(result.second.first, result.second.second);
    Point showtime = box1[0];

    // Find the target in the InzoneVehicle vector
    auto it = find_if(InzoneVehicle.begin(), InzoneVehicle.end(), [&](const Inzone& inzone) {
        return inzone.id == result.first;
    });

    if (pointPolygonTest(box1, center, false) > 0) {
        if (it != InzoneVehicle.end()) {
            // Update the time spent in the zone
            clock_t now = clock();
            double elapsedTime = double(now - it->Intime) / CLOCKS_PER_SEC;
            int elapsedSeconds = static_cast<int>(elapsedTime);
            putText(frame, formatTime(elapsedSeconds), Point(showtime.x + 65, showtime.y + 25), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 255), 2);

        } else {
            // Add the object to InzoneVehicle with the current time
            InzoneVehicle.push_back({clock(), result.first});
            putText(frame, "00:00", Point(showtime.x + 65, showtime.y + 25), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 255), 2);
        }
    } else {
        if (it != InzoneVehicle.end()) {
            // The object left the zone, remove it from InzoneVehicle
            InzoneVehicle.erase(it);
        }
    }
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
   
    polylines(frame, box1, true, Scalar(255, 255, 0), 2);

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
        Track(box);
        rectangle(frame, box, Scalar(0, 255, 0), 2);

        String label = format("%.2f", confidences[idx]);
        label = format("%d: %s", classIds[idx], label.c_str());

        pair<int, pair<int, int>> result = getid(box);

        int baseLine;
        Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        int top = max(box.y, labelSize.height);
        rectangle(frame, Point(box.x, top - labelSize.height),
                    Point(box.x + labelSize.width, top + baseLine),
                    Scalar(255, 255, 255), FILLED);
        putText(frame, label, Point(box.x, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1);
        circle(frame, Point(result.second.first,result.second.second), 2, Scalar(0, 0, 1), 2);
        // putText(frame, to_string(result.first), Point(result.second.first,result.second.second-5), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 1), 3);

        checkInZone(frame , result );
        // bool vehicleInside = false; // Variable to track if the vehicle is inside the zone
        // checkInZone(frame, result, vehicleInside);
    }
   
    // polylines(frame, box2, true, Scalar(255, 0, 255), 2); 

     

    
    return frame; 

}




int main()
{

auto net = readNet("Models/yolov4-tiny.cfg", "Models/yolov4-tiny.weights");

net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

VideoCapture cap("Models/Video.mp4"); 

timeStart = clock(); 

if (!cap.isOpened()) {
    cout << "Error: Could not open video." << endl;
    return -1;
}

while (cap.isOpened()) {

   
    Mat frame;
    cap >> frame; 


    if (frame.empty()) {
        cout << "End of video." << endl;
        break;
    }
    
    auto detection = detect(frame,net);
    Mat annotatedframe = getbox(frame,detection);


    // resize(annotatedframe, annotatedframe, cv::Size(1000, 1000));
    imshow("Vehicle Monitoring", annotatedframe);  

    // timeStop = clock();
    // double elapsedTime = double(timeStop - timeStart) / CLOCKS_PER_SEC;

    // if (elapsedTime >= (lastTimmer + 1)) {
    //         timmer++;
    //         // lastTimmer = timmer;
    // }

    int key = cv::waitKey(1); 
    if (key == 'q') {
        break; 
    }

    }

cap.release();
destroyAllWindows();


return 0;


}