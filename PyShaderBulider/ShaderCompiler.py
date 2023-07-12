#Compile .shader file

import os
import Config
import Util
import traceback
import json
import sys

PARAM_MARK = "[[PARAM]]"
VERT_USAGE_MARK = "[[VertexUseParam]]"
FRAG_USAGE_MARK = "[[FragmentUseParam]]"
PASS_MARK = "[[Pass]]"
PASS_NAME_MARK = "[[PassName]]"
BLEND_MODE_MARK = "[[BlendMode]]"
DOUBLE_SIDE_MARK = "[[DoubleSided]]"

def get_use_stage(name, vert_usage, frag_usage):
    usage_state = []
    if name in vert_usage:
        usage_state.append('Geo')
    if name in frag_usage:
        usage_state.append('Frag')
    return usage_state

def gen_material_json(shader, shader_rel_path, param_table, vert_usage, frag_usage, pass_shader_dict):
    blend_mode = Util.process_purly_str(Util.get_data_between(shader, BLEND_MODE_MARK))
    double_side = Util.process_purly_str(Util.get_data_between(shader, DOUBLE_SIDE_MARK))
    double_side = double_side.lower() == "true" 
    material_json = {
        "AssetType" : "MaterialBase", 
        "BaseShader" : shader_rel_path, 
        "PassShaders" : pass_shader_dict,
        "BlendMode" : blend_mode,
        "DoubleSided" : double_side}

    
    material_json.update({"BufferParameters" : {"Vec" : param_table['Vec'], "Scalar" : param_table['Scalar'], "BufferStage" : get_use_stage('[[Buffer]]', vert_usage, frag_usage)}})
    tex = [None for i in range(0, len(param_table['Tex']) + len(param_table["TexCube"]))]

    for t in param_table['Tex']:
        idx = t[1]
        tex[idx] = {
            "Name" : t[0],
            "Stage" : get_use_stage(t[0], vert_usage, frag_usage),
            "Type" : "Tex2D"
        }
    
    for t in param_table['TexCube']:
        idx = t[1]
        tex[idx] = {
            "Name" : t[0],
            "Stage" : get_use_stage(t[0], vert_usage, frag_usage),
            "Type" : "TexCube"
        }
    
    material_json.update({"TexParam" : tex})
    return material_json
    

#if material pass is not None, generate material
def compile_shader(shader_path_rel, material_path_rel):
    full_shader_path = os.path.join(Config.CONTENT_DIR, shader_path_rel)

    pass_shader_dict = {}

    try:

        shader = Util.load_file_2_str_process_line(full_shader_path)

        param_table, warning = Util.param_resolver(Util.get_data_between(shader, PARAM_MARK))
        if len(warning) != 0:
            print("[{}]: Warning: {}".format(shader_path_rel, warning))
        
        vert_usage = Util.get_data_between(shader, VERT_USAGE_MARK)
        vert_usage = Util.split_mutli(vert_usage, ['\n', '\t', ' '], False)

        frag_usage = Util.get_data_between(shader, FRAG_USAGE_MARK)
        frag_usage = Util.split_mutli(frag_usage, ['\n', '\t', ' '], False)
        cur_start = 0
        while True:
            pass_content, end_pos = Util.get_data_between_pos(shader, PASS_MARK, cur_start)
            if end_pos == -1:
                break
            
            cur_start = end_pos + len(PASS_MARK)
            pass_name = Util.get_data_between(pass_content, PASS_NAME_MARK)
            pass_name = Util.process_purly_str(pass_name)
            if len(pass_name) == 0:
                print("[{}]: Error, no pass name, the pass will skiped".format(shader_path_rel))
            
            if pass_shader_dict.get(pass_name) != None:
                print("[{}]: Error, duplicate pass name {}".format(shader_path_rel, pass_name))
            
            if pass_name == "GBuffer":
                import PassMaterialShaderCompiler.GBufferPass as GBufferPassCompliler
                mesh, frag = GBufferPassCompliler.compile_pass_spv_shader(pass_content, param_table, vert_usage, frag_usage, shader_path_rel)

                print(mesh, frag)
                pass_shader_dict.update({pass_name : {"MeshShader" : Util.get_path_no_ext_name(mesh), "FragmentShader" : Util.get_path_no_ext_name(frag)}})
            
            else:

                print("[{}]: no compiler for pass {}".format(pass_name))
        
        if material_path_rel != None:
            material_json = gen_material_json(shader, shader_path_rel, param_table, vert_usage, frag_usage, pass_shader_dict)
            with open(material_path_rel, "wt+", encoding='utf-8') as f:
                json.dump(material_json, f)
    
    except Exception as e:
        
        print("[{}] Compile error:".format(shader_path_rel), e)
        traceback.print_stack()
        #raise(e)

if __name__ == "__main__":
    argc = len(sys.argv)
    if argc < 2:
        print("No input file")
        quit(1)
    
    shader_path = sys.argv[1]
    mat_path = None

    Index = 2
    while Index < argc:
        if sys.argv[Index] == "-M" and Index < argc - 1:
            mat_path = sys.argv[Index + 1]
            Index += 1
            
            pass
        Index += 1
        pass

    compile_shader(shader_path, mat_path)
    
#'./Materials/Shaders/BasicDisneyPBR.shader', "./.cache/test_mi.json"