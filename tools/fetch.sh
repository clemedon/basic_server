#!/bin/bash

echo -n > fetch.cpp

cat include/Utility.hpp >> fetch.cpp
cat src/Utility.cpp >> fetch.cpp

cat include/Client.hpp >> fetch.cpp
cat src/Client.cpp >> fetch.cpp

cat include/ServerSocket.hpp >> fetch.cpp
cat src/ServerSocket.cpp >> fetch.cpp

cat include/Server.hpp >> fetch.cpp
cat src/Server.cpp >> fetch.cpp

cat src/main.cpp >> fetch.cpp

# Remove lines with 'include "' pattern
sed -i '/include "/d' fetch.cpp
sed -i '/#if/d' fetch.cpp
sed -i '/#endif/d' fetch.cpp
sed -i '/#ifndef/d' fetch.cpp
