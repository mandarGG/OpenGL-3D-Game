#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>


#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/Audio.hpp>



using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

sf::SoundBuffer buffer1;
sf::SoundBuffer buffer2;
sf::SoundBuffer buffer3;
sf::SoundBuffer buffer4;
sf::Sound sound1;
sf::Sound sound2;
sf::Sound sound3;
sf::Sound sound4;
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int i1=-1,i2=1,i3=3,i4=0,i5=0,i6=0,i7=0,i8=1,i9=0;
float j=0,j2=1,j3,j4=-1.8,j5=2.2,j6=1.8,j7,j8,j9;
int move=0;
float v1=((rand()%10)*4)/10.0-1.8,v2=((rand()%10)*4)/10.0-1.8;
float v3=((rand()%10)*4)/10.0-1.8,v4=((rand()%10)*4)/10.0-1.8;
float v5=((rand()%10)*4)/10.0-1.8,v6=((rand()%10)*4)/10.0-1.8;
float obstacl=((rand()%10)*4)/10.0-1.8,obstacl1=((rand()%10)*4)/10.0-1.8;
float obstacl2=((rand()%10)*4)/10.0-1.8,obstacl3=((rand()%10)*4)/10.0-1.8;

/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}glm::mat4 rotateRectangle;

/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
        case 'c':
        case 'C':
            rectangle_rot_status = !rectangle_rot_status;
            break;
        case 'p':
        case 'P':
            triangle_rot_status = !triangle_rot_status;
            break;
       case ' ':
        j=5;
            break;
          case 's':
            j2=j2-1;
            break;
        case 'f':
            j2=j2+1;
            break;
        
        case 't':
        sound1.play();
       
        j7=0;
        i1=0,i2=3,i3=0,i4=0,i5=0,i6=0,i7=1,i8=1,i9=0;
            break;
        case 'v':
        sound1.play();
        j7=0;
            i1=-1,i2=1,i3=3,i4=0,i5=0,i6=0,i7=0,i8=1,i9=0;
            break;
        case 'i':
        sound1.play();
        j7=1;
            i1=j4-0.3,i2=j5-0.3,i3=j6-0.3,i4=j4+0.3,i5=j5+0.3,i6=j6+0.3,i7=0,i8=1,i9=0;
            break;
        case 'j':
            j6=j6-0.4;
            break;

    }
}

