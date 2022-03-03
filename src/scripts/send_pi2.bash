tar -cvf ./build/burtos.tar.gz cfg src CMakeLists.txt
scp build/burtos.tar.gz pi@192.168.1.21:/home/pi/burtos-scp/burtos.tar.gz
