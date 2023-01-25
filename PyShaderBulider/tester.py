from Util import param_resolver
try:
    params, war = param_resolver('''Scalar a b c
Scalar B
Scalar c
Vec d
Tex e''')

    print(params)
    print("[Warning]", war)
    
except Exception as e:
    print("[Compile error]", e)