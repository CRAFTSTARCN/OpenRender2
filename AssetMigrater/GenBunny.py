import random

scene = '''
{{
    "AssetType" : "Level",

    "Objects" : [{}],

    "Skybox" :  
    {{
        "Skybox" : "./Texture/AnimSky/T_Anim_Skybox.json",
        "Radiance" : "./Texture/AnimSky/T_Anim_Radiance.json",
        "Irradiance" : "./Texture/AnimSky/T_Anim_Irradiance.json"
    }},

    "Camera" : 
    {{
        "Position" : 
        {{
            "X" : 0.0,
            "Y" : 0.0,
            "Z" : 0.0
        }},
        "Rotation" : 
        {{
            "X" : 0.0,
            "Y" : -90.0,
            "Z" : 0.0
        }},

        "Near" : 0.1,
        "Far" : 100000,
        "FOVV" : 75.0,

        "CameraMoveSpeed" : 70.0,
        "CameraLookSpeed" : 0.1
    }},
    "DirectionalLight":
    {{
        "Color" : 
        {{
            "R" : 0.7,
            "G" : 0.8,
            "B" : 1.0,
            "A" : 1.0
        }},
        "Direction" : 
        {{
            "X" : 0.0,
            "Y" : 0.72,
            "Z" : 0.72
        }}
    }}
}}
'''

temp = '''{{
            "Name" : "Bunny_{}",
            "Scripts" : 
            [
                {{
                    "Type" : "StaticMeshComponent",
                    "Name" : "Mesh",
                    "Parent" : null,
                    "MaterialOverride": "./Materials/{}",
                    "StaticMesh" : "./Mesh/S_Bunny.json",
                    "PackIndex" : 0,
                    "Blocker" : true,
                    "Transform" : 
                    {{
                        "Translate" : 
                        {{
                            "X" : {},
                            "Y" : {},
                            "Z" : {}
                        }},
                        "Rotation" : 
                        {{
                            "X" : {},
                            "Y" : {},
                            "Z" : {}
                        }},
                        "Scale" : 
                        {{
                            "X" : {},
                            "Y" : {},
                            "Z" : {}
                        }}
                    }}
                }},
                {{
                    "Type" : "StaticMeshComponent",
                    "Name" : "MeshEye",
                    "Parent" : 0,
                    "StaticMesh" : "./Mesh/S_Bunny.json",
                    "PackIndex" : 1,
                    "Blocker" : true,
                    "MaterialOverride" : "./Materials/{}",
                    "Transform" : null
                }}
            ]
        }},'''

M_LIST = ['MI_Bunny.json', 'MI_Bunny_1.json', 'MI_Bunny_2.json', 'MI_Bunny_3.json']

GEN_CNT = 1000

TRANSLATE_RANGE = ((-1000, 2000), (-1000, 2000), (-1000, 2000))
ROTATION_RANGE = ((-90, 90), (-90, 90), (-90, 90))
S_RANGE = (20, 50)

out = ""

for i in range(GEN_CNT):
    mat_idx = random.randrange(0, len(M_LIST))
    mat = M_LIST[mat_idx]
    tx = random.randrange(TRANSLATE_RANGE[0][0], TRANSLATE_RANGE[0][1])
    ty = random.randrange(TRANSLATE_RANGE[1][0], TRANSLATE_RANGE[1][1])
    tz = random.randrange(TRANSLATE_RANGE[2][0], TRANSLATE_RANGE[2][1])

    rx = random.randrange(ROTATION_RANGE[0][0], ROTATION_RANGE[0][1])
    ry = random.randrange(ROTATION_RANGE[1][0], ROTATION_RANGE[1][1])
    rz = random.randrange(ROTATION_RANGE[2][0], ROTATION_RANGE[2][1])
    s = random.randrange(S_RANGE[0], S_RANGE[1])

    out += temp.format(i, mat, tx, ty, tz, rx, ry, rz, s, s, s, mat)

out_file = scene.format(out[:-1])

with open("GenBunny.txt", "wt+", encoding='utf-8') as f:
    f.write(out_file)