/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
  switch(key) {
    case GLUT_KEY_LEFT:
    j=1;
    move=move+1;
    if(move%6==0||move%6==1){
obstacl=((rand()%10)*4)/10.0-1.8,obstacl1=((rand()%10)*4)/10.0-1.8;obstacl2=((rand()%10)*4)/10.0-1.8,obstacl3=((rand()%10)*4)/10.0-1.8;
}
if(move%6!=0&&move%6!=1){
v1=((rand()%10)*4)/10.0-1.8,v2=((rand()%10)*4)/10.0-1.8;v3=((rand()%10)*4)/10.0-1.8,v4=((rand()%10)*4)/10.0-1.8;
v5=((rand()%10)*4)/10.0-1.8,v6=((rand()%10)*4)/10.0-1.8;
    }

        break;
    case GLUT_KEY_RIGHT:
    j=2;
    move=move+1;
    if(move%6==0||move%6==1){
obstacl=((rand()%10)*4)/10.0-1.8,obstacl1=((rand()%10)*4)/10.0-1.8;obstacl2=((rand()%10)*4)/10.0-1.8,obstacl3=((rand()%10)*4)/10.0-1.8;
}
if(move%6!=0&&move%6!=1){
v1=((rand()%10)*4)/10.0-1.8,v2=((rand()%10)*4)/10.0-1.8;v3=((rand()%10)*4)/10.0-1.8,v4=((rand()%10)*4)/10.0-1.8;
v5=((rand()%10)*4)/10.0-1.8,v6=((rand()%10)*4)/10.0-1.8;
    }
        break;
    case GLUT_KEY_UP:
    sound3.play();

    j=3;
    move=move+1;
    if(move%6==0||move%6==1){
obstacl=((rand()%10)*4)/10.0-1.8,obstacl1=((rand()%10)*4)/10.0-1.8;obstacl2=((rand()%10)*4)/10.0-1.8,obstacl3=((rand()%10)*4)/10.0-1.8;
}
if(move%6!=0&&move%6!=1){
v1=((rand()%10)*4)/10.0-1.8,v2=((rand()%10)*4)/10.0-1.8;v3=((rand()%10)*4)/10.0-1.8,v4=((rand()%10)*4)/10.0-1.8;
v5=((rand()%10)*4)/10.0-1.8,v6=((rand()%10)*4)/10.0-1.8;
    }
        break;
    case GLUT_KEY_DOWN:
    j=4;
    move=move+1;
    if(move%6==0||move%6==1){
obstacl=((rand()%10)*4)/10.0-1.8,obstacl1=((rand()%10)*4)/10.0-1.8;obstacl2=((rand()%10)*4)/10.0-1.8,obstacl3=((rand()%10)*4)/10.0-1.8;
}
if(move%6!=0&&move%6!=1){
v1=((rand()%10)*4)/10.0-1.8,v2=((rand()%10)*4)/10.0-1.8;v3=((rand()%10)*4)/10.0-1.8,v4=((rand()%10)*4)/10.0-1.8;
v5=((rand()%10)*4)/10.0-1.8,v6=((rand()%10)*4)/10.0-1.8;
    }
        break;
        }
}

 /*Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
  int mandar=1;
}

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_UP)
                triangle_rot_dir *= -1;
            j=1;
    move=move+1;
            break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_UP) {
                rectangle_rot_dir *= -1;
                j=2;
    move=move+1;
            }
            break;
        default:
            break;
    }
}

/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int x, int y)
{
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle,*cube,*cuboid,*cuboi,*cuboi1,*cuboi2,*cuboi3,*cuboi4,*cuboi5,*cuboi6,*cuboi7,*cuboi8,*cuboid1,*cuboid2;
VAO *obstacle1, *obstacle2,*obstacle3,*obstacle4,*obstacle5;
// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.1f,-0.2f,-0.1f, // triangle 1 : begin

     -0.1f,-0.2f, 0.1f,

     -0.1f, 0.2f, 0.1f, // triangle 1 : end

     0.1f, 0.2f,-0.1f, // triangle 2 : begin

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f,-0.1f, // triangle 2 : end

     0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f,0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f,-0.1f,

     0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f, 0.1f,

     -0.1f,-0.2f, 0.1f,

     0.1f,-0.2f, 0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f, 0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f,-0.2f, 0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f, 0.2f,-0.1f,

     -0.1f, 0.2f,-0.1f,

     0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f,-0.1f,

     -0.1f, 0.2, 0.1f,

     0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f, 0.1f,

     0.1f,-0.2f, 0.1f
    

  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0,  // color 1
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0, // color 1
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0,  // color 1
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0,  // color 1
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0,  // color 1
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
  //obstacle1= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   //obstacle2= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   //obstacle3= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   //obstacle4= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   //obstacle5= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createObst()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.1f,-0.2f,-0.1f, // triangle 1 : begin

     -0.1f,-0.2f, 0.1f,

     -0.1f, 0.2f, 0.1f, // triangle 1 : end

     0.1f, 0.2f,-0.1f, // triangle 2 : begin

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f,-0.1f, // triangle 2 : end

     0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f,0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f,-0.1f,

     0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f, 0.1f,

     -0.1f,-0.2f, 0.1f,

     0.1f,-0.2f, 0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f, 0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f,-0.2f, 0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f, 0.2f,-0.1f,

     -0.1f, 0.2f,-0.1f,

     0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f,-0.1f,

     -0.1f, 0.2, 0.1f,

     0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f, 0.1f,

     0.1f,-0.2f, 0.1f
 };
 //static const GLfloat g_color_buffer_data[] = {
static const GLfloat color_buffer_data [] = {
     0.583f,  0.771f,  0.014f,

     0.609f,  0.115f,  0.436f,

     0.327f,  0.483f,  0.844f,

     0.822f,  0.569f,  0.201f,

     0.435f,  0.602f,  0.223f,

     0.310f,  0.747f,  0.185f,

     0.597f,  0.770f,  0.761f,

     0.559f,  0.436f,  0.730f,

     0.359f,  0.583f,  0.152f,

     0.483f,  0.596f,  0.789f,

     0.559f,  0.861f,  0.639f,

     0.195f,  0.548f,  0.859f,

     0.014f,  0.184f,  0.576f,

     0.771f,  0.328f,  0.970f,

     0.406f,  0.615f,  0.116f,

     0.676f,  0.977f,  0.133f,

     0.971f,  0.572f,  0.833f,

     0.140f,  0.616f,  0.489f,

     0.997f,  0.513f,  0.064f,

     0.945f,  0.719f,  0.592f,

     0.543f,  0.021f,  0.978f,

     0.279f,  0.317f,  0.505f,

     0.167f,  0.620f,  0.077f,

     0.347f,  0.857f,  0.137f,

     0.055f,  0.953f,  0.042f,

     0.714f,  0.505f,  0.345f,

     0.783f,  0.290f,  0.734f,

     0.722f,  0.645f,  0.174f,

     0.302f,  0.455f,  0.848f,

     0.225f,  0.587f,  0.040f,

     0.517f,  0.713f,  0.338f,

     0.053f,  0.959f,  0.120f,

     0.393f,  0.621f,  0.362f,

     0.673f,  0.211f,  0.457f,

     0.820f,  0.883f,  0.371f,

     0.982f,  0.099f,  0.879f

 };
 obstacle1= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   obstacle2= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   obstacle3= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   obstacle4= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   obstacle5= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCube ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -2.0f,-2.0f,-2.0f, // triangle 1 : begin

     -2.0f,-2.0f, 2.0f,

     -2.0f, 2.0f, 2.0f, // triangle 1 : end

     2.0f, 2.0f,-2.0f, // triangle 2 : begin

     -2.0f,-2.0f,-2.0f,

     -2.0f, 2.0f,-2.0f, // triangle 2 : end

     2.0f,-2.0f, 2.0f,

     -2.0f,-2.0f,-2.0f,

     2.0f,-2.0f,-2.0f,

     2.0f, 2.0f,-2.0f,

     2.0f,-2.0f,-2.0f,

     -2.0f,-2.0f,-2.0f,

     -2.0f,-2.0f,-2.0f,

     -2.0f, 2.0f, 2.0f,

     -2.0f, 2.0f,-2.0f,

     2.0f,-2.0f, 2.0f,

     -2.0f,-2.0f, 2.0f,

     -2.0f,-2.0f,-2.0f,

     -2.0f, 2.0f, 2.0f,

     -2.0f,-2.0f, 2.0f,

     2.0f,-2.0f, 2.0f,

     2.0f, 2.0f, 2.0f,

     2.0f,-2.0f,-2.0f,

     2.0f, 2.0f,-2.0f,

     2.0f,-2.0f,-2.0f,

     2.0f, 2.0f, 2.0f,

     2.0f,-2.0f, 2.0f,

     2.0f, 2.0f, 2.0f,

     2.0f, 2.0f,-2.0f,

     -2.0f, 2.0f,-2.0f,

     2.0f, 2.0f, 2.0f,

     -2.0f, 2.0f,-2.0f,

     -2.0f, 2.0f, 2.0f,

     2.0f, 2.0f, 2.0f,

     -2.0f, 2.0f, 2.0f,

     2.0f,-2.0f, 2.0f
  };

  static const GLfloat color_buffer_data [] = {
    0.583f,  0.771f,  0.014f,

     0.609f,  0.115f,  0.436f,

     0.327f,  0.483f,  0.844f,

     0.822f,  0.569f,  0.201f,

     0.435f,  0.602f,  0.223f,

     0.310f,  0.747f,  0.185f,

     0.597f,  0.770f,  0.761f,

     0.559f,  0.436f,  0.730f,

     0.359f,  0.583f,  0.152f,

     0.483f,  0.596f,  0.789f,

     0.559f,  0.861f,  0.639f,

     0.195f,  0.548f,  0.859f,

     0.014f,  0.184f,  0.576f,

     0.771f,  0.328f,  0.970f,

     0.406f,  0.615f,  0.116f,

     0.676f,  0.977f,  0.133f,

     0.971f,  0.572f,  0.833f,

     0.140f,  0.616f,  0.489f,

     0.997f,  0.513f,  0.064f,

     0.945f,  0.719f,  0.592f,

     0.543f,  0.021f,  0.978f,

     0.279f,  0.317f,  0.505f,

     0.167f,  0.620f,  0.077f,

     0.347f,  0.857f,  0.137f,

     0.055f,  0.953f,  0.042f,

     0.714f,  0.505f,  0.345f,

     0.783f,  0.290f,  0.734f,

     0.722f,  0.645f,  0.174f,

     0.302f,  0.455f,  0.848f,

     0.225f,  0.587f,  0.040f,

     0.517f,  0.713f,  0.338f,

     0.053f,  0.959f,  0.120f,

     0.393f,  0.621f,  0.362f,

     0.673f,  0.211f,  0.457f,

     0.820f,  0.883f,  0.371f,

     0.982f,  0.099f,  0.879f
    
  };

  
  // create3DObject creates and returns a handle to a VAO that can be used later
   cube= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCuboid ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.2f,-2.0f,-0.2f, // triangle 1 : begin

     -0.2f,-2.0f, -0.2f,

     -0.2f, 2.0f, 0.2f, // triangle 1 : end

     0.2f, 2.0f,-0.2f, // triangle 2 : begin

     -0.2f,-2.0f,-0.2f,

     -0.2f, 2.0f,-0.2f, // triangle 2 : end

     0.2f,-2.0f, 0.2f,

     -0.2f,-2.0f,-0.2f,

     0.2f,-2.0f,-0.2f,

     0.2f, 2.0f,-0.2f,

     0.2f,-2.0f,-0.2f,

     -0.2f,-2.0f,-0.2f,

     -0.2f,-2.0f,-0.2f,

     -0.2f, 2.0f, 0.2f,

     -0.2f, 2.0f,-0.2f,

     0.2f,-2.0f, 0.2f,

     -0.2f,-2.0f, 0.2f,

     -0.2f,-2.0f,-0.2f,

     -0.2f, 2.0f, 0.2f,

     -0.2f,-2.0f, 0.2f,

     0.2f,-2.0f, 0.2f,

     0.2f, 2.0f, 0.2f,

     0.2f,-2.0f,-0.2f,

     0.2f, 2.0f,-0.2f,

     0.2f,-2.0f,-0.2f,

     0.2f, 2.0f, 0.2f,

     0.2f,-2.0f, 0.2f,

     0.2f, 2.0f, 0.2f,

     0.2f, 2.0f,-0.2f,

     -0.2f, 2.0f,-0.2f,

     0.2f, 2.0f, 0.2f,

     -0.2f, 2.0f,-0.2f,

     -0.2f, 2.0f,0.2f,

     0.2f, 2.0f, 0.2f,

     -0.2f, 2.0f, 0.2f,

     0.2f,-2.0f, 0.2f
  };

  static const GLfloat color_buffer_data [] = {
    0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f,

     0.0f,0.0f,0.0f
    
  };
// create3DObject creates and returns a handle to a VAO that can be used later
   cuboid= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi1= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi2= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi3= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi4= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi5= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi6= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi7= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboi8= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboid1= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   cuboid2= create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void Triangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  if(j4>-0.6&&j4<-0.2&&j6>0.2&&j6<0.6)
  {
    j=0;
    j4=-1.8;
    j6=1.8;
    sound2.play();
    cout<<"lose";
  }
  else if((j4<-1.4&&j6>0.2&&j6<0.6)||(j4>0.8&&j4<1.2&&j6>0.2&&j6<0.6)||(j4>0.2&&j4<0.6&&j6>0.8&&j6<1.2)||(j4>-1.2&&j4<-0.8&&j6>-1&&j6<-0.6)||(j4>-1.2&&j4<-0.8&&j6>1.8&&j6<1.4)||(j4>-1.2&&j4<-0.8&&j6>1.6&&j6<2)||(j4<-1.4&&j6>-0.6&&j6<-0.2)||(j4==-1&&j6==-1)||(j4==0.6&&j6>-1.4)||(j4>-0.2&&j4<-0.2&&j6>-1.4)||(j4>0.2&&j4<0.6&&j6>1.6&&j6<2))
  {  
  j=0; 
    j4=-1.8;
    j6=1.8;
    sound2.play();
    cout<<"lose";
  }
  else if(((j4==v1&&j6==v2)||(j4==v3&&j6==v4)||(j4==v5&&j6==v6))&&(move%6==0||move%6==1))
  {  
  j=0; 
    j4=-1.8;
    j6=1.8;
    cout<<"lose";
    sound2.play();
    cout<<"lose";
  }
  else if(((j4==obstacl&&j6==obstacl1)||(j4==obstacl2&&j6==obstacl3))&&(move%6!=0||move%6!=1))
  {  
  j=0; 
    j4=-1.8;
    j6=1.8;
    sound2.play();
    cout<<"lose";
  }
  else if(j4>1.4&&j6<-1.4)
  {
    j=0;
    j4=-1.8;
    j6=1.8;
    cout<<"win";
  }
  else if(j4>1.8||j4<-1.8||j6>1.8||j6<-1.8)
  {
    j=0;
    j4=-1.8;
    j6=1.8;
    sound2.play();
    cout<<"lose";
  }
  //cout<<"j4"<<j4<<"j6"<<j6;
  
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  /*glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);
  glm::vec3 cameraPos = glm::vec3(3.0f, 3.0f, 3.0f);  
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); 
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
up = glm::vec3(0.0f, 1.0f, 0.0f); 
cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);*/
/*glm::mat4 view;
view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), 
         glm::vec3(0.0f, 0.0f, 0.0f), 
         glm::vec3(0.0f, 1.0f, 0.0f));
         GLfloat radius = 10.0f;*/

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
if(j7==1){

            i1=j4,i2=j5,i3=j6,i4=j4,i5=j5,i6=0,i7=0,i8=1,i9=0;
}
  Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */
  //glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);  

  /*glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);*/
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(j4, j5, j6));        // glTranslatef
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(cube);

  Triangle();

  translateRectangle = glm::translate (glm::vec3(j4, j5, j6));        // glTranslatef

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatecuboid = glm::translate (glm::vec3(-0.2, 0, 0.2));    
  //translateRectangle = glm::translate (glm::vec3(-2, 0, 0));     
  //glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatecuboid*rotateRectangle);  
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(cuboid);
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatecuboi = glm::translate (glm::vec3(-1.8, 0, 0.2));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(1.0, 0, 0.2));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi1);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(0.6, 0, 1));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi2);
  Matrices.model = glm::mat4(1.0f);
  //translatecuboi = glm::translate (glm::vec3(1.8, 0, 1));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi3);
  Matrices.model = glm::mat4(1.0f);
  //translatecuboi = glm::translate (glm::vec3(-1.4, 0, 1));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi4);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(-1, 0, 1.8));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi5);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(-1.8, 0, -0.6));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi6);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(-1, 0, -1));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi7);
  Matrices.model = glm::mat4(1.0f);
  //translatecuboi = glm::translate (glm::vec3(0.6, 0, -1.8));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboi8);
  Matrices.model = glm::mat4(1.0f);
  //translatecuboi = glm::translate (glm::vec3(-1, 0, -1.8));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboid1);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(0.6, 0, 1.8));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cuboid2);
  unsigned int microseconds=100000;
  int i;
  if(move%6==0||move%6==1)
  {
    
    while(obstacl==-0.2&&obstacl1==0.2||obstacl==-1.8&&obstacl1==0.2||obstacl==1.0&&obstacl1==0.2||obstacl==0.6&&obstacl1==1.0||obstacl==-1.0&&obstacl1==1.8||obstacl==-1.8&&obstacl1==-0.6||obstacl==-1.0&&obstacl1==-1||obstacl==0.6&&obstacl1==1.8){
obstacl=((rand()%10)*4)/10.0-1.8;
  obstacl1=((rand()%10)*4)/10.0-1.8;
    }
    
   while(obstacl2==-0.2&&obstacl3==0.2||obstacl2==-1.8&&obstacl3==0.2||obstacl2==1.0&&obstacl3==0.2||obstacl2==0.6&&obstacl3==1.0||obstacl2==-1.0&&obstacl3==1.8||obstacl2==-1.8&&obstacl3==-0.6||obstacl2==-1.0&&obstacl3==-1||obstacl2==0.6&&obstacl3==1.8){
obstacl2=((rand()%10)*4)/10.0-1.8;
  obstacl3=((rand()%10)*4)/10.0-1.8;
  }
    Matrices.model = glm::mat4(1.0f);
    srand(time(NULL));
  translatecuboi = glm::translate (glm::vec3(v1, 2.2, v2));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obstacle1);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(v3, 2.2, v4));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obstacle1);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(v5, 2.2, v6));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obstacle1);
  }
  else
  {
     while(v1==-0.2&&v2==0.2||v1==-1.8&&v2==0.2||v1==1.0&&v2==0.2||v1==0.6&&v2==1.0||v1==-1.0&&v2==1.8||v1==-1.8&&v2==-0.6||v1==-1.0&&v2==-1||v1==0.6&&v2==1.8){
v1=((rand()%10)*4)/10.0-1.8;
  v2=((rand()%10)*4)/10.0-1.8;
    }
  while(v3==-0.2&&v4==0.2||v3==-1.8&&v4==0.2||v3==1.0&&v4==0.2||v3==0.6&&v4==1.0||v3==-1.0&&v4==1.8||v3==-1.8&&v4==-0.6||v3==-1.0&&v4==-1||v3==0.6&&v4==1.8){
v3=((rand()%10)*4)/10.0-1.8;
  v4=((rand()%10)*4)/10.0-1.8;
  }
  
  while(v5==-0.2&&v6==0.2||v5==-1.8&&v6==0.2||v5==1.0&&v6==0.2||v5==0.6&&v6==1.0||v5==-1.0&&v6==1.8||v5==-1.8&&v6==-0.6||v5==-1.0&&v6==-1||v5==0.6&&v6==1.8){
v5=((rand()%10)*4)/10.0-1.8;
  v6=((rand()%10)*4)/10.0-1.8;
  }
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(obstacl, 2.2, obstacl1));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obstacle1);
  Matrices.model = glm::mat4(1.0f);
  translatecuboi = glm::translate (glm::vec3(obstacl2, 2.2, obstacl3));    
  Matrices.model *= (translatecuboi*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obstacle1);
  }
  for(i=0;i<j2&&j!=0;i++)
  {
    if(j==5)
    {
        sleep(0.1);
        //for(i=0;i<100;i++)
  //{
                j6=j6-0.1;
                j5=j5+0.1;
                Matrices.model = glm::mat4(1.0f);
                translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
   // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);

  
  //for(i=0;i<100;i++)
  //{
                j6=j6+0.1;
                j5=j5-0.1;
                Matrices.model = glm::mat4(1.0f);
  translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
  Matrices.model *= (translateRectangle*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
  //}
  }
    else if(j==1)
    {
      j4=j4+0.4;
translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }
    else if(j==2)
    {
      j4=j4-0.4;
translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }
    else if(j==3)
    {
      j6=j6-0.4;
translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }
    else if(j==4)
    {
      j6=j6+0.4;
translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }
    Triangle();
    Matrices.model = glm::mat4(1.0f);

  //translateRectangle = glm::translate (glm::vec3(-2, 0, 0));     
  //glm::mat4 translateRectangle = glm::translate (glm::vec3(j4, j5, j6));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
  
  }
  j=0;
  Matrices.model = glm::mat4(1.0f);
  //translateRectangle = glm::translate (glm::vec3(-2, 0, 0));     
  //glm::mat4 translateRectangle = glm::translate (glm::vec3(j4, j5, j6));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle*rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
  // Swap the frame buffers
  glutSwapBuffers ();

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)
    
    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (2.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	createRectangle ();
    createObst();
  createCube ();
  createCuboid ();

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

	initGL (width, height);

    if(!buffer1.loadFromFile("change.wav"))
        return -1;
    sound1.setBuffer(buffer1);

    if(!buffer2.loadFromFile("lose.wav"))
        return -1;
    sound2.setBuffer(buffer2);

    if(!buffer3.loadFromFile("pacman_beginning.wav"))
        return -1;
    sound3.setBuffer(buffer3);

    /*if(!buffer1.loadFromfile("boing_x.wav"))
        return -1
    sound1.setBuffer(buffer1);*/


    glutMainLoop ();

    return 0;
}
