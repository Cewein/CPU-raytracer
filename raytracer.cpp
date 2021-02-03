#include <fstream>

#include "raytracer.h"

#define FLT_MAX 3.402823466e+38
const float PI  = 3.14159265359;
const float PHI = 1.61803398875;
const int MAX_BOUNCE = 5;

vec3 origin(ray r) {return r.A; }
vec3 direction(ray r) {return r.B; }
vec3 pointAtParameter(ray r, float t) { return r.A + t*r.B; }

bool hitSphere(ray r, float tmin, float tmax, hitRecord * rec, sphere s)
{
    vec3 oc = origin(r) - s.center;
    float a = dot(direction(r),direction(r));
    float b = dot(oc, direction(r));
    float c = dot(oc,oc)-s.radius*s.radius;
    float d = b*b - a*c;
    if (d > 0.) 
    {
        float temp = (-b - sqrt(b*b-a*c))/a;
        if(temp < tmax && temp > tmin)
        {
            rec->t = temp;
            rec->p = pointAtParameter(r,rec->t);
            rec->normal = (rec->p - s.center) / s.radius;
            rec->mat = s.mat;
            rec->color = s.color;
            return true;
        }
        temp = (-b + sqrt(b*b-a*c))/a;
        if(temp < tmax && temp > tmin)
        {
            rec->t = temp;
            rec->p = pointAtParameter(r,rec->t);
            rec->normal = (rec->p - s.center) / s.radius;
            rec->mat = s.mat;
            rec->color = s.color;
            return true;
        }
    }
    return false;
}

bool hit(ray r, float tmin, float tmax, hitRecord * rec, hitableList hitList)
{
    hitRecord * tempRec = new hitRecord();
    bool hitAny = false;
    float closestSoFar = tmax;


    for(int i = 0; i < hitList.size; i++)
    {
        
        if(hitSphere(r,tmin, closestSoFar,tempRec, hitList.list[i]))
        {
            hitAny = true;
            closestSoFar = tempRec->t;
            
        }
    }

    *rec = *tempRec;

    delete tempRec;


    return hitAny;
}

ray getRay(float u, float v, camera cam) { return ray(cam.o,cam.llc + u*cam.h + v*cam.v); }

