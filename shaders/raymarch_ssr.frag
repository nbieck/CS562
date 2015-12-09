#version 440

layout(binding = 4) uniform sampler2D HiZBuffer;
layout(binding = 2) uniform sampler2D Position;
layout(binding = 3) uniform sampler2D Normal;
layout(binding = 1) uniform sampler2D SceneColor;

uniform mat4 View;
uniform mat4 Proj;
uniform vec2 Dims;
uniform float MipLevels;
uniform float near;
uniform float far;
uniform float dist_threshold = 1;

out vec3 reflectionColor;

const int RAYMARCH_START_LEVEL = 1;
const int RAYMARCH_STOP_LEVEL = 0;
const int RAYMARCH_MAX_ITERATIONS = 128;
const float epsilon = 0.00001;
const float FADE_START = 0.9;
const float FADE_END = 1;

float linearizeDepth(float d)
{
    const float projA = (far + near) / (far - near);
    const float projB = (2 * far * near) / (far - near);

    return projB / ((d * 2 - 1) - projA);
}

vec3 Ray(vec3 origin, vec3 direction, float depth)
{
    return origin + direction * depth;
}

vec2 GetCellCount(float level)
{
    return floor(Dims / ((level > 0.0) ? exp2(level) : 1.0));
}

vec2 GetCell(vec2 ray, vec2 cellCount)
{
    return floor(ray * cellCount);
}

bool CrossedCellBoundary(vec2 A, vec2 B)
{
    return any(notEqual(A, B));
}

vec3 IntersectCellBoundary(vec3 origin, vec3 direction, vec2 cellIdx, vec2 cellCount, vec2 crossStep, vec2 crossOffset)
{
    vec2 cell = cellIdx + crossStep;
    cell /= cellCount;
    cell += crossOffset;

    vec2 delta = cell - origin.xy;
    delta /= direction.xy;

    float t = min(delta.x, delta.y);

    return Ray(origin, direction, t);
}

vec3 HiZTrace(vec3 p, vec3 v)
{
    const vec2 offsetEpsilon = vec2(2.0) / Dims;
    //compute base data
    vec2 crossStep = vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? 1.0 : -1.0);
    vec2 crossOffset = crossStep * offsetEpsilon;
    crossStep = step(vec2(0), crossStep);

    vec3 ray = p;
    
    //get direction that goes from near with z=0 -> on near plane, z=1 -> on far plane
    vec3 d = v / v.z;
    //compute ray origin on near plane
    vec3 o = Ray(p, d, -p.z);

    int level = RAYMARCH_START_LEVEL;
    int iteration = 0;

    //advance ray one cell to not have immediate self-intersection
    vec2 startCellCount = GetCellCount(float(level));
    vec2 rayCell = GetCell(ray.xy, startCellCount);
    ray = IntersectCellBoundary(o, d, rayCell, startCellCount, crossStep, crossOffset);

    while (level >= RAYMARCH_STOP_LEVEL && iteration < RAYMARCH_MAX_ITERATIONS)
    {
        //advance to bottom of current cell
        float min_z = textureLod(HiZBuffer, ray.xy, level).r;

        //advance only if we would not be going back up
        vec3 tmpRay = Ray(o, d, max(min_z, ray.z));

        //compare cells
        vec2 cellCount = GetCellCount(float(level));
        vec2 currCell = GetCell(ray.xy, cellCount);
        vec2 nextCell = GetCell(tmpRay.xy, cellCount);

        //if we crossed a cell boundary, step only to the boundary instead
        if (CrossedCellBoundary(currCell, nextCell))
        {
            tmpRay = IntersectCellBoundary(o, d, currCell, cellCount, crossStep, crossOffset);
            level = min(level + 2, int(MipLevels));
        }

        ray = tmpRay;

        level--;
        iteration++;
    }
    
    return ray;
/*
    float rel_iteration = float(iteration) / float(RAYMARCH_MAX_ITERATIONS);
    rel_iteration *= 4.0;

    if (rel_iteration < 1)
        return mix(vec3(1, 0, 0), vec3(1, 1, 0), clamp(rel_iteration, 0.0, 1.0));
    else if (rel_iteration < 2)
        return mix(vec3(1, 1, 0), vec3(0, 1, 0), clamp(rel_iteration - 1, 0.0, 1.0));
    else if (rel_iteration < 3)
        return mix(vec3(0, 1, 0), vec3(0, 1, 1), clamp(rel_iteration - 2, 0.0, 1.0));
    return mix(vec3(0, 1, 1), vec3(0, 0, 1), clamp(rel_iteration - 3, 0.0, 1.0));
*/
}

void main()
{
    //compute reflection vector that we trace along
    
    ivec2 FragIdx = ivec2(gl_FragCoord.xy);
    vec2 uv = vec2(FragIdx) / Dims;
    float depth = texelFetch(HiZBuffer, FragIdx, 0).r;

    //get world space data from g-buffer
    vec3 pos_w = texelFetch(Position, FragIdx, 0).xyz;
    vec3 norm_w = texelFetch(Normal, FragIdx, 0).xyz;

    //convert to view space and compute view space reflection vector
    vec3 pos_v = (View * vec4(pos_w, 1)).xyz;
    vec3 norm_v = (View * vec4(norm_w, 0)).xyz;

    if (all(equal(norm_v, vec3(0))))
        discard;

    //normalize just to be safe
    norm_v = normalize(norm_v);

    //we are in view space, so our view vector is just the normalized position directly
    vec3 view_vec = normalize(pos_v);
    
    vec3 reflect_v = reflect(view_vec, norm_v);

    vec3 p_prime_v = pos_v + reflect_v;

    vec4 p_prime_clip = Proj * vec4(p_prime_v, 1);

    vec3 p_prime_ss = p_prime_clip.xyz / p_prime_clip.w;
    p_prime_ss.xyz = p_prime_ss.xyz * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);

    vec3 p_ss = vec3(uv, depth);
    vec3 reflect_ss = p_prime_ss - p_ss;

    //don't try to trace rays that are towards the camera, or too close to parallel the image plane
    if (reflect_ss.z < epsilon)
        discard;

    //we now have our point and reflected vector, so we can trace;
    //do the actual trace to determine intersection
    vec3 ray_intersection = HiZTrace(p_ss, reflect_ss);

    //detect out-of framebuffer
    if (any(greaterThan(ray_intersection.xy, vec2(1))) || any(lessThan(ray_intersection.xy, vec2(0))))
        discard;

/*
    //detect false positive intersection
    float actualDepth = textureLod(HiZBuffer, ray_intersection.xy, 0).r;
    if (abs(linearizeDepth(actualDepth) - linearizeDepth(ray_intersection.z)) > dist_threshold)
        discard;
*/

    float dir_fade = dot(view_vec, reflect_v);

    vec2 border = abs(ray_intersection.xy - vec2(0.5)) * 2;
    float border_fade = 1.0 - clamp((border.x - FADE_START) / (FADE_END - FADE_START), 0, 1);
    border_fade *= 1.0 - clamp((border.y - FADE_START) / (FADE_END - FADE_START), 0, 1);

    float total_fade = dir_fade * border_fade;
    
    //do the final color lookup based on where our ray ends up
    reflectionColor = textureLod(SceneColor, ray_intersection.xy, 0).rgb * total_fade;
    //reflectionColor = ray_intersection;
}
