FROM ubuntu:18.04

# Install requirements
RUN apt-get update && apt-get install -y \
    # OpenCV dependencies
    build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev \
    python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev \
    libcanberra-gtk-module libcanberra-gtk3-module \
    gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev\
    libopenexr-dev


# Clone, build and install OpenCV
RUN git clone https://github.com/opencv/opencv.git && \
    cd /opencv && mkdir build && cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D WITH_GSTREAMER=ON  -D CMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j"$(nproc)" && \
    make install && \
    rm -rf /opencv

RUN mkdir /home/Object_Time_counter

WORKDIR /home/Object_Time_counter

COPY . /home/Object_Time_counter

RUN g++ -o VehicleTimer VehicleTimer.cpp -I/usr/local/include/opencv4  -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_dnn -lopencv_videoio -lopencv_imgproc

RUN export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

CMD [ "run.sh" ]