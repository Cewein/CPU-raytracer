// raymarcher.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets.

#pragma once

#include <iostream>
#include <cmath>

#include <swizzle/glsl/vector.h>
#include <swizzle/glsl/vector_functions.h>
#include <swizzle/glsl/scalar_support.h>

typedef swizzle::glsl::vector< float, 1 > vec1;
typedef swizzle::glsl::vector< float, 2 > vec2;
typedef swizzle::glsl::vector< float, 3 > vec3;
typedef swizzle::glsl::vector< float, 4 > vec4;

struct ray {
    vec3 A;
    vec3 B;

    ray(vec3 A, vec3 B)
    {
        this->A=A;
        this->B=B;
    }
};
    
struct hitRecord {
    float t;
    vec3 p;
    vec3 normal;
    int mat;
    vec3 color;
    hitRecord()
    {
    }
};

struct sphere 
{
    vec3 center;
    float radius;
    int mat;
    vec3 color;

    sphere(vec3 c, float r, int m, vec3 col)
    {
        center = c;
        radius = r;
        mat = m;
        color = col;
    }
};
    
struct hitableList {
    sphere * list;
    int size;
    
    hitableList(sphere * list, int size)
    {
        this->list = list;
        this->size = size;
    }
};

struct camera {
    vec3 llc;
    vec3 h;
    vec3 v;
    vec3 o;
};

// TODO: Référencez ici les en-têtes supplémentaires nécessaires à votre programme.
