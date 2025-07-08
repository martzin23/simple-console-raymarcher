#include<stdio.h>
#include<math.h>
#include<windows.h>

#define XMAX 48*2
#define YMAX 32*2
#define MAXMARCHES 100
#define MAXDISTANCE 150
#define PIXELDISTANCE 0.02
#define DISTANCETHRESHOLD 0.01
#define AMBIENTLIGHT 0.02

struct bodyData {
	int shapeType;
	int materialId;
	float position[3];
	float radius;
};
struct bodyData bodyShape[] = { {0,0,{10,10,30},1}, {0,0,{0,0,-2000},1995}, {0,1,{0,0,0},5}, {1,5,{-10,-2,3},3}, {1,3,{12,0,3},3}, {1,2,{-9,5,-1},2} };
// struct bodyData bodyShape[] = { {0,0,{-500,100,500},1}, {0,5,{0,0,-11000},10995}, {1,0,{-17,0,-4},1}, {1,0,{-17,0,-2},1}, {1,0,{-17,0,0},1}, {1,0,{-17,0,2},1}, {1,4,{-80,-90,45},50}, {1,0,{8,5,-4},1}, {1,0,{8,5,-2},1}, {1,0,{8,5,0},1}, {1,0,{8,5,2},1}, {1,4,{80,-51,45},50}, {0,5,{8,5,6},4}, {0,5,{-17,0,6},4}, {0,3,{5,-7,-4},2.5}, {0,3,{-13,4,-4},2.5}, {0,3,{18,-2,-4},2.5}, {0,3,{-13,-20,-4},2.5} };
// struct bodyData bodyShape[] = { {0,0,{50,20,50},1}, {0,0,{0,0,-2000},1995}, {1,1,{-10,0,-3},2}, {1,1,{-10,0,0},2}, {1,1,{-10,0,4},2}, {1,2,{-10,0,8},4} };

struct rayData {
	float position[3];
	float orientation[3];
} ray[YMAX][XMAX];

float render[YMAX][XMAX][2] = {0};
float distanceTravelled = 0, hitNormal[3];
int jumpCount = 0, rayHit = 0;

float calculateDistance(int x, int y, int bodyIndex);
void vectorSubtract(float v1[], float v2[], float r1[], int vectorLength);
float vectorDotProduct(float v1[], float v2[], int vectorLength);
void vectorNormalise(float v1[], float r1[], int vectorLength);
float lerp(float x, float xmin, float xmax, float ymin, float ymax);
float march(int x, int y);
float absolute(float x);
float maximum(float x, float y);
float minimum(float x, float y);

int main() {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	char asciiLuminance[] = " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ&@";
	// " ,:,-;!~=*#$@"
	// " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ&@"
	int colorPallete[6][4] = {{7, 7, 15, 15}, {9, 9, 9, 9}, {5, 5, 13, 13}, {12, 12, 12, 12}, {14, 14, 14, 14}, {2, 2, 10, 10}};
	//0 gray 1 blue 2 purple 3 red 4 yellow 5 green 

	//Initialize rays (camera setup)
	for (int y=0; y<YMAX; y++) {
		for (int x=0; x<XMAX; x++) {
			ray[y][x].orientation[0] = PIXELDISTANCE*(x-XMAX/2);
			ray[y][x].orientation[1] = -1;
			ray[y][x].orientation[2] = -PIXELDISTANCE*(y-YMAX/2);
			vectorNormalise(ray[y][x].orientation, ray[y][x].orientation, 3);
			ray[y][x].position[0] = 0;
			ray[y][x].position[1] = 15;
			ray[y][x].position[2] = 0;
		}
	}
	
	//Ray marching & printing
	int hitIndex;
	for (int y=0; y<YMAX; y++) {
		for (int x=0; x<XMAX; x++) {
			//View ray
			hitIndex = march(x, y);

			if ( rayHit == 1 ) {
				render[y][x][1] = bodyShape[hitIndex].materialId;

				if (bodyShape[hitIndex].shapeType == 0) {
					vectorSubtract(ray[y][x].position, bodyShape[hitIndex].position, hitNormal, 3);
					vectorNormalise(hitNormal, hitNormal, 3);
				}
				else {
					for (int i=0; i<3; i++) hitNormal[i] = 0;
					if ( ray[y][x].position[0] > bodyShape[hitIndex].position[0] + bodyShape[hitIndex].radius) hitNormal[0] = 1;
					else if ( ray[y][x].position[0] < bodyShape[hitIndex].position[0] - bodyShape[hitIndex].radius) hitNormal[0] = -1;
					else if ( ray[y][x].position[1] > bodyShape[hitIndex].position[1] + bodyShape[hitIndex].radius) hitNormal[1] = 1;
					else if ( ray[y][x].position[1] < bodyShape[hitIndex].position[1] - bodyShape[hitIndex].radius) hitNormal[1] = -1;
					else if ( ray[y][x].position[2] > bodyShape[hitIndex].position[2] + bodyShape[hitIndex].radius) hitNormal[2] = 1;
					else if ( ray[y][x].position[2] < bodyShape[hitIndex].position[2] - bodyShape[hitIndex].radius) hitNormal[2] = -1;
				}

				vectorSubtract(bodyShape[0].position, ray[y][x].position, ray[y][x].orientation, 3);
				vectorNormalise(ray[y][x].orientation, ray[y][x].orientation, 3);
				if ( vectorDotProduct(hitNormal, ray[y][x].orientation, 3) < 0 )
					render[y][x][0] = AMBIENTLIGHT;
				else
					render[y][x][0] = lerp(vectorDotProduct(hitNormal, ray[y][x].orientation, 3), 0, 1, AMBIENTLIGHT, 1);
				// render[y][x][0] = render[y][x][0] * pow(lerp(jumpCount, 1, MAXMARCHES, 1, 0),3);	// ambient occlusion & fog effect
				render[y][x][0] = render[y][x][0] * lerp(distanceTravelled, 0, 150, 1, 0);	// distance fog effect
				
				//Shadow ray
				for (int i=0; i<3; i++) 
					ray[y][x].position[i] = ray[y][x].position[i] + 10*DISTANCETHRESHOLD * ray[y][x].orientation[i];
				hitIndex = march(x, y);
				if ( hitIndex != 0 && rayHit == 1 ) render[y][x][0] = AMBIENTLIGHT;

				if (render[y][x][0]>1) render[y][x][0] = 1;
				if (render[y][x][0]<0) render[y][x][0] = 0;
			}

			//Draw pixel
			for (int i=0; i<sizeof(asciiLuminance); i++) {
				if ( lerp(render[y][x][0], 0, 1, 0, sizeof(asciiLuminance)-1) >= i-0.5 && lerp(render[y][x][0], 0, 1, 0, sizeof(asciiLuminance)-1) < i+0.5 ) {
					SetConsoleTextAttribute(hConsole, colorPallete[(int)render[y][x][1]][(int)lerp(i, 0, sizeof(asciiLuminance)-1, 0, 4)]);
					printf("%c ", asciiLuminance[i]);	
				}
			}
		}
		printf("\n");
	}
    SetConsoleTextAttribute(hConsole, 15);
	return 0;
}

