
void transform_aabb(in vec3 center, in vec3 extend, in mat4 transform_mat, out vec3 o_center, out vec3 o_extend) 
{
    vec3 min_corner = center - extend;
    vec3 max_corner = center + extend;

    vec4 cur_min = transform_mat * vec4(min_corner, 1.0);
    vec4 cur_max = cur_min;

    for(uint i=1; i<8; ++i) 
    {
        bvec3 dicision = bvec3((i&1) != 0, (i&2) != 0, (i&4) != 0);
        vec4 corner = vec4(mix(min_corner, max_corner, dicision), 1.0);
        corner = transform_mat * corner;

        cur_min.x = min(corner.x, cur_min.x);
        cur_min.y = min(corner.y, cur_min.y);
        cur_min.z = min(corner.z, cur_min.z);

        cur_max.x = max(corner.x, cur_max.x);
        cur_max.y = max(corner.y, cur_max.y);
        cur_max.z = max(corner.z, cur_max.z);
    }

    o_center = (cur_max + cur_min).xyz / 2.0;
    o_extend = (cur_max - cur_min).xyz / 2.0;
}

/*
 * transform aabb with model matrix, faster than any transform
 */
void transform_aabb_model(in vec3 center, in vec3 extend, in mat4 model_mat, out vec3 o_center, out vec3 o_extend) 
{
    o_center = (model_mat * vec4(center, 1.0)).xyz;

    vec3 right = model_mat[0].xyz * extend.x;
    vec3 up = model_mat[1].xyz * extend.y;
    vec3 forward = model_mat[2].xyz * extend.z;

    float world_i = abs(dot(vec3(1.0, 0.0, 0.0), right)) + 
                    abs(dot(vec3(1.0, 0.0, 0.0), up)) +
                    abs(dot(vec3(1.0, 0.0, 0.0), forward));

    float world_j = abs(dot(vec3(0.0, 1.0, 0.0), right)) + 
                    abs(dot(vec3(0.0, 1.0, 0.0), up)) +
                    abs(dot(vec3(0.0, 1.0, 0.0), forward));

    float world_k = abs(dot(vec3(0.0, 0.0, 1.0), right)) + 
                    abs(dot(vec3(0.0, 0.0, 1.0), up)) +
                    abs(dot(vec3(0.0, 0.0, 1.0), forward));

    o_extend = vec3(world_i, world_j, world_k);
    
}

float point_distance_2_plane(vec4 plane, vec3 point) 
{
    return dot(plane.xyz, point) - plane.w;
}


bool intersect(in vec3 center, in vec3 extend, in vec4 plane) 
{

    float r = dot(extend , abs(center));
    float temp = point_distance_2_plane(plane, center);
    return -r <= temp;
}

bool early_culling(in vec3 center, in vec3 extend, in mat4 model_mat, in Frustum frustum) 
{
    vec3 center_world;
    vec3 extend_world;
    transform_aabb_model(center, extend, model_mat, center_world, extend_world);

    bool ans = true;
    for(int i=0; i<6; ++i) 
    {
        ans = ans && intersect(center_world, extend_world, frustum.planes[i]);
    }

    return ans;

}