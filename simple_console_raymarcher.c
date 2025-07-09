#include<stdio.h>
#include<math.h>
#include<windows.h>

#define WIDTH 96 // width in characters
#define HEIGHT 64 // height in characters
#define PIXELDISTANCE 0.03 // FOV
#define MAXMARCHES 100 // maximum number of steps a ray can take
#define MAXDISTANCE 150 // maximum distance a ray can travel
#define DISTANCETHRESHOLD 0.01 // distance from surface to stop a ray

struct bodyData {
	int shapeType; // 0 - sphere, 1 - cube
	int color; // windows terminal color code
	float position[3]; // from the camera perspective: -x = right, -y = forward, +z = up
	float radius;		
};
// This array of structs describes all of the bodies in a scene. The format is described above this line.
// The first element is always the point light source that is used for shading!
struct bodyData bodies[] = { {0,15,{10,10,30},1}, {0,15,{0,0,-2000},1995}, {0,9,{0,0,0},5}, {1,10,{-10,-2,3},3}, {1,12,{12,0,3},3}, {1,13,{-9,5,-1},2} };
// struct bodyData bodies[] = { {0,15,{10,10,30},1}, {0,15,{0,0,-2000},1995}};
// struct bodyData bodies[] = { {0,15,{-500,100,500},1}, {0,10,{0,0,-11000},10995}, {1,15,{-17,0,-4},1}, {1,15,{-17,0,-2},1}, {1,15,{-17,0,0},1}, {1,15,{-17,0,2},1}, {1,15,{8,5,-4},1}, {1,15,{8,5,-2},1}, {1,15,{8,5,0},1}, {1,15,{8,5,2},1}, {0,10,{8,5,6},4}, {0,10,{-17,0,6},4}, {0,12,{5,-7,-4},2.5}, {0,12,{-13,4,-4},2.5}, {0,12,{18,-2,-4},2.5}, {0,12,{-13,-20,-4},2}, {1,14,{-80,-90,45},50}, {1,14,{80,-51,45},50} };

typedef struct rayData {
	float position[3];
	float orientation[3];
	float hitNormal[3];
	float distanceTravelled;
	int jumpCount;
	int hit;
	int hitIndex;
} Ray;

float calculateDistance(float* position, int bodyIndex);
void vectorSubtract(float v1[], float v2[], float r1[], int vectorDimension);
float vectorDotProduct(float v1[], float v2[], int vectorDimension);
void vectorNormalise(float v1[], float r1[], int vectorDimension);
float lerp(float x, float xmin, float xmax, float ymin, float ymax);
void rayMarch(Ray *ray);

int main() {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	char asciiLuminance[] = " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ&@";

	// ray marching, shading, printing
	for (int y=0; y<HEIGHT; y++) {
		for (int x=0; x<WIDTH; x++) {
			float renderBrightness = 0;
			int renderColor = 0;

			// initialize ray (camera setup)
			Ray ray;
			ray.orientation[0] = PIXELDISTANCE*(x-WIDTH/2);
			ray.orientation[1] = -1;
			ray.orientation[2] = -PIXELDISTANCE*(y-HEIGHT/2);
			vectorNormalise(ray.orientation, ray.orientation, 3);
			ray.position[0] = 0;
			ray.position[1] = 15;
			ray.position[2] = 0;

			// camera ray
			rayMarch(&ray);

			if ( ray.hit == 1 ) {
				renderColor = bodies[ray.hitIndex].color;

				// normals
				if (bodies[ray.hitIndex].shapeType == 0) {
					vectorSubtract(ray.position, bodies[ray.hitIndex].position, ray.hitNormal, 3);
				} else {
					for (int i=0; i<3; i++) ray.hitNormal[i] = 0;
					if (ray.position[0] >= bodies[ray.hitIndex].position[0] + bodies[ray.hitIndex].radius) ray.hitNormal[0] = 1;
					else if (ray.position[0] < bodies[ray.hitIndex].position[0] - bodies[ray.hitIndex].radius) ray.hitNormal[0] = -1;
					else if (ray.position[1] >= bodies[ray.hitIndex].position[1] + bodies[ray.hitIndex].radius) ray.hitNormal[1] = 1;
					else if (ray.position[1] < bodies[ray.hitIndex].position[1] - bodies[ray.hitIndex].radius) ray.hitNormal[1] = -1;
					else if (ray.position[2] >= bodies[ray.hitIndex].position[2] + bodies[ray.hitIndex].radius) ray.hitNormal[2] = 1;
					else if (ray.position[2] < bodies[ray.hitIndex].position[2] - bodies[ray.hitIndex].radius) ray.hitNormal[2] = -1;
				}
				vectorNormalise(ray.hitNormal, ray.hitNormal, 3);

				// shading
				const float ambient_light = 0.02;
				const float ao_power = 2;
				vectorSubtract(bodies[0].position, ray.position, ray.orientation, 3);
				vectorNormalise(ray.orientation, ray.orientation, 3);
				renderBrightness = lerp(fmaxf(vectorDotProduct(ray.hitNormal, ray.orientation, 3), 0.0), 0, 1, ambient_light, 1);
				renderBrightness *= pow(lerp(ray.jumpCount, 1, MAXMARCHES, 1, 0), ao_power);	// ambient occlusion & fog effect
				
				// shadow ray
				for (int i=0; i<3; i++) 
					ray.position[i] = ray.position[i] + 10*DISTANCETHRESHOLD * ray.orientation[i];
				rayMarch(&ray);
				if (ray.hitIndex != 0 && ray.hit == 1) renderBrightness = ambient_light;

				renderBrightness = fmaxf(fminf(renderBrightness, 1.0), 0.0);
			}

			// Draw pixel
			SetConsoleTextAttribute(hConsole, renderColor);
			if (!(renderBrightness >= 0.0))
				renderBrightness = 0.0;
			printf("%c ", asciiLuminance[(int)(renderBrightness * (sizeof(asciiLuminance)-1))]);	
		}
		printf("\n");
	}
    SetConsoleTextAttribute(hConsole, 15);
	return 0;
}

