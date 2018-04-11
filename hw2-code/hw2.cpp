/*
 CSCI 420 Computer Graphics, USC
 Assignment 2: Roller Coaster
 Summary: Roller coaster code with extra credit opportunities implemented (see README.md)
 Student Name: Aditya Aggarwal
 Student username: agga140@usc.edu
 */

#include <iostream>
#include <cstring>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"
#include <vector>
#include <array>
#include <time.h>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctime>
#include <glm/gtc/type_ptr.hpp>


#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#ifdef WIN32
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };


//plane points
float plane[6][3] = {{-500, 0, -500},{-500, 0, 500}, {500, 0, 500}, {-500, 0, -500}, {500, 0, 500}, {500, 0, -500}};

//viewport
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";



//int screenshot_counter = 118;


ImageIO * heightmapImage;

//declared variables added by Aditya Aggarwal (as part of assignment)

//timer - for mation on tracks
double real_start = std::clock();




BasicPipelineProgram * pipelineProgram = new BasicPipelineProgram(); //pipeline program

//VAOs
GLuint vao_spline_points;
GLuint vao_ground;
GLuint vao_sky;
GLuint vao_track;

//Texture Handles
GLuint groundHandle;
GLuint splineTexHandle;
GLuint splineLineTexHandle;
GLuint skyHandle;
glm::vec3 v_vec;


//store previous vectors to render back plane of track
glm::vec3 stored_bvec;
glm::vec3 stored_nvec;
glm::vec3 stored_xyz;



int screenshot_counter = 0;

//vertices for track
vector<float> track_vertices;
vector<float> track_vertices_normals;
vector<float> track_vertices_texels;



GLuint buffer_two;//buffer for vertices plane
GLuint skydome_points;//buffer for skydome points
GLuint track_points; //buffer for track


OpenGLMatrix * openGLMatrix = new OpenGLMatrix(); //matrix class used for projection and model view matrices




// represents one control point along the spline
struct Point
{
    double x;
    double y;
    double z;
};

// spline struct
// contains how many control points the spline has, and an array of control points
struct Spline
{
    int numControlPoints;
    Point * points;
};



// total number of splines
int numSplines;


int global_u; //current u
int global_spline_amount; //number of cubic curves per spline
int global_ymax; //maximum y value of track + 50 for realistic motion/change in u
int global_index_spline; //curent index of spline array


//sky dome information
vector<float> skyDomeVertices;
vector<float> skyDomeTexels;

Spline * splines; //spline array


time_t early_timer;
time_t late_timer;



//generate a sky dome to model the sky - rendered w/ series of triangle strips
void generateSkyDome() {
    float radius = 500.0; //radius of sky dome
    
    OpenGLMatrix rotationMatrix;
    glm::vec4 center_point = glm::vec4(0.0, radius, 0.0, 0.0);
    
    
    for(int i=0; i<90; ++i) { //to make a hemispher 90 degrees vertical rotation
        for(int j=0; j<=360; ++j) { //360 degrees horizontal rotation
            
            
            //upper coordinate
            
            rotationMatrix.LoadIdentity();
            rotationMatrix.Rotate(i, 1, 0, 0);
            rotationMatrix.Rotate(j, 0, 1, 0);
            
            float m[16];
            
            rotationMatrix.GetMatrix(m);
            
            glm::mat4 rotMat = glm::make_mat4(m);
            glm::vec4 newVec = center_point*rotMat;
            
            skyDomeVertices.push_back(newVec.x);
            skyDomeVertices.push_back(newVec.y);
            skyDomeVertices.push_back(newVec.z);
            
            
            skyDomeTexels.push_back(i/90.0);
            skyDomeTexels.push_back(j/360.0);
           
            
            
            
            
            //lower coordinate
            
            
            
            rotationMatrix.LoadIdentity();
            rotationMatrix.Rotate(i+1, 1, 0, 0);
            rotationMatrix.Rotate(j, 0, 1, 0);
            
            rotationMatrix.GetMatrix(m);
            rotMat = glm::make_mat4(m);
            newVec =  center_point*rotMat;
            
            
            skyDomeVertices.push_back(newVec.x);
            skyDomeVertices.push_back(newVec.y);
            skyDomeVertices.push_back(newVec.z);
            
            skyDomeTexels.push_back(i/90.0);
            skyDomeTexels.push_back(j/360.0);
            
            
            
        }
    }
    
    
    
    
    
}





