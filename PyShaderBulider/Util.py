PARAM_TYPE_VEC4 = 0
PARAM_TYPE_SCALAR = 1
import os
import shutil

def resolve_param_one_line(param : str):
    warning_info = ""
    type_name = param.split(' ')
        
    p_type = type_name[0]
    p_name = ""
    found = False
    for i in range(1, len(type_name)):
        if len(type_name[i]) == 0:
            continue
        if found:
            warning_info += "Warnning: ignore identifier {}, cause a name has provided before\n".format(type_name[i])
        else:
            found = True
            p_name = type_name[i]

        pass
    if not found:
        raise Exception("Missing param name for param type: {}}".format(p_type))
    
    return (p_type, p_name, warning_info)

def check_param_name(name : str):
    return name[0].isalpha() or name[0] == '_'

def param_resolver(params : str):

    warning_info = ""
    used_name = set()

    param_tab = {"Vec" : [], "Scalar" : [], "Tex" : [], "TexCube" : []}

    param_lst = params.split('\n')
    for param in param_lst:

        param : str
        if len(param) == 0:
            continue
        (p_type, p_name, war) = resolve_param_one_line(param)
        warning_info += war

        if param_tab.get(p_type, None) == None:
            raise Exception("Invalid param type: {}".format(p_type))

        if not check_param_name(p_name):
            raise Exception("Invalid param name: {}".format(p_name))

        if p_name in used_name :
            raise Exception("Duplicate name {}".format(p_name))
        used_name.add(p_name)

        param_tab[p_type].append(p_name)
    
    return (param_tab, warning_info)


def process_line(line : str):

    useless = [' ', '\t']

    for i in range(len(line)):
        if not (line[i] in useless):
            return line[i : ]
    
    return ""

def copy_from_dev_2_bin(name : str):
    from Config import DEV_DIR, BINARY_DIR
    src_dir = os.path.join(DEV_DIR, name)
    dst_dir = os.path.join(BINARY_DIR, name)
    
    if(os.path.exists(dst_dir)):
        shutil.rmtree(dst_dir)

    shutil.copytree(src_dir, dst_dir)