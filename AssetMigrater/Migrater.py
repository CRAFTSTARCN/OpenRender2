import os
import shutil


DEV_DIR = "../"
BINARY_DIR = "../Binary/"

def copy_from_dev_2_bin(name : str):
    src_dir = os.path.join(DEV_DIR, name)
    dst_dir = os.path.join(BINARY_DIR, name)
    
    if(os.path.exists(dst_dir)):
        shutil.rmtree(dst_dir)

    shutil.copytree(src_dir, dst_dir)

def move_all():
    copy_from_dev_2_bin("Content")

if __name__ == "__main__":
    move_all()