int loadSplines(char * argv)
{
    char * cName = (char *) malloc(128 * sizeof(char));
    FILE * fileList;
    FILE * fileSpline;
    int iType, i = 0, j, iLength;

    // load the track file
    fileList = fopen(argv, "r");

    if (fileList == NULL)
    {
        printf ("can't open file\n");
        exit(1);
    }
    
    
    
    // stores the number of splines in a global variable
    fscanf(fileList, "%d", &numSplines);
    numSplines = numSplines;
    
    splines = (Spline*) malloc(numSplines * sizeof(Spline));
    cout << numSplines << " num splines!" << endl;
    // reads through the spline files
    for (j = 0; j < numSplines; j++)
    {
        i = 0;
        fscanf(fileList, "%s", cName);
        fileSpline = fopen(cName, "r");
        if (fileSpline == NULL)
        {
            printf ("can't open file 2\n");
            exit(1);
        }
        
        // gets length for spline file
        fscanf(fileSpline, "%d %d", &iLength, &iType);
        
        // allocate memory for all the points
        splines[j].points = (Point *)malloc(iLength * sizeof(Point));
        splines[j].numControlPoints = iLength;
        
        // saves the data to the struct
        while (fscanf(fileSpline, "%lf %lf %lf",
                      &splines[j].points[i].x,
                      &splines[j].points[i].y,
                      &splines[j].points[i].z) != EOF)
        {
            i++;
        }
    }
    
    free(cName);
    
    return 0;
}





//allows for motion on spline

void moveonSpline() {
    
    int num_of_control_points = splines[global_index_spline].numControlPoints;
    float s = 0.5; //tension parameter
    glm::mat4 B = glm::mat4(-s, 2*s, -s, 0, 2-s, s-3, 0,1, s-2, 3-2*s, s, 0, s, -s, 0, 0); //basis matrix
    
    
    
    glm::mat3x4 C = glm::mat3x4((float)splines[global_index_spline].points[global_spline_amount-1].x, (float)splines[global_index_spline].points[global_spline_amount].x, (float)splines[global_index_spline].points[global_spline_amount+1].x, (float)splines[global_index_spline].points[global_spline_amount+2].x, (float)splines[global_index_spline].points[global_spline_amount-1].y, (float)splines[global_index_spline].points[global_spline_amount].y, (float)splines[global_index_spline].points[global_spline_amount+1].y, (float)splines[global_index_spline].points[global_spline_amount+2].y,(float)splines[global_index_spline].points[global_spline_amount-1].z, (float)splines[global_index_spline].points[global_spline_amount].z, (float)splines[global_index_spline].points[global_spline_amount+1].z, (float)splines[global_index_spline].points[global_spline_amount+2].z); //defining control matrix
    
    glm::mat3x4 spline_matrix = B * C;
    float u_decimal = global_u/1000.0;
   
    
    //calculate normal, binormal, and xyz vector
    glm::vec4 paramater = glm::vec4(pow(u_decimal, 3), pow(u_decimal, 2), pow(u_decimal, 1), 1);
    glm::vec4 tangent_parameter = glm::vec4(3*pow(u_decimal, 2), 2*u_decimal, 1, 0);
    glm::vec3 xyzprime = tangent_parameter * spline_matrix;
    float xyzprimeMag = glm::length(xyzprime);
    xyzprime = glm::normalize(xyzprime);
    glm::vec3 xyz =  paramater * spline_matrix;
    glm::vec3 nvec = cross(xyz, v_vec);
    glm::vec3 bvec = cross(xyzprime, nvec);
    bvec = glm::normalize(bvec);
    
    
    
    v_vec = xyz; //reset v_vec to ensure binormal is normal
    
    
    xyz = xyz *10.0f; //scale motion as track is scaled
    
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
    openGLMatrix->LoadIdentity();
    
    
    openGLMatrix->LookAt( xyz.x + bvec.x, xyz.y + 50  +bvec.y, xyz.z +bvec.z, (xyz.x) + xyzprime.x + bvec.x , (xyz.y) + xyzprime.y + 50 + bvec.y,  (xyz.z) + xyzprime.z + bvec.z*0.5, bvec.x , bvec.y  , bvec.z); //look at function to move camera
    
    
    
    global_u = 1 + global_u + (0.001*(std::clock()-real_start)*(sqrt((2*(9.8)*(global_ymax -xyz.y -50 -bvec.y)))))/xyzprimeMag;
    real_start = std::clock(); //calculate next u based on gravity, height, and time
    
    //reset u and increment counter for cubic curves of spine
    if(global_u>=1000) {
        global_u = 1;
        ++global_spline_amount;
        
    }
    
    
    
    
    
}


