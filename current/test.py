from ctypes import cast, POINTER
from math import floor
from comtypes import CLSCTX_ALL
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
devices = AudioUtilities.GetSpeakers()

interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
volume = cast(interface, POINTER(IAudioEndpointVolume))

last_volume = floor(volume.GetMasterVolumeLevelScalar()*100)
while True:
    curr_volume = floor(volume.GetMasterVolumeLevelScalar()*100)
    if not last_volume == curr_volume:
        print(curr_volume)
    last_volume = curr_volume