float march(int x, int y) {
	int bodyIndex, closestIndex;
	float closestDistance = 0;
	rayHit = jumpCount = distanceTravelled = 0;
	for (int marchCount=0; marchCount<MAXMARCHES && !rayHit && distanceTravelled<MAXDISTANCE; marchCount++) {
		closestIndex = 0;
		closestDistance = calculateDistance(x, y, 0);
		for (bodyIndex=1; bodyIndex<sizeof(bodyShape); bodyIndex++) {
			if ( closestDistance > calculateDistance(x, y, bodyIndex) ) {
				closestDistance = calculateDistance(x, y, bodyIndex);
				closestIndex = bodyIndex;
			}
		}
		if ( closestDistance < DISTANCETHRESHOLD ) {
			rayHit = 1;
		}
		else {
			ray[y][x].position[0] = ray[y][x].position[0] + closestDistance * ray[y][x].orientation[0];
			ray[y][x].position[1] = ray[y][x].position[1] + closestDistance * ray[y][x].orientation[1];
			ray[y][x].position[2] = ray[y][x].position[2] + closestDistance * ray[y][x].orientation[2];
			distanceTravelled = distanceTravelled + closestDistance;
			jumpCount++; 
		}
	}
	return closestIndex;
}

float calculateDistance(int x, int y, int bodyIndex) {
	float intermediate[3] = {0};
	switch (bodyShape[bodyIndex].shapeType) {
		case 0:
			return sqrt(pow(ray[y][x].position[0]-bodyShape[bodyIndex].position[0], 2)
			 + pow(ray[y][x].position[1]-bodyShape[bodyIndex].position[1], 2)
			 + pow(ray[y][x].position[2]-bodyShape[bodyIndex].position[2], 2)) - bodyShape[bodyIndex].radius;
		case 1: // Cube SDF by Inigo Quilez
			vectorSubtract(bodyShape[bodyIndex].position,ray[y][x].position,intermediate,3);
			intermediate[0] = absolute(intermediate[0]) - bodyShape[bodyIndex].radius;
			intermediate[1] = absolute(intermediate[1]) - bodyShape[bodyIndex].radius;
			intermediate[2] = absolute(intermediate[2]) - bodyShape[bodyIndex].radius;
			intermediate[0] = maximum(intermediate[0], 0.0);
			intermediate[1] = maximum(intermediate[1], 0.0);
			intermediate[2] = maximum(intermediate[2], 0.0);
			return sqrt(pow(intermediate[0],2)+pow(intermediate[1],2)+pow(intermediate[2],2));
	}
}

void vectorSubtract(float v1[], float v2[], float r1[], int vectorLength) {
	for (int i=0; i<vectorLength; i++)
		r1[i] = v1[i] - v2[i];
	return;
}
float vectorDotProduct(float v1[], float v2[], int vectorLength) {
	float dotProduct = 0;
	for (int i=0; i<vectorLength; i++)
		dotProduct = dotProduct + (v1[i] * v2[i]);
	return dotProduct;
}
void vectorNormalise(float v1[], float r1[], int vectorLength) {
	float sum = 0;
	for (int i=0; i<vectorLength; i++)
		sum = sum + pow(v1[i],2);
	for (int i=0; i<vectorLength; i++)
		r1[i] = v1[i] / sqrt(sum);
	return;
}
float lerp(float x, float xmin, float xmax, float ymin, float ymax) {
	return ((x-xmin)/(xmax-xmin))*(ymax-ymin) + ymin;
}
float absolute(float x) {
	if (x<0) return -x;
	else return x;
}
float maximum(float x, float y) {
	if (x<y) return y;
	else return x;
}
float minimum(float x, float y) {
	if (x>y) return y;
	else return x;
}