//creates track from control points and catmull rom splines
void generatePointsFromSplines() {
    
    for (int i = 0; i< numSplines; ++i) {
        int num_of_control_points = splines[i].numControlPoints;
        global_ymax = 0;
        float s = 0.5; //tension parameter
        glm::mat4 B = glm::mat4(-s, 2*s, -s, 0, 2-s, s-3, 0,1, s-2, 3-2*s, s, 0, s, -s, 0, 0); //basis matrix
        
        for(int j=1; j< num_of_control_points-2; ++j) {
            
            
            
            glm::mat3x4 C = glm::mat3x4((float)splines[i].points[j-1].x, (float)splines[i].points[j].x, (float)splines[i].points[j+1].x, (float)splines[i].points[j+2].x, (float)splines[i].points[j-1].y, (float)splines[i].points[j].y, (float)splines[i].points[j+1].y, (float)splines[i].points[j+2].y,(float)splines[i].points[j-1].z, (float)splines[i].points[j].z, (float)splines[i].points[j+1].z, (float)splines[i].points[j+2].z); //defining control matrix
            
            glm::mat3x4 spline_matrix = B * C;
            
            
            for(int u=1; u<=1000; ++u){
                float u_decimal = u/1000.0;
                
                //calculate normal, binormal, and xyz vector
                glm::vec4 paramater = glm::vec4(pow(u_decimal, 3), pow(u_decimal, 2), pow(u_decimal, 1), 1);
                glm::vec3 xyz =  paramater * spline_matrix;
                //update for y max before position vector is normalized
                if((xyz.y*10 + 50) >  global_ymax)
                    global_ymax = xyz.y*10 + 50;
                glm::vec4 tangent_parameter = glm::vec4(3*pow(u_decimal, 2), 2*u_decimal, 1, 0);
                glm::vec3 xyzprime = tangent_parameter * spline_matrix;
                xyzprime = glm::normalize(xyzprime);
                glm::vec3 nvec = cross(v_vec, xyz);
                glm::vec3 bvec = cross(xyzprime, nvec);
                bvec = glm::normalize(bvec);
                v_vec = xyz;
                nvec = glm::normalize(nvec);
                xyz = xyz*10.0f;
                xyz = glm::vec3(xyz.x, xyz.y+50, xyz.z);
                
                
                
                
                //geneate block on track
                if(u%100 == 0) { //intervals of 100 units
                    if(!(u%200==0)) { //back face
                        
                        
                        //normals
                        for(int i=0; i<6; ++i) {
                            track_vertices_normals.push_back(bvec.x);
                            track_vertices_normals.push_back(bvec.y);
                            track_vertices_normals.push_back(bvec.z);
                        }
                        
                        
                        //back plane
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z + nvec.z); //xyz coordinates
                    
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.75); //texels
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z + nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(1);
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y  - 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(1);
                        
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z + nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.75);
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(1);
                        
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.75);
                        
                        //store vectors of back face
                        stored_bvec = bvec;
                        stored_nvec = nvec;
                        stored_xyz = xyz;
                        
                        
                    } else { //front face
                        
                        
                        
                        //normals
                        for(int i=0; i<18; ++i) {
                            track_vertices_normals.push_back(bvec.x);
                            track_vertices_normals.push_back(bvec.y);
                            track_vertices_normals.push_back(bvec.z);
                        }
                        
                        //back plane
                        
                        track_vertices.push_back(stored_xyz.x + 0.2*stored_bvec.x + stored_nvec.x);
                        track_vertices.push_back(stored_xyz.y + 0.2*stored_bvec.y + stored_nvec.y);
                        track_vertices.push_back(stored_xyz.z + 0.2*stored_bvec.z + stored_nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.75);
                        
                        
                        track_vertices.push_back(stored_xyz.x + 0.2*stored_bvec.x - stored_nvec.x);
                        track_vertices.push_back(stored_xyz.y + 0.2*stored_bvec.y - stored_nvec.y);
                        track_vertices.push_back(stored_xyz.z + 0.2*stored_bvec.z - stored_nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.75);
                        
            
                        track_vertices.push_back(xyz.x + 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.5);
                
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.5);
                        
                        
                        track_vertices.push_back(stored_xyz.x + 0.2*stored_bvec.x + stored_nvec.x);
                        track_vertices.push_back(stored_xyz.y + 0.2*stored_bvec.y + stored_nvec.y);
                        track_vertices.push_back(stored_xyz.z + 0.2*stored_bvec.z + stored_nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.75);
                        
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z + nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.5);
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        //bottom plane
                        
                        track_vertices.push_back(stored_xyz.x - 0.2*stored_bvec.x - stored_nvec.x);
                        track_vertices.push_back(stored_xyz.y - 0.2*stored_bvec.y - stored_nvec.y);
                        track_vertices.push_back(stored_xyz.z - 0.2*stored_bvec.z - stored_nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0);
                        
                        
                        track_vertices.push_back(stored_xyz.x - 0.2*stored_bvec.x + stored_nvec.x);
                        track_vertices.push_back(stored_xyz.y - 0.2*stored_bvec.y + stored_nvec.y);
                        track_vertices.push_back(stored_xyz.z - 0.2*stored_bvec.z + stored_nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0);
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.25);
                        
                        
                        track_vertices.push_back(stored_xyz.x - 0.2*stored_bvec.x + stored_nvec.x);
                        track_vertices.push_back(stored_xyz.y - 0.2*stored_bvec.y + stored_nvec.y);
                        track_vertices.push_back(stored_xyz.z - 0.2*stored_bvec.z + stored_nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0);
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.25);
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z + nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.25);
                        
                        
                        
                        //top plane
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.25);
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z + nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.25);
                        
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z + nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.5);
                        
                        
                        track_vertices.push_back(xyz.x - 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y - 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z - 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.25);
                        
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x + nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y + nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z + nvec.z);
                        
                        track_vertices_texels.push_back(0);
                        track_vertices_texels.push_back(0.5);
                        
                        
                        track_vertices.push_back(xyz.x + 0.2*bvec.x - nvec.x);
                        track_vertices.push_back(xyz.y + 0.2*bvec.y - nvec.y);
                        track_vertices.push_back(xyz.z + 0.2*bvec.z - nvec.z);
                        
                        track_vertices_texels.push_back(1);
                        track_vertices_texels.push_back(0.5);
                        
                    }
                    
                }
                
                
            }
            
        }
        
        v_vec = glm::vec3(1,1,0); //reset vector used to calculate first binormal
    }
    
    
}


