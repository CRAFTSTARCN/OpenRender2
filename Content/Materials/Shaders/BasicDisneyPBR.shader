[[PARAM]]
    Vec Emissive
    Scalar Metallic
    Scalar Specular
    Scalar Roughness
    Scalar Subsurface
    Scalar Anisotropy
    Tex BaseColor
[[PARAM]]

[[VertexUseParam]]
[[VertexUseParam]]

[[FragmentUseParam]]
    [[Buffer]]
    BaseColor
[[FragmentUseParam]]

[[BlendMode]]
Opaque
[[BlendMode]]

[[DoubleSided]]
false
[[DoubleSided]]

[[Pass]] 
    [[PassName]]
    GBuffer
    [[PassName]]
    
    [[VertexProcessInclude]]
    [[VertexProcessInclude]]

    //void process_vertex(in VertexStructure vertex, in InstanceData instance, uint vertex_id, uint meshlet_id);
    //Outs:
    //vec3 world_position;
    //vec3 world_normal;
    //vec3 world_tangent;
    //vec2 uv;

    //NDC Position
    //SystemTargetPosition

    [[VertexProcess]]
        vec3 wp = (instance.mat_model * vec4(vertex.x, vertex.y, vertex.z, 1.0)).xyz;
        OUT[vertex_id].world_position = wp;
        OUT[vertex_id].world_normal = normalize((instance.mat_model_normal * vec4(vertex.Nx, vertex.Ny, vertex.Nz, 0.0)).xyz);
        OUT[vertex_id].world_tangent = normalize((instance.mat_model * vec4(vertex.Tx, vertex.Ty, vertex.Tz, 0.0)).xyz);
        OUT[vertex_id].uv = vec2(vertex.u, vertex.v);
        [[SystemTargetPosition]] = mat_view_proj * vec4(wp, 1.0);
    [[VertexProcess]]

    [[FragmentProcessInclude]]
        #include "BuiltinModel/DefaultLitUtil.glsl"
    [[FragmentProcessInclude]]

    // void fragment_function(
    //     out vec4 gbuffer_a,
    //     out vec4 gbuffer_b,
    //     out vec4 gbuffer_c,
    //     out vec4 gbuffer_d,
    //     out vec4 gbuffer_e)

    [[GBufferFragment]]
    
        DefaultLitGBufferData data;
        data.base_color = texture(BaseColor, IN.uv).rgb;
        data.ambient_occlusion = 1.0;
        data.normal = normalize(IN.world_normal);
        data.subsurface = Subsurface;
        data.emissive = Emissive.rgb;
        data.tangent = normalize(IN.world_tangent);
        data.anisotropy = Anisotropy;
        data.metallic = Metallic;
        data.specular = Specular;
        data.roughness = Roughness;
        EncodeDefaultLitGBuffer(
            data, 
            gbuffer_a,
            gbuffer_b,
            gbuffer_c,
            gbuffer_d,
            gbuffer_e);

    [[GBufferFragment]]


[[Pass]]