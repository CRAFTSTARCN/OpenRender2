MATERIAL_UNIFORM_FMT = "layout(std140, set = {}, binding = 0) uniform _material_uniform "
TEXTURE_UNIFORM_FMT = "layout(set = {}, binding = {}) uniform {} {}; \n"

TEXTURE2D_STR = "sampler2D"
TEXTURECUBE_STR = "samplerCube"
TEXTUREARRAY_STR = "samplerArray"

def gen_texture_declare(set, binding, type, name):
    return TEXTURE_UNIFORM_FMT.format(set, binding, type,name)

def gen_buffer_content(vec, scalar)->str:
    if len(vec) == 0 and len(scalar) == 0:
        return "vec4 _empty_padding;"
    
    content = ""
    for v in vec:
        content += "vec4 {}; \n".format(v)
    
    for s in scalar:
        content += "float {}; \n".format(s)
    
    remain = 4 - (len(scalar) % 4)

    for pd in range(remain):
        content += "float _end_padding_{}; \n".format(pd)
    return content

def build_usage_str(material_buffer_str, tex_usage : dict, usage):
    res = ""
    for u in usage:
        if u == "[[Buffer]]":
            res += material_buffer_str
        else:
            val = tex_usage.get(u)
            if val != None:
                res += val
    
    return res