int initTexture(const char * imageFilename, GLuint textureHandle)
{
    // read the texture image
    ImageIO img;
    ImageIO::fileFormatType imgFormat;
    ImageIO::errorType err = img.load(imageFilename, &imgFormat);
    
    if (err != ImageIO::OK)
    {
        printf("Loading texture from %s failed.\n", imageFilename);
        return -1;
    }
    
    // check that the number of bytes is a multiple of 4
    if (img.getWidth() * img.getBytesPerPixel() % 4)
    {
        printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
        return -1;
    }
    
    // allocate space for an array of pixels
    int width = img.getWidth();
    int height = img.getHeight();
    unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA
    
    // fill the pixelsRGBA array with the image pixels
    memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
    for (int h = 0; h < height; h++)
        for (int w = 0; w < width; w++)
        {
            // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
            pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
            pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
            pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
            pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque
            
            // set the RGBA channels, based on the loaded image
            int numChannels = img.getBytesPerPixel();
            for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
                pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
        }
    
    // bind the texture
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    
    // initialize the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);
    
    // generate the mipmaps for this texture
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // set the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // query support for anisotropic texture filtering
    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    printf("Max available anisotropic samples: %f\n", fLargest);
    // set anisotropic texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);
    
    // query for any errors
    GLenum errCode = glGetError();
    if (errCode != 0)
    {
        printf("Texture initialization error. Error code: %d.\n", errCode);
        return -1;
    }
    
    // de-allocate the pixel array -- it is no longer needed
    delete [] pixelsRGBA;
    
    return 0;
}






