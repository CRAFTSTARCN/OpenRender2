import subprocess
from Config import *
import traceback

STAGE_TASK = "task"
STAGE_MESH = "mesh"
STAGE_FRAG = "frag"
STAGE_VERT = "vert"

def log_compile_info(cat : str, content : str):
    print("[{}]: {}".format(cat, content))

def log_compile_error(cat : str, content : str):
    print("[ERROR][{}]: {}".format(cat, content))


def compile_shader_2_spv(shader_path : str, shader_stag : str, output_file : str) :
    try:
        log_compile_info(shader_path, "Start compiling to spv...")

        proc = subprocess.Popen([
            GLSLC, 
            "-I", 
            INCLUDE_PATH, 
            "-fshader-stage={}".format(shader_stag), 
            "--target-spv={}".format(SPRIV_VERSION),
            shader_path,
            "-o",
            output_file], stderr=subprocess.STDOUT, stdout=subprocess.PIPE, encoding='UTF-8')

        compile_info = proc.communicate()[0]
        if not len(compile_info) == 0:
            log_compile_info(shader_path, compile_info)

        log_compile_info(shader_path, "Finish compiling to spv...")

    except Exception as e:
        print(e)
        traceback.print_exc()

