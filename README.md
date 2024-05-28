# rs5x4 host rewrite in C++!

python host kept dying and freezing all devices

# Usage
download the release zip and extract it to a folder. Edit the spotifykeys.txt file with your own values from spotify's dashboard. then just run the exe

# Current Issues / TO-DO
- [x] for cycling music note: skip by 3 and include by 3 bytes, (0xE2, 0x99, 0xAB only, no individual bytes)
- [x] No text scrolling
- [x] Music symbol not showing properly (♫ → ΓÖ½, some unicode UTF-8 issue) (FOUND FIX, CONVERT SYMBOL TO {0xE2, 0x99, 0xAB} AND SEND)
- [ ] Unsure if spotify token refresh is working properly
- [ ] platform compatibility? figure out how to distribute

# latest changes
- packaged working on desktop and laptop
- solved music symbol scrolling, perfect now 🤓
- solved long string stack corruption (by changing to vector lol)
