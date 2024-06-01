# rs5x4 host rewritten in C++

python host kept dying and freezing all devices

python version still in python branch

# *IMPORTANT*

currently only tested for Windows (W11). Linux behavior is still very buggy with many segfaults

# Usage
download the release zip and extract it to a folder. 

Edit the spotifykeys.txt file with your own values from spotify's dashboard. ***REDIRECT URI MUST BE [http://localhost/8888](http://localhost:8888/callback)***, if you want to use a different one spotifytoken.h will need to be changed accordingly and recompiled

then just double click start.vbs which will start the host in the background (its called test.exe)

# Current Issues / TO-DO
- [ ] experiment with spotify api calls per second upper limit (currently 3/s)
- [ ] occasional crashing, figure it out
- [x] implement threads for sending image with same logic as python (song change = kill image thread and start new one)
- [x] if image write fails wait for new device instead of continuing
- [ ] *this is probably firmware side but occasionally parts of image are horizontally shifted (has to do with image_counter not being incremented by 30 properly between messages)
- [x] for cycling music note: skip by 3 and include by 3 bytes, (0xE2, 0x99, 0xAB only, no individual bytes)
- [x] No text scrolling
- [x] Music symbol not showing properly (♫ → ΓÖ½, some unicode UTF-8 issue) (FOUND FIX, CONVERT SYMBOL TO {0xE2, 0x99, 0xAB} AND SEND)
- [ ] Unsure if spotify token refresh is working properly
- [ ] platform compatibility? figure out how to distribute
- [ ] convert to a working windows service

# latest changes
- packaged working on desktop and laptop
- solved music symbol scrolling, perfect now 🤓
- solved long string stack corruption (by changing to vector lol)


# Steps for building from source

This is mainly so I don't forget. 

Install CMake, ninja, mingw. Clone repo and install requirements in vcpkg_rf.txt. Run commands in commands.txt
