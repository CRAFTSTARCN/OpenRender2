{
    vec3 camera_position;
    float camera_position_padding;

    vec3 directional_light_direction;
    float directional_light_direction_padding;

    vec3 directional_light_color;
    float directional_light_color_padding;

    vec3 ambient_light_color;
    float ambient_light_color_padding;

    Frustum camera_frustum;
    
    mat4 mat_view_proj;
    mat4 mat_view_proj_inv;
    mat4 directional_light_view_proj;
};
