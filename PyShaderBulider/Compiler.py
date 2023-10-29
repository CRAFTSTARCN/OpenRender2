# main comple tool
# compile everything 

import sys

sys.path.append("./")

import SPVCompiler 
from Theading import add_work, all_shutdown


def test():

    add_work(SPVCompiler.compile_shader_2_spv_glsl, "../PassShader/src/MeshDrawTask.Task.glsl", SPVCompiler.STAGE_TASK, "../PassShader/spv/MeshDrawTask.Task.spv")
    add_work(SPVCompiler.compile_shader_2_spv_glsl, "../PassShader/src/DeferredShadingMesh.Mesh.glsl", SPVCompiler.STAGE_MESH, "../PassShader/spv/DeferredShadingMesh.Mesh.spv")
    add_work(SPVCompiler.compile_shader_2_spv_glsl, "../PassShader/src/DeferredShadingFrag.Frag.glsl", SPVCompiler.STAGE_FRAG, "../PassShader/spv/DeferredShadingFrag.Frag.spv")

    all_shutdown()

if __name__ == "__main__":
    from Util import copy_from_dev_2_bin_dir
    test()
    copy_from_dev_2_bin_dir("PassShader")
    copy_from_dev_2_bin_dir("Content/Materials/Shaders")