//render Function
void renderFunc(){
    //draw
    GLint first = 0;
    
    
    //phong lighting - get handles
    GLuint program = pipelineProgram->GetProgramHandle();
    GLuint h_matka=glGetUniformLocation(program, "matKa");
    GLuint h_matks=glGetUniformLocation(program, "matKs");
    GLuint h_matkd=glGetUniformLocation(program, "matKd");
    GLuint h_matksexp =glGetUniformLocation(program, "matKsExp");
    
    //setting reflection coefficients for track
    float matka[4] = { 0.05, 0.05, 0.05, 0.05};
    float matks[4] = { 0.003, 0.003, 0.003, 0};
    float matkd[4] = { 0.001, 0.001, 0.001, 0};
    
    //upload lighting coefficient vectors
    glUniform4fv(h_matka, 1, matka);
    glUniform4fv(h_matks, 1, matks);
    glUniform4fv(h_matkd, 1, matkd);
    glUniform1f(h_matksexp, 0.8);

    
    //texture mapping for track
    glBindTexture(GL_TEXTURE_2D, splineTexHandle);
    glBindVertexArray(vao_track);
    
    glDrawArrays(GL_TRIANGLES, 0, track_vertices.size()/3);
    
    

    
    
    
    
    //lighting vectors for background
    float matkag[4] = {0.2, 0.2, 0.2, 0};
    float matksg[4] = {0, 0, 0, 0.0};
    float matkdg[4] = {0, 0, 0, 0};
    
    
    //upload lighting coefficients
    glUniform4fv(h_matka, 1, matkag);
    glUniform4fv(h_matks, 1, matksg);
    glUniform4fv(h_matkd, 1, matkdg);
    glUniform1f(h_matksexp, 1);
    
    //sky
    glBindTexture(GL_TEXTURE_2D, skyHandle);
    glBindVertexArray(vao_sky);
    
    for(int i=0; i<90; ++i) //draw sky dome
        glDrawArrays(GL_TRIANGLE_STRIP, i*(360*2+2), 360*2+2);
    
    glBindTexture(GL_TEXTURE_2D, groundHandle); //ground
    
    glBindVertexArray(vao_ground);
    glDrawArrays(GL_TRIANGLES, first, 6); //draw ground plane
    
    
    
    

    glutSwapBuffers();
    glBindVertexArray(0); //unbind vao
}



void initVBO(){
    
    
    //buffer for track
    glGenBuffers(1, &track_points);
    glBindBuffer(GL_ARRAY_BUFFER, track_points);
    
    glBufferData(GL_ARRAY_BUFFER, track_vertices.size()  * sizeof(float) + track_vertices_normals.size() * sizeof(float) + track_vertices_texels.size()*sizeof(float),
                 NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0,track_vertices.size()  * sizeof(float), &track_vertices[0]); //track vertices
    
    glBufferSubData(GL_ARRAY_BUFFER, track_vertices.size()  * sizeof(float),track_vertices_normals.size() * sizeof(float), &track_vertices_normals[0]); //track normals
    
    
    
    glBufferSubData(GL_ARRAY_BUFFER, track_vertices.size()  * sizeof(float) + track_vertices_normals.size() * sizeof(float), track_vertices_texels.size() * sizeof(float), &track_vertices_texels[0]); //track texels
    
    
    
    
    
    
    
    
    
    
    
    
    
    //buffer for points of plane
    glGenBuffers(1, &buffer_two);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_two);
    glBufferData(GL_ARRAY_BUFFER, 6*3*sizeof(float) + 6*2*sizeof(float),
                 NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6*3*sizeof(float), plane); //vertices
    
    
    
    
    //uv coordinates for texture
    float texel_coordinates[6][2] = {{0, 0}, {0, 5}, {5, 5}, {0, 0}, {5, 5}, {5, 0}};
    glBufferSubData(GL_ARRAY_BUFFER, 6*3*sizeof(float), 6*2*sizeof(float), texel_coordinates); //texels
    
    
    //normals
    float normals_plane[6][3] = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}};
    glBufferSubData(GL_ARRAY_BUFFER, 6*3*sizeof(float) + 6*2*sizeof(float),6*3*sizeof(float) , normals_plane); //normals
    
    
    
    
   
    
    
    //buffer for skydome
    glGenBuffers(1, &skydome_points);
    glBindBuffer(GL_ARRAY_BUFFER, skydome_points);
    glBufferData(GL_ARRAY_BUFFER, skyDomeVertices.size()*sizeof(float) + skyDomeTexels.size()*sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, skyDomeVertices.size()*sizeof(float), &skyDomeVertices[0]); //vertices
    glBufferSubData(GL_ARRAY_BUFFER, skyDomeVertices.size()*sizeof(float), skyDomeTexels.size()*sizeof(float), &skyDomeTexels[0]); //texels
    
    
    
}

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
    unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);
    
    ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);
    
    if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
        cout << "File " << filename << " saved successfully." << endl;
    else
        cout << "Failed to save file " << filename << '.' << endl;
    
    delete [] screenshotData;
}

