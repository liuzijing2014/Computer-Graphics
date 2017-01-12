/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: <Zijing Liu>
 * *************************
*/ 

#ifdef WIN32 
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <glm/glm.hpp> 
#include <imageIO.h>

#ifdef WIN32
  #define strcasecmp _stricmp
#endif

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100
#define EPSILON 0.00001

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_JPEG;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

// super sampling rate
#define SIMPLE_LIMIT 5
// ray direction difference for the super sampling
#define SIMPLE_DIF 0.001

#define SOFT_SHADOW_LIMITS 15
// soft shadow bias
#define SOFT_SHADOW_BIAS 0.075

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double normal[3];
	double shininess;
};

struct Triangle
{
	Vertex v[3];
};

struct Sphere
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double shininess;
	double radius;
};

struct Light
{
	double position[3];
	double color[3];
};

// stores intersection information
struct Hit
{
    enum 
	{
		TRIANGLE,
		SPHERE,
		NONE
    } hit_obj;
    int index;
};

// stores interpolated date that used for the intersection point
struct HitInfo
{
	glm::dvec3 diffuse;
	glm::dvec3 specular;
	glm::dvec3 normal;
	double shineness;
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

// double xSampleDiff[SIMPLE_LIMIT] = {-SIMPLE_DIF, 0.0, SIMPLE_DIF, -SIMPLE_DIF, SIMPLE_DIF, -SIMPLE_DIF, 0.0, SIMPLE_DIF};
// double ySampleDiff[SIMPLE_LIMIT] = {SIMPLE_DIF, SIMPLE_DIF, SIMPLE_DIF, 0.0, 0.0, -SIMPLE_DIF, -SIMPLE_DIF, -SIMPLE_DIF};

double xSampleDiff[SIMPLE_LIMIT] = {-SIMPLE_DIF, SIMPLE_DIF, -SIMPLE_DIF, SIMPLE_DIF, 0.0};
double ySampleDiff[SIMPLE_LIMIT] = {SIMPLE_DIF, SIMPLE_DIF, -SIMPLE_DIF, -SIMPLE_DIF, 0.0};

// double xSampleDiff[SIMPLE_LIMIT] = {0.0};
// double ySampleDiff[SIMPLE_LIMIT] = {0.0};

glm::dvec3 softShadowBias[SOFT_SHADOW_LIMITS] =
{
	glm::dvec3(SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS),
	glm::dvec3(-SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS),
	glm::dvec3(SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS),
	glm::dvec3(-SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS),
	glm::dvec3(SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS),
	glm::dvec3(-SOFT_SHADOW_BIAS, SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS),
	glm::dvec3(SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS),
	glm::dvec3(-SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS, -SOFT_SHADOW_BIAS),
	glm::dvec3(SOFT_SHADOW_BIAS, 0.0, 0.0),
	glm::dvec3(-SOFT_SHADOW_BIAS, 0.0, 0.0),
	glm::dvec3(0.0, SOFT_SHADOW_BIAS, 0.0),
	glm::dvec3(0.0, -SOFT_SHADOW_BIAS, 0.0),
	glm::dvec3(0.0, 0.0, SOFT_SHADOW_BIAS),
	glm::dvec3(0.0, 0.0, -SOFT_SHADOW_BIAS),
	glm::dvec3(0.0, 0.0, 0.0)
};

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
double SpherehitPosition(const glm::dvec3& start, const glm::dvec3& ray, int sphereIndex);
double TrianglehitPosition(const glm::dvec3 &start, const glm::dvec3 &ray, int triangleIndex);
void InterpolateAtHit(bool isSphere, const glm::dvec3 &hitPos, int index, HitInfo& info);
glm::dvec3 PhongLighting(const HitInfo& info, const glm::dvec3& hitPos, const glm::dvec3& ray);
bool ShadowCheck(const glm::dvec3 &L, const glm::dvec3 &lightPos, const glm::dvec3 &hitPos);

/*
 * Sphere intersection detection. implementation is based on the lecture notes.
 */
double SpherehitPosition(const glm::dvec3& start, const glm::dvec3& ray, int sphereIndex)
{
    const Sphere &sphere = spheres[sphereIndex];

    double a = 1;
    double b = 2 * (ray.x * (start.x - sphere.position[0]) + ray.y * (start.y - sphere.position[1]) + ray.z * (start.z - sphere.position[2]));
    double c = pow(start.x - sphere.position[0], 2) + pow(start.y - sphere.position[1], 2) + pow(start.z - sphere.position[2], 2) - pow(sphere.radius, 2);

    if ((b * b - 4 * c) < 0.0)
    {
		return -1.0;
	}
	
	double t0 = (-b + sqrt(b*b - 4 * c)) / 2;
	double t1 = (-b - sqrt(b*b - 4 * c)) / 2;

	if(t0 > 0 && t1 > 0)
	{
	    return fmin(t0, t1);
	}
	else
	{
	    return -1.0;
	}
}

/*
 * use Möller–Trumbore hitPosion algorithm to fast calculate the hitPosion
 * the implementation is based on the wikipedia pseudo code 
 * from https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm.
 */
double TrianglehitPosition(const glm::dvec3 &start, const glm::dvec3 &ray, int triangleIndex)
{
	const Triangle &triangle = triangles[triangleIndex];

	// Position of 3 vertices of the triangle
	glm::dvec3 v1 = glm::dvec3(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
	glm::dvec3 v2 = glm::dvec3(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
	glm::dvec3 v3 = glm::dvec3(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);

	// edges of the triangle
	glm::dvec3 edge1 = v2 - v1;
	glm::dvec3 edge2 = v3 - v1;

	glm::dvec3 P = glm::cross(ray, edge2);
	float det = glm::dot(edge1, P);

	if(det > -EPSILON && det < EPSILON) 
	{
		return -1.0;
	};

	float invdet = 1.f / det;

	glm::dvec3 T = start - v1;
	float u = glm::dot(T, P) * invdet;
	if(u < 0.f || u > 1.f)
	{
	    return -1.0;
	}

	glm::dvec3 Q = glm::cross(T, edge1);
	float v = glm::dot(ray, Q) * invdet;
	if(v < 0.f || (u + v) > 1.f)
	{
	    return -1.0;
	}

	double t = glm::dot(edge2, Q) * invdet;

	if(t > EPSILON)
	{
	    return t;
	}

	return -1.0f;
}

/*
 * Get the hit information at the hit point. interpolation is used.
 */
void InterpolateAtHit(bool isSphere, const glm::dvec3 &hitPos, int index, HitInfo& info)
{
	// interpolate based on sphere model
	if(isSphere)
	{
	    const Sphere &sphere = spheres[index];

		// normal
	    info.normal = glm::normalize(glm::dvec3(hitPos.x - sphere.position[0], 
												hitPos.y - sphere.position[1], 
												hitPos.z - sphere.position[2]));

	    // diffuse power
	    info.diffuse = glm::dvec3(sphere.color_diffuse[0], sphere.color_diffuse[1], sphere.color_diffuse[2]);

		// specular power
	    info.specular = glm::dvec3(sphere.color_specular[0], sphere.color_specular[1], sphere.color_specular[2]);

		// shineness
		info.shineness = sphere.shininess;
	}
	// interpolate based on triangle model
	else
	{
	    const Triangle &triangle = triangles[index];

		// Position of 3 vertices of the triangle
		glm::dvec3 v1 = glm::dvec3(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
		glm::dvec3 v2 = glm::dvec3(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
		glm::dvec3 v3 = glm::dvec3(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);

		// Barycentric coordinates
		glm::dvec3 AB = v1 - hitPos;
		glm::dvec3 AC = v2 - hitPos;
		double a = glm::length(glm::cross(AB, AC)) / 2.f;

		AB = v2 - hitPos;
		AC = v3 - hitPos;
		double b = glm::length(glm::cross(AB, AC)) / 2.f;

		AB = v1 - v3;
		AC = v2 - v3;
		double total = glm::length(glm::cross(AB, AC)) / 2.f;

		double c = total - a - b;
		double inveTotal = 1.f / total;

		// interpolate normal
		info.normal = inveTotal * ( a * glm::dvec3(triangle.v[2].normal[0], triangle.v[2].normal[1], triangle.v[2].normal[2]) +
					   				b * glm::dvec3(triangle.v[0].normal[0], triangle.v[0].normal[1], triangle.v[0].normal[2]) +
					  		 		c * glm::dvec3(triangle.v[1].normal[0], triangle.v[1].normal[1], triangle.v[1].normal[2])  );

		// interpolate diffuse
		info.diffuse = inveTotal * ( a * glm::dvec3(triangle.v[2].color_diffuse[0], triangle.v[2].color_diffuse[1], triangle.v[2].color_diffuse[2]) +
					   				 b * glm::dvec3(triangle.v[0].color_diffuse[0], triangle.v[0].color_diffuse[1], triangle.v[0].color_diffuse[2]) +
					  		 		 c * glm::dvec3(triangle.v[1].color_diffuse[0], triangle.v[1].color_diffuse[1], triangle.v[1].color_diffuse[2])  );
		
		// interpolate specular
		info.specular = inveTotal * ( a * glm::dvec3(triangle.v[2].color_specular[0], triangle.v[2].color_specular[1], triangle.v[2].color_specular[2]) +
					   				  b * glm::dvec3(triangle.v[0].color_specular[0], triangle.v[0].color_specular[1], triangle.v[0].color_specular[2]) +
					  		 		  c * glm::dvec3(triangle.v[1].color_specular[0], triangle.v[1].color_specular[1], triangle.v[1].color_specular[2])  );

		// interpolate shineness
		info.shineness = inveTotal * ( a * triangle.v[2].shininess +
					   				   b * triangle.v[0].shininess +
					  		 		   c * triangle.v[1].shininess );
	}
}

/*
 * Check if the pixel is within the shadow area for the specific light
 */
bool ShadowCheck(const glm::dvec3 &L, const glm::dvec3 &lightPos, const glm::dvec3 &hitPos)
{
		unsigned i;
		double min_depth = DBL_MAX;
		bool hit = false;

		for (i = 0; i < num_spheres; ++i)
		{
			double t = SpherehitPosition(hitPos, L, i);
			if(t != -1.0 && t < min_depth)
			{
			    hit = true;
			    min_depth = t;
			}
		}

		for (i = 0; i < num_triangles; ++i)
		{
			double t = TrianglehitPosition(hitPos, L, i);
			if(t != -1.0 && t < min_depth)
			{
			    hit = true;
			    min_depth = t;
			}
		}

		if(hit)
		{
		    double lightDis = glm::length(lightPos - hitPos);
		    double blockDis = glm::length(L * min_depth);
			if((blockDis-EPSILON) < lightDis)
			{
			    return false;
			}
			else
			{
			    return true;
			}
		}
		return true;
}

/*
 * Calculate the final color based on phong lighting model
 */
glm::dvec3 PhongLighting(const HitInfo& info, const glm::dvec3& hitPos, const glm::dvec3& ray)
{
    glm::dvec3 finalColor = glm::dvec3(ambient_light[0], ambient_light[1], ambient_light[2]);
    glm::dvec3 N = glm::normalize(info.normal);
    for (unsigned i = 0; i < num_lights; ++i)
    {
		const Light& light = lights[i];
		glm::dvec3 col = glm::dvec3(0.0, 0.0, 0.0);
		for (unsigned j = 0; j < SOFT_SHADOW_LIMITS; ++j)
		{
		    glm::dvec3 lightPos = glm::dvec3(light.position[0], light.position[1], light.position[2]) + softShadowBias[j];
		    glm::dvec3 L = glm::normalize(lightPos - hitPos);
		    // if not within the shadow, do lighting calculation
		    if (ShadowCheck(L, lightPos, hitPos))
		    {
				glm::dvec3 V = glm::normalize(ray * -1.0);
				glm::dvec3 R = glm::normalize(N * glm::dot(L, N) * 2.0 - L);
				double LDotN = fmax(glm::dot(L, N), 0.0);
				double RDotV = fmax(glm::dot(R, V), 0.0);

				// apply lighting effects
				col += glm::dvec3(light.color[0], light.color[1], light.color[2]) *
										(info.diffuse * LDotN + info.specular * (pow(RDotV, info.shineness)));
			}
		}
		col /= (1.0 * SOFT_SHADOW_LIMITS);
		finalColor += col;
    }
    finalColor = glm::clamp(finalColor, 0.0, 1.0);
    return finalColor * 255.0;
}

/*
 * Initiate ray tracying and do pixel color calculation.
 */
void draw_scene()
{
	//a simple test output
	for(unsigned int x=0; x<WIDTH; x++)
	{
		glPointSize(2.0);  
		glBegin(GL_POINTS);
		for(unsigned int y=0; y<HEIGHT; y++)
		{
		    glm::dvec3 color = glm::dvec3(0.0, 0.0, 0.0);
			glm::dvec3 cray;
			glm::dvec3 origin = glm::dvec3(0.0, 0.0, 0.0);
			double radFov = fov / 180 * M_PI;
			cray.x = (4.0 / 3.0) * tan(radFov / 2.0) * ((double)x - 320.0) / 320.0;
			cray.y = tan(radFov / 2.0) * ((double)y - 240.0) / 240.0;
			cray.z = -1;

			for (unsigned int w = 0; w < SIMPLE_LIMIT; ++w)
		    {
				glm::dvec3 ray = cray;
				ray.x += xSampleDiff[w];
				ray.y += ySampleDiff[w];

				ray = glm::normalize(ray);

				unsigned i;
				double min_depth = DBL_MAX;
				Hit hit;
				hit.hit_obj = Hit::NONE;

				for (i = 0; i < num_spheres; ++i)
				{
					double t = SpherehitPosition(origin, ray, i);
					if (t != -1.0 && t < min_depth)
					{
						min_depth = t;
						hit.hit_obj = Hit::SPHERE;
						hit.index = i;
					}
				}

				for (i = 0; i < num_triangles; ++i)
				{
					double t = TrianglehitPosition(origin, ray, i);
					if (t != -1.0 && t < min_depth)
					{
					min_depth = t;
					hit.hit_obj = Hit::TRIANGLE;
					hit.index = i;
					}
				}

				if (hit.hit_obj == Hit::SPHERE)
				{
					glm::dvec3 intersecPos = origin + ray * min_depth;
					HitInfo info;
					InterpolateAtHit(true, intersecPos, hit.index, info);
					color += PhongLighting(info, intersecPos, ray);
				}
				else if (hit.hit_obj == Hit::TRIANGLE)
				{
					glm::dvec3 intersecPos = origin + ray * min_depth;
					HitInfo info;
					InterpolateAtHit(false, intersecPos, hit.index, info);
					color += PhongLighting(info, intersecPos, ray);
				}
				else
				{
					color += glm::dvec3(255.0, 255.0, 255.0);
				}
			}
			color /= SIMPLE_LIMIT;
			color = glm::clamp(color, 0.0, 255.0);
			plot_pixel(x, y, color.x, color.y, color.z);
		}
		glEnd();
		glFlush();
	}
	printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
	glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	buffer[y][x][0] = r;
	buffer[y][x][1] = g;
	buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	plot_pixel_display(x,y,r,g,b);
	if(mode == MODE_JPEG)
	{
		plot_pixel_jpeg(x,y,r,g,b);
	}
}

void save_jpg()
{
	printf("Saving JPEG file: %s\n", filename);

	ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
	if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
	{
		printf("Error in Saving\n");
	}
	else 
	{
		printf("File saved Successfully\n");
	}
}

void parse_check(const char *expected, char *found)
{
	if(strcasecmp(expected,found))
	{
		printf("Expected '%s ' found '%s '\n", expected, found);
		printf("Parse error, abnormal abortion\n");
		exit(0);
	}
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
	char str[100];
	fscanf(file,"%s",str);
	parse_check(check,str);
	fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
	printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
	char str[100];
	fscanf(file,"%s",str);
	parse_check("rad:",str);
	fscanf(file,"%lf",r);
	printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
	char s[100];
	fscanf(file,"%s",s);
	parse_check("shi:",s);
	fscanf(file,"%lf",shi);
	printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
	FILE * file = fopen(argv,"r");
	int number_of_objects;
	char type[50];
	Triangle t;
	Sphere s;
	Light l;
	fscanf(file,"%i", &number_of_objects);

	printf("number of objects: %i\n",number_of_objects);

	parse_doubles(file,"amb:",ambient_light);

	for(int i=0; i<number_of_objects; i++)
	{
		fscanf(file,"%s\n",type);
		printf("%s\n",type);
		if(strcasecmp(type,"triangle")==0)
		{
			printf("found triangle\n");
			for(int j=0;j < 3;j++)
			{
				parse_doubles(file,"pos:",t.v[j].position);
				parse_doubles(file,"nor:",t.v[j].normal);
				parse_doubles(file,"dif:",t.v[j].color_diffuse);
				parse_doubles(file,"spe:",t.v[j].color_specular);
				parse_shi(file,&t.v[j].shininess);
			}

			if(num_triangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				exit(0);
			}
			triangles[num_triangles++] = t;
		}
		else if(strcasecmp(type,"sphere")==0)
		{
			printf("found sphere\n");

			parse_doubles(file,"pos:",s.position);
			parse_rad(file,&s.radius);
			parse_doubles(file,"dif:",s.color_diffuse);
			parse_doubles(file,"spe:",s.color_specular);
			parse_shi(file,&s.shininess);

			if(num_spheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				exit(0);
			}
			spheres[num_spheres++] = s;
		}
		else if(strcasecmp(type,"light")==0)
		{
			printf("found light\n");
			parse_doubles(file,"pos:",l.position);
			parse_doubles(file,"col:",l.color);

			if(num_lights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				exit(0);
			}
			lights[num_lights++] = l;
		}
		else
		{
			printf("unknown type in scene description:\n%s\n",type);
			exit(0);
		}
	}
	return 0;
}

/*
 * Adding minor lights for each major light. It is for the soft shadowing.
 */
void display()
{
}

void init()
{
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,WIDTH,0,HEIGHT,1,-1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  	//hack to make it only draw once
	static int once=0;
	if(!once)
	{
		draw_scene();
		if(mode == MODE_JPEG)
		save_jpg();
	}
	once=1;
}

int main(int argc, char ** argv) 
{
	if ((argc < 2) || (argc > 3))
	{  
		printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
		exit(0);
	}
	if(argc == 3)
	{
		mode = MODE_JPEG;
		filename = argv[2];
	}
	else if(argc == 2)
		mode = MODE_DISPLAY;

	glutInit(&argc,argv);
	loadScene(argv[1]);

	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(WIDTH,HEIGHT);
	int window = glutCreateWindow("Ray Tracer");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	init();
	glutMainLoop();
}

