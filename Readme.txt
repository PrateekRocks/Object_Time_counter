Hello 
I am Prateek 

This assgment is timer running on the ROI for any vehicle that is present inside ROI







docker build -t vehicletimer .




FROM ubuntu:18.04

# Install requirements
RUN apt-get update && apt-get install -y \
    # OpenCV dependencies
    build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev \
    python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev \
    libcanberra-gtk-module libcanberra-gtk3-module

# Clone, build and install OpenCV
RUN git clone https://github.com/opencv/opencv.git && \
    cd /opencv && mkdir build && cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j"$(nproc)" && \
    make install && \
    rm -rf /opencv

RUN export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

RUN mkdir /home/Object_Time_counter

WORKDIR /home/Object_Time_counter
exit
RUN g++ -o VehicleTimer VehicleTimer.cpp -I/usr/local/include/opencv4  -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_dnn -lopencv_videoio -lopencv_imgproc

RUN export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

CMD [ "run.sh" ]



docker run command  :  docker run  -v /tmp/.X11-unix:/tmp/.X11-unix -v $(pwd):/home/Object_Time_counter -e DISPLAY=$DISPLAY -p 5000:5000 -p 8888:8888 -it vehicletimer
                     
 g++ -o VehicleTimer VehicleTimer.cpp -I/usr/local/include/opencv4  -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_dnn -lopencv_videoio -lopencv_imgproc



