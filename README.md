# rs5x4_host
host program for rs5x4 to display spotify information

# To-Do
- [ ] try to fully migrate to c++

# SETUP

1. Install python and virtualenv (pip install virtualenv)
2. cd current
3. Create virtualenv inside current (virtualenv virtualenvName)
4. .\virtualenvName\Scripts\activate (on windows, linux uses 'source .\virtualenvName\bin\activate')
5. pip install spotipy requests hid Pillow
6. create spotifykeys.py file, create 3 variables client_id, client_secret, and redirect_uri. They are all strings, first 2 are copied directly from spotify dev dashboard from your app page. last must be 'https://localhost:8888/callback'
7. python main.py
8. You will be prompted to log into spotify, log in and copy the url you were redirected to into the console, this will store temporary tokens in a 'tokens.txt' file. If this happens, everything should be working properly by now
