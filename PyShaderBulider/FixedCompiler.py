#fixed pass shader tool
#only compile passes' fixed shader

import Config
import os

files = os.walk(Config.CACHE_DIR)

for (dir_path, dir_name, file_name) in files:
    print(dir_path, dir_name, file_name)