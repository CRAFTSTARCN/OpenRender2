import sys

sys.path.append("./")

import SPVCompiler 
from Theading import add_work, all_shutdown


def test():
    add_work(SPVCompiler.compile_shader_2_spv, "../Content/Materials/Shaders/BasicDisneyPBR.GBuffer.Mesh.glsl", SPVCompiler.STAGE_MESH, "../Content/Materials/Shaders/BasicDisneyPBR.GBuffer.Mesh.spv")
    add_work(SPVCompiler.compile_shader_2_spv, "../Content/Materials/Shaders/BasicDisneyPBR.GBuffer.Frag.glsl", SPVCompiler.STAGE_FRAG, "../Content/Materials/Shaders/BasicDisneyPBR.GBuffer.Frag.spv")

    add_work(SPVCompiler.compile_shader_2_spv, "../PassShader/src/PreGBufferTask.Task.glsl", SPVCompiler.STAGE_TASK, "../PassShader/spv/PreGBufferTask.Task.spv")
    add_work(SPVCompiler.compile_shader_2_spv, "../PassShader/src/DeferredShadingMesh.Mesh.glsl", SPVCompiler.STAGE_MESH, "../PassShader/spv/DeferredShadingMesh.Mesh.spv")
    add_work(SPVCompiler.compile_shader_2_spv, "../PassShader/src/DeferredShadingFrag.Frag.glsl", SPVCompiler.STAGE_FRAG, "../PassShader/spv/DeferredShadingFrag.Frag.spv")

    all_shutdown()

def compile_all(spawn_material : bool, material_rel_dir : str):
    pass

def compile_all_fixed():
    pass

def compile_shader(shader_path : str, spawn_material : bool, material_rel_dir : str):
    pass

def compile_fixed(shader_name : str):

    pass

if __name__ == "__main__":
    from Util import copy_from_dev_2_bin
    test()
    copy_from_dev_2_bin("PassShader")
    copy_from_dev_2_bin("Content/Materials/Shaders")