#!/bin/bash

echo -n > main.cpp

cat include/Utility.hpp >> main.cpp
cat src/Utility.cpp >> main.cpp

cat include/Client.hpp >> main.cpp
cat src/Client.cpp >> main.cpp

cat include/ServerSocket.hpp >> main.cpp
cat src/ServerSocket.cpp >> main.cpp

cat include/Server.hpp >> main.cpp
cat src/Server.cpp >> main.cpp

cat src/main.cpp >> main.cpp

# Remove lines with 'include "' pattern
sed -i '/include "/d' main.cpp
sed -i '/#if/d' main.cpp
sed -i '/#endif/d' main.cpp
sed -i '/#ifndef/d' main.cpp
