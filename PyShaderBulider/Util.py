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
    tex_type_name = ["Tex", "TexCube"]

    param_lst = params.split('\n')
    tex_idx = 0
    for idx, param in enumerate(param_lst):

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

        if p_type in tex_type_name:
            param_tab[p_type].append((p_name, tex_idx))
            tex_idx += 1
        else:
            param_tab[p_type].append(p_name)
    
    return (param_tab, warning_info)


def process_line(line : str):

    useless = [' ', '\t']
    begin = 0
    end = len(line)-1
    for i in range(len(line)):
        if not (line[i] in useless):
            begin = i
            break
    for i in range(len(line)-1, begin-1, -1):
        if not (line[i] in useless):
            end = i
            break
    
    return line[begin : end+1]

def process_purly_str(content : str):
    return content.replace(' ', '').replace('\t', '').replace('\n', '')

def copy_from_dev_2_bin_dir(name : str):
    from Config import DEV_DIR, BINARY_DIR
    src_dir = os.path.join(DEV_DIR, name)
    dst_dir = os.path.join(BINARY_DIR, name)
    
    if(os.path.exists(dst_dir)):
        shutil.rmtree(dst_dir)

    shutil.copytree(src_dir, dst_dir)


def load_file_2_str(full_path)->str:
    content = ""
    with open(full_path,"rt", encoding='utf-8') as f:
        content = f.read()
    return content

def load_file_2_str_process_line(full_path):
    res = ""
    with open(full_path, "rt", encoding = 'utf-8') as f:
        while True:
            l = process_line(f.readline())
            if(l == '\n'):
                continue
            if(len(l) == 0):
                break
            res += l
    return res

def save_str_2_file(full_path, content) :

    dir_name = os.path.dirname(full_path)
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)

    with open(full_path, "wt", encoding = 'utf-8') as f:
        f.write(content)
        

def get_data_between(content, mark):
    start = content.find(mark)    
    if(start != -1):
        end = content.find(mark, start + len(mark))

        if(end != -1):
           return content[start + len(mark) : end]
        else:
            return ""
            
    return ""

def get_data_between_pos(content, mark, start_pos):
    start = content.find(mark, start_pos)    
    if(start != -1):
        end = content.find(mark, start + len(mark))

        if(end != -1):
           return (content[start + len(mark) : end], end)
        else:
            return ("", -1)
    
    return ("", -1)

def split_mutli(content, mark : list, remain_empty = True):
    res = []
    l = len(content)
    begin, end = 0,0
    while end != l:
        if content[end] in mark:
            if begin == end and remain_empty:
                res.append('')
            if begin != end:
                res.append(content[begin : end])
            end += 1
            begin = end
        else:
            end += 1
    if begin < l:
        res.append(content[begin : l])
    return res

def get_path_no_ext_name(ori_path):
    last_point = ori_path.rfind('.')
    if last_point == -1:
        return ori_path

    return ori_path[:last_point]