void setTextureUnit(GLint unit)
{
    glActiveTexture(unit); // select the active texture unit
    // get a handle to the “textureImage” shader variable
    GLuint program = pipelineProgram->GetProgramHandle();
    GLint h_textureImage = glGetUniformLocation(program, "textureImage");
    // deem the shader variable “textureImage” to read from texture unit “unit”
    glUniform1i(h_textureImage, unit - GL_TEXTURE0);
}

void displayFunc()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //z - buffer
    
    
    
    
    
    
    
    //model view
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
    float m[16];
    openGLMatrix->GetMatrix(m);
    pipelineProgram->SetModelViewMatrix(m);
    
    
    //projection view
    openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
    float p[16]; // column-major
    openGLMatrix->GetMatrix(p);
    pipelineProgram->SetProjectionMatrix(p);
    
    
    
    
    
    
    
    
    //upload normal matrix
    GLuint program = pipelineProgram->GetProgramHandle();
    GLuint h_normalMatrix =glGetUniformLocation(program, "normalMatrix");
    float n[16];
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
    openGLMatrix->GetNormalMatrix(n); // get normal matrix
    
    GLboolean isRowMajor = GL_FALSE;
    glUniformMatrix4fv(h_normalMatrix, 1, isRowMajor, n);
    
    
    
    
    // setting the light of the sun
    GLuint h_viewLightDirection =glGetUniformLocation(program, "viewLightDirection");
    float lightDirection[3] = { 0, 499, 0};
    glUniform3fv(h_viewLightDirection, 1, lightDirection);
    
    
    
    
    
    
    
    pipelineProgram->Bind(); // bind the pipeline program
    
    setTextureUnit(GL_TEXTURE0); //set texture unit target
    
    renderFunc(); //render the triangle
    
    
}





void bindProgram(){
    
    
    GLuint program = pipelineProgram->GetProgramHandle();
    
    
    
    
    
    
    
    //vao for ground
    glBindBuffer(GL_ARRAY_BUFFER, buffer_two);
    glGenVertexArrays(1, &vao_ground);
    glBindVertexArray(vao_ground);
    const void * offset = (const void*) 0;
    GLuint loc = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
    
    
    loc = glGetAttribLocation(program, "texCoord");
    glEnableVertexAttribArray(loc); // enable the “texCoord” attribute
    offset = (const void*) (6*3*sizeof(float));
    GLsizei stride = 0;
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, stride, offset);
    
    // get location index of the “normal” shader variable
    loc = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(loc); // enable the “normal” attribute
    offset = (const void*) (6*5*sizeof(float));
    GLboolean normalized = GL_FALSE;
    // set the layout of the “normal” attribute data
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);
    
    
    
    
    
    
    
    
    //vao for sky
    glBindBuffer(GL_ARRAY_BUFFER, skydome_points); //position
    glGenVertexArrays(1, &vao_sky);
    glBindVertexArray(vao_sky);
    loc = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(loc);
    offset = (const void*) 0;
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
    
    
    loc = glGetAttribLocation(program, "texCoord"); //tex coordinates
    glEnableVertexAttribArray(loc);
    offset = (const void*) (skyDomeVertices.size()*sizeof(float));
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, stride, offset);
    
    
    
    
    
    
    
    //vao for the track
    glBindBuffer(GL_ARRAY_BUFFER, track_points);
    glGenVertexArrays(1, &vao_track);
    glBindVertexArray(vao_track);
    loc = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(loc);
    offset = (const void*) 0;
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
    
    // get location index of the “normal” shader variable
    loc = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(loc);
    offset = (const void*) (track_vertices.size()*sizeof(float));
    normalized = GL_FALSE;
    // set the layout of the “normal” attribute data
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);
    
    
    loc = glGetAttribLocation(program, "texCoord");
    glEnableVertexAttribArray(loc); // enable the “texCoord” attribute
    offset = (const void*) (track_vertices.size()*2*sizeof(float));
    glVertexAttribPointer(loc, 2, GL_FLOAT, normalized, stride, offset);
    
    
    
    
    
    
    
    
}




