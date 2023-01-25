#define PI 3.14159265358979

float pow5(float v)
{
    float vv = v*v;
    return vv*vv*v;
}

vec3 get_F0(vec3 base_color, float specular, float metallic) 
{
    return mix(specular * 0.08 * vec3(1.0,1.0,1.0), base_color, metallic);
}

float F_schlick_factor(float u)
{
    return pow5(1-u);
}

vec3 F_schlick(vec3 F0, float u) 
{
    return F0 + (vec3(1.0,1.0,1.0) - F0) * F_schlick_factor(u);
}

vec3 F_schlick_roughness(vec3 F0, float u, float roughness) 
{
    return F0 + (max(vec3(1.0-roughness,1.0-roughness,1.0-roughness), F0) - F0) * F_schlick_factor(u);
}

float D_GGX(float alpha, float NdH)
{
    float a2 = alpha * alpha;
    float denom = NdH * NdH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float D_GGX_anisotropy(float HdX, float HdY, float NdH, float ax, float ay) 
{
    float denom = (HdX * HdX) / (ax * ax)  + (HdY * HdY) / (ay * ay) + NdH * NdH;
    return 1.0 / (PI * ax * ay * denom * denom);
}

float remapping_roughness(float ori_roughness) 
{
    float sqrt_a = (0.5 + ori_roughness / 2.0);
    return sqrt_a * sqrt_a;
}

//V factor, no 4(n dot v) on numerator
float G_smith_GGX_V(float NdVL, float a_G) 
{
    float a = a_G * a_G;
    float d2 = NdVL * NdVL;
    return 1.0 / (NdVL + sqrt(a + d2 - a * d2));
}

float G_smith_GGX_anisotropy_V(float NdVL, float VLdX, float VLdY, float ax, float ay)
{
    float dxa = VLdX*ax;
    float dya = VLdY*ay;
    return 1.0 / (NdVL + sqrt(dxa*dxa + dya*dya + NdVL * NdVL));
}


//Hight quality surface brdf
vec3 BRDF_HQ(
    vec3 L, 
    vec3 V, 
    vec3 N, 
    vec3 X, 
    vec3 Y, 
    vec3 F0,
    vec3 base_color, 
    float metallic, 
    float roughness, 
    float subsurface,
    float anisotropy) 
{
    vec3 H = normalize(L+V);
    float NdL = dot(N,L);
    float NdV = dot(N,V);
    float NdH = dot(N,H);
    float LdH = dot(L,H);

    if(NdL < 0 || NdV < 0) 
    {
        return vec3(0.0, 0.0, 0.0);
    }
    
    //diffusion
    float FL = F_schlick_factor(NdL);
    float FV = F_schlick_factor(NdV);
    float FD90 = 0.5 + 2* LdH * LdH * roughness;
    float F_diffuse = mix(1.0, FD90, FL) * mix(1.0, FD90, FV);

    //subsurface
    float FSS90 = LdH * LdH * roughness;
    float FSS = mix(1.0, FSS90, FL) * mix(1.0, FSS90, FV);
    float SS = 1.25 * (FSS * (1 / (NdL + NdV) - 0.5) + 0.5);

    //specular

    vec3 F_spec = F_schlick(F0, LdH);

    float aspect = (1 - anisotropy * 0.9);
    float ax = max(0.001, roughness * roughness / aspect);
    float ay = max(0.001, roughness * roughness * aspect);
    float D_spec = D_GGX_anisotropy(dot(H, X), dot(H, Y), NdH, ax, ay);
    float G_spec = G_smith_GGX_anisotropy_V(NdL, dot(L,X), dot(L,Y), ax, ay) * 
                   G_smith_GGX_anisotropy_V(NdV, dot(V,X), dot(V,Y), ax, ay);


    return ((1 / PI) * mix(F_diffuse, SS, subsurface) * base_color) * (1 - metallic) + D_spec * F_spec * G_spec;
}  


//Middle quality brdf, no anisotropy, no subsurface, lambert diffusion
vec3 BRDF_MQ(
    vec3 L, 
    vec3 V, 
    vec3 N, 
    vec3 F0,
    vec3 base_color, 
    float metallic, 
    float roughness)
{
    vec3 H = normalize(L+V);
    float NdL = dot(N,L);
    float NdV = dot(N,V);
    float NdH = dot(N,H);
    float LdH = dot(L,H);

    float a_G = remapping_roughness(roughness);

    float D_spec = D_GGX(max(roughness*roughness, 0.01 * 0.01), NdH);
    vec3 F_spec = F_schlick(F0, LdH);
    float G_spec = G_smith_GGX_V(NdL, a_G) * G_smith_GGX_V(NdV, a_G);

    vec3 KD = (vec3(1.0,1.0,1.0) - F_spec) * (1-metallic);
    return base_color * KD / PI + D_spec * F_spec * G_spec;
}