float random (vec2 st) {
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt= dot(st.xy ,vec2(a,b));
    float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

vec3 randInUnitSphere(vec2 st,int sample) {
    float phi = random(st.yx + vec2(sample)) * 2.0 * 3.14159265;
    float theta = random(st.xy + vec2(sample)) * 3.14169265;
    
    return vec3(cos(phi) * sin(theta), cos(theta), sin(phi) * sin(theta));
}

bool checkRefract(vec3 v, vec3 n, float niOverNt)
{
    vec3 uv = v;
    float dt = dot(uv, n);
    float discriminant = 1.0 - niOverNt * niOverNt * (1.-dt*dt);
    if (discriminant > 0.)
    {
        return true;
    }
    return false;
}

float schlick(float cosine, float refIdx)
{
    float r0 = (1.-refIdx)/(1.+refIdx);
    r0 = r0*r0;
    return r0 + (1.-r0) * pow((1. - cosine),5.);
}

//https://docs.gl/sl4/refract
vec3 refract(vec3 I, vec3 N, float eta)
{
    float k = 1.0 - eta * eta * (1.0 - dot(N,I) * dot(N,I));
    if(k < 0.0)
        return vec3(0.);
    else
        return eta * I - (eta * dot(N,I) + sqrtf(k)) * N;
}
              
vec3 color(ray r, hitableList hitList, vec2 st, int sample)
{
    hitRecord * rec = new hitRecord();
    vec3 unitDirection;
    float t;
    
    vec3 att = vec3(1.);
    
    int bounceSize = MAX_BOUNCE;
    int bounce = 0;
    
    while(hit(r, 0.001, FLT_MAX, rec, hitList) && bounce < bounceSize)
    {
        unitDirection = normalize(direction(r));
        if(rec->mat == 0)
        {
            vec3 target = rec->p + rec->normal + randInUnitSphere(st,sample);
            r = ray(rec->p, target-rec->p);
            att *= rec->color;
        }
        if(rec->mat == 1)
        {
            float globalFuzz = 0.0;// change to 0 for full reflection
            vec3 reflected = reflect(unitDirection, rec->normal);
            r = ray(rec->p, reflected + globalFuzz*randInUnitSphere(st,sample));
            att *= rec->color; // if  att *= rec.color * 50; then the shpere become a light source o_O
        }
        if(rec->mat == 2)
        {
            float refractiveIndex = 1.9;
            vec3 outwardNormal;
            vec3 reflected =reflect(unitDirection, rec->normal);
            float niOverNt;
            vec3 refracted;
            
            float refProb;
            float cosine;
            
            if(dot(unitDirection, rec->normal) > 0.)
            {
                outwardNormal = -rec->normal;
                niOverNt = refractiveIndex;
                cosine = refractiveIndex * dot(direction(r), rec->normal) / length(direction(r));
            }
            else
            {
                outwardNormal = rec->normal;
                niOverNt = 1.0 / refractiveIndex;
                cosine = -dot(direction(r), rec->normal) / length(direction(r));
            }
            if(checkRefract(unitDirection, outwardNormal, niOverNt))
            {
                
                refProb = schlick(cosine, refractiveIndex);
            }
            else
            {
                refProb = 1.0;
            }
            
            if(random(vec2(sample) + st) > refProb)
            {   
            r = ray(rec->p, refract(unitDirection, outwardNormal,niOverNt));
        	}
            else
            {
                r = ray(rec->p, reflected);
            }
            
            //vec3 refracted = refract(unitDirection, rec.normal, 1.0 / refractiveIndex);
        }
        bounce++;
    }

    delete(rec);
    unitDirection = normalize(direction(r));
    t =  (unitDirection.y + 1.);
    return att * t*vec3(0.6,0.8,1.);
}



vec3 trace(vec2 coord, vec2 resolution, int maxSample)
{
    vec2 st = coord.xy/resolution.xy;
    camera cam;
    cam.llc = vec3(-2.,-1.,-1.);
    cam.h = vec3(4.,0.,0.);
    cam.v = vec3(0.,2.25,0.);
    cam.o = vec3(0.);
    
    sphere sList[] = {sphere(vec3(0.,0.,-1.5),0.5,0,vec3(0.8,0.3,0.3)),
        sphere(vec3(-1.,.0,-1.5),0.5,2,vec3(0.8,0.8,0.8)),
        sphere(vec3(1.0,.0,-1.5),0.5,1,vec3(0.8,0.6,0.2)),
        sphere(vec3(0.,-100.5,-1.),100.,0,vec3(0.8,0.8,0.0)),
        sphere(vec3(0.,0.5,-10.),5.,1,vec3(0.8,0.6,0.2))};

    hitableList list(sList,5);
    
    

    //sampling the image
    vec3 col = vec3(0);
    for(int i = 0; i < maxSample; i++)
    {
        vec2 aa = vec2(random(st + vec2(i))) / resolution.xy;
        ray r = getRay(st.x + aa.x, st.y + aa.y, cam);
 	    col += color(r,list,st,i);
    }

    return vec3(col/maxSample);
}

int main()
{
    int imageWidth = 1920;
    int imageHeight = 1080;
	
    std::ofstream ppm;
    ppm.open("picture.ppm");
    ppm << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";
    for(int y = imageHeight; y > 0; y--)
    {
        std::cout << "Scanlines remaining: " << y << ' ' << std::endl;
        for(int x = imageWidth; x > 0; x--)
        {
            vec3 col = trace(vec2(x,y), vec2(imageWidth,imageHeight), 50);
            
            //color grading
            col = pow(col, vec3(0.4545));

            int r = min(255*col.x,255);
            int g = min(255*col.y,255);
            int b = min(255*col.z,255);

            ppm << r << " " << g << " " << b << " " << std::endl;
        }
    }
    ppm.close();
	 
	return 0;
}