void idleFunc()
{
    
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);

    if(global_index_spline < numSplines) {
        if(splines[global_index_spline].numControlPoints -2 !=global_spline_amount) {
            moveonSpline(); //move on current spline
        } else {
            ++global_index_spline; //move to next spline
            global_spline_amount = 1;
            global_u = 1;
        }
    }
    
    
    
    //my own screen shot code for creating an animation -- commented out
    
   /* time(&late_timer); //timer for frame rate when taking screenshots
    
    if(screenshot_counter<500){
     if(screenshot_counter<10) {
     std::stringstream ss;
     ss << "00" << screenshot_counter << ".jpg";
     saveScreenshot(ss.str().c_str());
     } else if (screenshot_counter<100) {
     std::stringstream ss;
     ss << "0" << screenshot_counter << ".jpg";
     saveScreenshot(ss.str().c_str());
     } else {
     std::stringstream ss;
     ss << screenshot_counter << ".jpg";
     saveScreenshot(ss.str().c_str());
     }
     time(&early_timer);
     ++screenshot_counter;
     }
    */
    
    // make the screen update
    glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
    
    
    // setup perspective matrix
    glViewport(0, 0, w, h);
    
    openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
    openGLMatrix->LoadIdentity();
    
    
    
    
    float aspectRatio = 16.0/9;
    openGLMatrix->Perspective(45.0, aspectRatio, 0.01, 5000);
    
    
    //switch back to model view for safety
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
    
    
    
}



void mouseMotionDragFunc(int x, int y)
{
    // mouse has moved and one of the mouse buttons is pressed (dragging)
    
    // the change in mouse position since the last invocation of this function
    int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };
    
    switch (controlState)
    {
            // translate the landscape
        case TRANSLATE:
            if (leftMouseButton)
            {
                // control x,y translation via the left mouse button
                landTranslate[0] += mousePosDelta[0];
                landTranslate[1] -= mousePosDelta[1];
            }
            if (middleMouseButton)
            {
                // control z translation via the middle mouse button
                landTranslate[2] += mousePosDelta[1];
                
            }
            
            break;
            
            // rotate the landscape
        case ROTATE:
            if (leftMouseButton)
            {
                // control x,y rotation via the left mouse button
                landRotate[0] += mousePosDelta[1];
                landRotate[1] += mousePosDelta[0];
            }
            if (middleMouseButton)
            {
                // control z rotation via the middle mouse button
                landRotate[2] += mousePosDelta[1];
            }
            break;
            
            // scale the landscape
        case SCALE:
            if (leftMouseButton)
            {
                // control x,y scaling via the left mouse button
                landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
                landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
            }
            if (middleMouseButton)
            {
                // control z scaling via the middle mouse button
                landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
            }
            break;
    }
    
    
    //rotate
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
    openGLMatrix->Rotate(landRotate[0], 1, 0, 0);
    openGLMatrix->Rotate(landRotate[1], 0, 1, 0);
    openGLMatrix->Rotate(landRotate[2], 0, 0, 1);
    
    //reset
    landRotate[0]=0.0;
    landRotate[1]=0.0;
    landRotate[2]=0.0;
    
    
    //scale
    openGLMatrix->Scale(landScale[0], landScale[1], landScale[2]);
    
    //reset
    landScale[0] = 1.0;
    landScale[1] = 1.0;
    landScale[2] = 1.0;
    
    
    //translate
    openGLMatrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
    
    //reset
    landTranslate[0] =0.0;
    landTranslate[1] = 0.0;
    landTranslate[2] = 0.0;
    
    
    
    
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
    // mouse has moved
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
    // a mouse button has has been pressed or depressed
    
    // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
    switch (button)
    {
        case GLUT_LEFT_BUTTON:
            leftMouseButton = (state == GLUT_DOWN);
            
            break;
            
        case GLUT_MIDDLE_BUTTON:
            middleMouseButton = (state == GLUT_DOWN);
            
            break;
            
        case GLUT_RIGHT_BUTTON:
            rightMouseButton = (state == GLUT_DOWN);
            
            break;
    }
    
    
    // keep track of whether CTRL and SHIFT keys are pressed
    switch (glutGetModifiers())
    {
        case GLUT_ACTIVE_CTRL:
            controlState = TRANSLATE;
            break;
            
        case GLUT_ACTIVE_SHIFT:
            controlState = SCALE;
            break;
            
            // if CTRL and SHIFT are not pressed, we are in rotate mode
        default:
            controlState = ROTATE;
            break;
    }
    
    
    
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27: // ESC key
            exit(0); // exit the program
            break;
            
        case ' ':
            cout << "You pressed the spacebar." << endl;
            break;
            
        case 'x':
            // take a screenshot
            saveScreenshot("screenshot.jpg");
            break;
            
            
            
            
            
    }
}