void rayMarch(Ray *ray) {
	int bodyIndex, closestIndex;
	float closestDistance = 0;
	ray->hit = ray->jumpCount = ray->distanceTravelled = ray->hitIndex = 0;
	for (int marchCount=0; marchCount<MAXMARCHES && !ray->hit && ray->distanceTravelled<MAXDISTANCE; marchCount++) {
		closestIndex = 0;
		closestDistance = calculateDistance(ray->position, 0);
		for (bodyIndex=1; bodyIndex<sizeof(bodies)/(sizeof(int)*2+sizeof(float)*2); bodyIndex++) {
			float tempDistance = calculateDistance(ray->position, bodyIndex);
			if (closestDistance > tempDistance) {
				closestDistance = tempDistance;
				closestIndex = bodyIndex;
			}
		}
		if ( closestDistance < DISTANCETHRESHOLD ) {
			ray->hit = 1;
			ray->hitIndex = closestIndex;
		} else {
			ray->position[0] = ray->position[0] + closestDistance * ray->orientation[0];
			ray->position[1] = ray->position[1] + closestDistance * ray->orientation[1];
			ray->position[2] = ray->position[2] + closestDistance * ray->orientation[2];
			ray->distanceTravelled = ray->distanceTravelled + closestDistance;
			ray->jumpCount++; 
		}
	}
	return;
}

float calculateDistance(float* position, int bodyIndex) {
	float temp[3] = {0};
	switch (bodies[bodyIndex].shapeType) {
		case 0:
			return sqrt(pow(position[0] - bodies[bodyIndex].position[0], 2)
			 + pow(position[1] - bodies[bodyIndex].position[1], 2)
			 + pow(position[2] - bodies[bodyIndex].position[2], 2)) - bodies[bodyIndex].radius;
		case 1: // Box SDF by Inigo Quilez (https://iquilezles.org/articles/distfunctions/)
			vectorSubtract(bodies[bodyIndex].position, position, temp,3);
			temp[0] = fabsf(temp[0]) - bodies[bodyIndex].radius;
			temp[1] = fabsf(temp[1]) - bodies[bodyIndex].radius;
			temp[2] = fabsf(temp[2]) - bodies[bodyIndex].radius;
			temp[0] = fmaxf(temp[0], 0.0);
			temp[1] = fmaxf(temp[1], 0.0);
			temp[2] = fmaxf(temp[2], 0.0);
			return sqrt(pow(temp[0],2) + pow(temp[1],2) + pow(temp[2],2));
	}
}

void vectorSubtract(float v1[], float v2[], float r1[], int vectorDimension) {
	for (int i=0; i<vectorDimension; i++)
		r1[i] = v1[i] - v2[i];
	return;
}

float vectorDotProduct(float v1[], float v2[], int vectorDimension) {
	float dotProduct = 0;
	for (int i=0; i<vectorDimension; i++)
		dotProduct = dotProduct + (v1[i] * v2[i]);
	return dotProduct;
}

void vectorNormalise(float v1[], float r1[], int vectorDimension) {
	float sum = 0;
	for (int i=0; i<vectorDimension; i++)
		sum = sum + pow(v1[i],2);
	for (int i=0; i<vectorDimension; i++)
		r1[i] = v1[i] / sqrt(sum);
	return;
}

float lerp(float x, float xmin, float xmax, float ymin, float ymax) {
	return ((x-xmin)/(xmax-xmin))*(ymax-ymin) + ymin;
}