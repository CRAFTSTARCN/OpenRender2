vec3 DecodeNormal(vec3 raw)
{
    return raw * 2.0 - 1.0;
}

vec3 ApplyGLNormal(vec3 raw_normal_map, vec3 normal, vec3 tangent)
{
    vec3 bi = cross(normal, tangent);
    vec3 decode_normal = normalize(DecodeNormal(raw_normal_map));
    vec3 adjusted = (tangent * decode_normal.x) + (bi * decode_normal.y) + (normal * decode_normal.z);
    return normalize(adjusted);
}

vec3 ApplyDXNormal(vec3 raw_normal_map, vec3 normal, vec3 tangent)
{
    vec3 bi = cross(normal, tangent);
    vec3 decode_normal = normalize(DecodeNormal(raw_normal_map));
    vec3 adjusted = tangent * decode_normal.x - bi * decode_normal.y + normal * decode_normal.z;
    return normalize(adjusted);
}
