from PassMaterialShaderCompiler import GLSLTool 
import os
import Config
import Util
import SPVCompiler

PASS_NAME = "GBuffer"

MESH_TEMPLATE_NAME = "GBufferTemplateMesh.Mesh.glsl"
FRAG_TEMPLATE_NAME = "GBufferTemplateFrag.Frag.glsl"

MATERIAL_SET = 2

#Template file tag
MAT_DATA_TAG = "[[MATERIAL_DATA]]"
INCLUDE_FILE_TAG = "[[INCLUDE_FILES]]"
MESH_VERT_PROCESS_TAG = "[[MESH_VERTEX_PROCESSOR]]"
FRAG_PROCESS_TAG = "[[FRAGMENT_GBUFFER_FUNCTION]]"


#Process file mark
VERT_INCLUDE_FILE_MARK = "[[VertexProcessInclude]]"
MESH_VERT_PROCESS_MARK = "[[VertexProcess]]"
TARGET_POSITION_MARK = "[[SystemTargetPosition]]"

FRAG_INCLUDE_FILE_MARK = "[[FragmentProcessInclude]]"
FRAG_PROCESS_MARK = "[[GBufferFragment]]"


#functioin template
FUNC_TEMP_VERT ='''
void process_vertex(in VertexStructure vertex, in InstanceData instance, uint vertex_id, uint meshlet_id) 
{
    if(MESHLET_VERTEX_ITERATIONS * WORKGROUP_SIZE > VERTEX_OUTPUT_SIZE && vertex_id >= VERTEX_OUTPUT_SIZE) 
    {
        return;
    }
    [[BODY]]

#ifdef _DEBUG_MESHLET
    OUT[vertex_id].debug_color = debug_colors[meshlet_id % DEBUG_COLOR_COUNT];
#endif

}
'''



FUNC_TEMP_FRAG = '''
void fragment_function(
    out vec4 gbuffer_a,
    out vec4 gbuffer_b,
    out vec4 gbuffer_c,
    out vec4 gbuffer_d,
    out vec4 gbuffer_e)
{
#ifdef _DEBUG_MESHLET
    EncodeUnlitGBuffer(IN.debug_color, gbuffer_c, gbuffer_d); //texture(BaseColor, IN.uv).rgb;
#else
    [[BODY]]
#endif
}
'''

def compile_pass_spv_shader(passbody, param_table, vert_usage, frag_usage, shader_file_path_rel) :
    mseh_shader_path, frag_shader_path = generate_shader_file_glsl(passbody, param_table, vert_usage, frag_usage, shader_file_path_rel)

    mesh_spv_name = os.path.basename(Util.get_path_no_ext_name(mseh_shader_path) + ".spv")
    frag_spv_name = os.path.basename(Util.get_path_no_ext_name(frag_shader_path) + ".spv")

    shader_dir = os.path.dirname(shader_file_path_rel)
    mesh_spv_path = os.path.join(Config.CONTENT_DIR,shader_dir, mesh_spv_name)
    frag_spv_path = os.path.join(Config.CONTENT_DIR,shader_dir, frag_spv_name)

    SPVCompiler.compile_shader_2_spv_glsl(mseh_shader_path, SPVCompiler.STAGE_MESH, mesh_spv_path)
    SPVCompiler.compile_shader_2_spv_glsl(frag_shader_path, SPVCompiler.STAGE_FRAG, frag_spv_path)

    return (os.path.join(shader_dir, mesh_spv_name).replace('\\', '/'), os.path.join(shader_dir, frag_spv_name).replace('\\', '/'))

def generate_shader_file_glsl(passbody, param_table, vert_usage, frag_usage, shader_file_path_rel : str):
    material_buffer_str = GLSLTool.MATERIAL_UNIFORM_FMT.format(MATERIAL_SET) + "{\n" + \
        GLSLTool.gen_buffer_content(param_table["Vec"], param_table["Scalar"]) + "};\n"
    
    tex_str = {}

    for tex in param_table["Tex"]:
        dec_str = GLSLTool.gen_texture_declare(MATERIAL_SET, tex[1] + 1, GLSLTool.TEXTURE2D_STR, tex[0])
        tex_str.update({tex[0] : dec_str}) 


    for tex in param_table["TexCube"]:
        dec_str = GLSLTool.gen_texture_declare(MATERIAL_SET, tex[1] + 1, GLSLTool.TEXTURECUBE_STR, tex[0])
        tex_str.update({tex[0] : dec_str}) 


    shader_file_rel_no_ext = Util.get_path_no_ext_name(shader_file_path_rel)
    mesh_shader_rel_path = shader_file_rel_no_ext + ".GBuffer.Mesh.glsl"
    frag_shader_rel_path = shader_file_rel_no_ext + ".GBuffer.Frag.glsl"
    mesh_shader_file_path = os.path.join(Config.CACHE_DIR, mesh_shader_rel_path)
    frag_shader_file_path = os.path.join(Config.CACHE_DIR, frag_shader_rel_path)

    generate_mesh_shader_file_glsl(passbody, material_buffer_str, tex_str, vert_usage, mesh_shader_file_path)
    generate_frag_shader_file_glsl(passbody, material_buffer_str, tex_str, frag_usage, frag_shader_file_path)

    return (mesh_shader_file_path, frag_shader_file_path)

def generate_mesh_shader_file_glsl(passbody : str, material_buffer_str, tex_str, vert_usage, save_path):
    mesh_full_path = os.path.join(Config.TEMPLATE_FILE_PATH, MESH_TEMPLATE_NAME)
    template_str = Util.load_file_2_str(mesh_full_path)
    
    material_str = GLSLTool.build_usage_str(material_buffer_str, tex_str, vert_usage)

    template_str = template_str.replace(MAT_DATA_TAG, material_str)

    template_str = template_str.replace(INCLUDE_FILE_TAG, Util.get_data_between(passbody, VERT_INCLUDE_FILE_MARK))
    
    process_content = Util.get_data_between(passbody, MESH_VERT_PROCESS_MARK)
    process_content = process_content.replace(TARGET_POSITION_MARK, "gl_MeshVerticesEXT[vertex_id].gl_Position")

    template_str = template_str.replace(MESH_VERT_PROCESS_TAG, FUNC_TEMP_VERT.replace("[[BODY]]", process_content))

    Util.save_str_2_file(save_path, template_str)

def generate_frag_shader_file_glsl(passbody : str, material_buffer_str, tex_str, frag_usage, save_path):
    frag_full_path = os.path.join(Config.TEMPLATE_FILE_PATH, FRAG_TEMPLATE_NAME)
    template_str = Util.load_file_2_str(frag_full_path)
    
    material_str = GLSLTool.build_usage_str(material_buffer_str, tex_str, frag_usage)

    template_str = template_str.replace(MAT_DATA_TAG, material_str)

    template_str = template_str.replace(INCLUDE_FILE_TAG, '#include "BuiltinModel/UnlitUtil.glsl" \n' + Util.get_data_between(passbody, FRAG_INCLUDE_FILE_MARK))
    
    process_content = Util.get_data_between(passbody, FRAG_PROCESS_MARK)

    template_str = template_str.replace(FRAG_PROCESS_TAG, FUNC_TEMP_FRAG.replace("[[BODY]]", process_content))

    Util.save_str_2_file(save_path, template_str)

