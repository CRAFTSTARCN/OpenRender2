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

def copy_file_from_dev_2_bin(name : str):
    src_path = os.path.join(DEV_DIR, name)
    dst_path = os.path.join(BINARY_DIR, name)

    if(os.path.exists(dst_path)):
        os.remove(dst_path)
    shutil.copyfile(src_path, dst_path)

def move_all():
    copy_from_dev_2_bin("Content")
    copy_from_dev_2_bin("PassShader")
    copy_file_from_dev_2_bin("Project.json")

if __name__ == "__main__":
    move_all()