void initPipelineProgram() {
    //compiles pipeline program
    pipelineProgram = new BasicPipelineProgram();
    pipelineProgram->Init(shaderBasePath);
    GLuint program = pipelineProgram->GetProgramHandle();
    
    
    
    
}

void initScene(int argc, char *argv[])
{
    
     cout << "I am here" << endl;
    //string splines_string = "splines/circle.sp";
    loadSplines(argv[1]);
     cout << "I am here" << endl;
    v_vec = glm::vec3(1,1,0);
    generatePointsFromSplines();
    generateSkyDome();
    
    glGenTextures(1, &groundHandle);
    int code = initTexture("ground_texture.jpg", groundHandle);
    glGenTextures(1, &splineTexHandle);
    int code_spline = initTexture("wood_texture.jpg", splineTexHandle);
    cout << "I am here" << endl;
    int code_spline_line = initTexture("wood_texture.jpg", splineLineTexHandle);
    int code_sky = initTexture("sky_texture.jpg", skyHandle);
    if ((code | code_spline | code_sky) != 0)
    {
        printf("Error loading the texture image.\n");
        exit(EXIT_FAILURE);
    }
    
    global_u = 1;
    global_spline_amount = 0;
    
    openGLMatrix->LoadIdentity();
    //translate camera
    openGLMatrix->LookAt(0, 50 , -50, 0, 50,0, 0.0, 1, 0.0);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    initVBO();
    
    initPipelineProgram(); //for shader compiling
    
    bindProgram(); //for VAOs binding to pipeline program
    
    //time(&early_timer); -- starting timer for animation frames
}




int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "The arguments are incorrect." << endl;
        cout << "usage: ./hw1 <heightmap file>" << endl;
        exit(EXIT_FAILURE);
    }
    
    cout << "Initializing GLUT..." << endl;
    glutInit(&argc,argv);
    
    cout << "Initializing OpenGL..." << endl;
    
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif
    
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(windowTitle);
    
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    
    // tells glut to use a particular display function to redraw
    glutDisplayFunc(displayFunc);
    // perform animation inside idleFunc
    glutIdleFunc(idleFunc);
    // callback for mouse drags
    glutMotionFunc(mouseMotionDragFunc);
    // callback for idle mouse movement
    glutPassiveMotionFunc(mouseMotionFunc);
    // callback for mouse button changes
    glutMouseFunc(mouseButtonFunc);
    // callback for resizing the window
    glutReshapeFunc(reshapeFunc);
    // callback for pressing the keys on the keyboard
    glutKeyboardFunc(keyboardFunc);
    
    // init glew
#ifdef __APPLE__
    // nothing is needed on Apple
#else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
        cout << "error: " << glewGetErrorString(result) << endl;
        exit(EXIT_FAILURE);
    }
#endif
    
    // do initialization
    initScene(argc, argv);
    
    // sink forever into the glut loop
    glutMainLoop();
}


