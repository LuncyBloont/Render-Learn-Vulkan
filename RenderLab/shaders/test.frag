#version 450

precision highp float;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;

layout (location = 0) out vec4 fragColor;

void mainImage( out vec4 fragColor, in vec2 fragCoord );

vec4 iResolution = vec4(800.0, 600.0, 1.0, 1.0);
float iTime = 221.54;

void main() {
    mainImage(fragColor, (vec2(position.x, -position.y) + 0.5) * iResolution.xy);
    fragColor *= mix(color, vec4(1.0), 0.6);
}

/*
 * Copyright (c) 2021 LuncyTB. All rights reserved.
 */

#define MAX_DIS 100.0
#define MAX_STEP 64
#define MIN_DIS 0.01

vec3 sun = normalize(vec3(-3., -8., 4.));
vec3 sunCol = vec3(1., .97, .92) * 1.2;

struct mater
{
    vec3 col;
    float smoth;
    float metlc; // Not supported
    vec3 emiss; // Not supported
};

struct idf
{
    float f;
    int id;
};

float sdSphere(in vec3 pos, in float r)
{
    return length(pos) - r;
}

float sdDnut(in vec3 pos, in float r, in float r2)
{
    return length(vec2(length(pos.xz) - r, pos.y)) - r2;
}

float sdClind(in vec3 pos, in float r, in float h)
{
    float l = length(pos.xz);
    float t = abs(pos.y);
    if (t < h)
    {
        if (l > r)
        {
            return l - r;
        }
        else
        {
            return max(l - r, t - h);
        }
    }
    else
    {
        if (l > r)
        {
            return length(vec2(l - r, t - h));
        }
        else
        {
            return t - h;
        }
    }
}

float mmin(in float a, in float b)
{
    float lerp = a > b ? pow(4.0, b - a) / 2. : 1. - pow(4.0, a - b) / 2.;
    return min(a, b);
}

idf idmin(in idf a, in idf b)
{
    idf c;
    if (a.f < b.f) c = a;
    else c = b;
    return c;
}

float scene(in vec3 pos)
{
    float d = MAX_DIS;
    
    d = mmin(sdSphere(pos - vec3(-.2, -.4, 3.), 1.), d);
    d = mmin(sdDnut(pos - vec3(-1., -0.7, 4.), 1.9, 0.2), d);
    d = mmin(sdSphere(pos - vec3(0.4, 0.4, 2.), 0.3), d);
    d = mmin(sdClind(pos - vec3(1.5, -0.2, 2.4), 0.4, 0.6), d);
    d = mmin(sdClind(pos - vec3(0.4, 0.05, 2.), 0.1, 0.6), d);
    
    return d;
}

idf getId(in vec3 pos)
{
    idf d = idf(MAX_DIS, 0);
    
    d = idmin(idf(sdSphere(pos - vec3(-.2, -.4, 3.), 1.), 2), d);
    d = idmin(idf(sdDnut(pos - vec3(-1., -0.7, 4.), 1.9, 0.2), 1), d);
    d = idmin(idf(sdSphere(pos - vec3(0.4, 0.4, 2.), 0.3), 1), d);
    d = idmin(idf(sdClind(pos - vec3(1.5, -0.2, 2.4), 0.4, 0.6), 2), d);
    d = idmin(idf(sdClind(pos - vec3(0.4, 0.05, 2.), 0.1, 0.6), 3), d);

    return d;
}

float rayMarch(in vec3 org, in vec3 dir, in float min_dis, in int steps)
{
    float d = min_dis * 3.;
    for (int i = 0; i < steps; i++)
    {
        float space = scene(org + dir * d);
        d += space;
        if (space < min_dis) break;
    }
    return d;
}

vec3 calNormal(in vec3 pos)
{
    vec2 e = vec2(MIN_DIS, 0.);
    return normalize(
        scene(pos) - vec3(scene(pos - e.xyy), scene(pos - e.yxy), scene(pos - e.yyx))
        );
}

float random(float n) {
    return fract(iTime * 0.23781 + sin(n) * 1000000.);
}

vec3 rand3(vec3 pos)
{
    return normalize(vec3(random(pos.x + cos(pos.z) + sin(pos.y)) - 0.5, 
        random(pos.y + cos(pos.x) + sin(pos.z)) - 0.5, 
        random(pos.z + cos(pos.y) + sin(pos.x)) - 0.5));
}

float calSun(in vec3 pos)
{
    float z = rayMarch(pos, -sun + rand3(pos) * 0.1, MIN_DIS * 4., MAX_STEP / 4);
    if (z < MAX_DIS)
        return 0.0;
    else return 1.0;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    mater ms[10];
    ms[1] = mater(vec3(1., 0.6, 0.4), 0.3, 0.1, vec3(0.));
    ms[2] = mater(vec3(0.4, 0.6, 0.97), 0.98, 0.1, vec3(0.));
    ms[3] = mater(vec3(0.98, 0.87, 0.71), 0.7, 0.1, vec3(0.));

    vec2 uv = (fragCoord.xy - iResolution.xy / 2.) / min(iResolution.x, iResolution.y);
    vec3 ray = normalize(vec3(uv, 0.7));
    vec3 eye = vec3(0.0, cos(iTime), 0.0);
    
    
    float z = rayMarch(eye, ray, MIN_DIS, MAX_STEP);
    vec3 col;
    vec3 _sky = mix(vec3(cos(ray.x * 4.3) * 0.5 + 0.5, sin(ray.y * 4.3) * 0.5 + 0.5, 1.0), vec3(0.7), 0.5);
    // if (z >= MAX_DIS) col = texture(iChannel0, ray).xyz;
    if (z >= MAX_DIS) col = _sky;
    else
    {
        
        vec3 normal = calNormal(eye + ray * z);
        int mid = getId(eye + ray * z).id;
        vec3 pos = eye + ray * z;
        vec3 ref = reflect(ray, normal + (1. - ms[mid].smoth) * rand3(pos));
        
        float z2 = rayMarch(pos, ref, MIN_DIS * 2., MAX_STEP / 2);
        col = max(0., dot(sun, -normal)) * sunCol * ms[mid].col * calSun(eye + ray * z);
        
        // if (z2 >= MAX_DIS / 2.) col += texture(iChannel0, ref).xyz * ms[mid].col;
        if (z2 >= MAX_DIS / 2.) col += _sky * ms[mid].col;
        else
        {
            vec3 normal2 = calNormal(pos + ref * z2);
            int mid2 = getId(pos + ref * z2).id;
            vec3 ref2 = reflect(ref, normal2 + (1. - ms[mid2].smoth) * rand3(pos + ref * z2));
            col += max(0., dot(sun, -normal2)) * sunCol * ms[mid2].col * ms[mid].col * 0.5;
            // col += texture(iChannel0, ref2).xyz * ms[mid2].col * ms[mid].col * 0.5;
            col += _sky * ms[mid2].col * ms[mid].col * 0.5;
        }
        
        
    }
    
    col.r = pow(col.r, 1.5);
    col.g = pow(col.g, 1.3);
    col.b = pow(col.b, 1.3);

    fragColor = vec4(col, 1.0);
}