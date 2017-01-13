#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
typedef struct basket{
    VAO *basketObject;
    float x,y; //bottom left vertex
    float width,height;
    int bricks;
    int color[3];
}basket;
basket basket1, basket2;

typedef struct gunS{
    VAO *gunObject;
    float x,y;
    float width, height;
    float rotationAngle;
}gunS;
gunS gun;

typedef struct scoreS{
    int score,level,nonBlackHit;
}scoreS;
scoreS score;

typedef struct laserS{
    float x,y;
    VAO * laserObject;
    static const float width=1.0, height=0.02;
    float rotationAngle;
    bool hit,forward;
    struct laserS* next;
}laserS;
laserS *laserFirst, *laserLast;

//VAO *basket1, *basket2,*gun;
void initialiseVariables(){
    basket1.x=-2.0f,basket1.y=-4.0f;
    basket2.x=2.0f,basket2.y=-4.0f;
    basket1.width=0.5,basket1.height=0.25;
    basket2.width=0.5, basket2.height=0.25;
    basket1.bricks=0,basket2.bricks=0;
    gun.x=-4.0f,gun.y=0.0f;
    gun.width=1.0, gun.height=0.2;
    gun.rotationAngle = 0;
    score.score=0;
    score.level=1;
    score.nonBlackHit=0;
    //laser.width=0.5, laser.height=0.2;
}
typedef struct brick{
    int color;
    VAO* brickObject;
    float x,y;
    bool hit;
    struct brick *next;
}brick;
brick *brickListLast=NULL,*brickListFirst=NULL;
int colorList[4][3]={
{1,0,0},
{0,1,0},
{0,0,1},
{0,0,0}
};

float timeBetweenBrickFall,manualTimeBrickFall;

GLuint programID;

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

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
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

void shoot();

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
    float brickMovement=0.5,gunMovement=0.5;
    if (action == GLFW_REPEAT || action==GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                (basket1.x)-=brickMovement;
                if ((basket1.x)<-3.0)
                    (basket1.x)=-3.0;
                break;
            case GLFW_KEY_V:
                (basket1.x)+=brickMovement;
                if ((basket1.x)+(basket1.width)>=(basket2.x))
                    (basket1.x)-=brickMovement;
                break;

            case GLFW_KEY_U:
                (basket2.x)-=brickMovement;
                if ((basket2.x)<=(basket1.x)+(basket1.width))
                    basket2.x+=brickMovement;
                break;
            case GLFW_KEY_I:
                basket2.x+=brickMovement;
                if ((basket2.x)+(basket2.width)>4.75)
                    (basket2.x)-=brickMovement;
                break;


            case GLFW_KEY_S:
                gun.y+=gunMovement;
                if(gun.y>3.5)
                    gun.y-=gunMovement;
                break;
            case GLFW_KEY_F:
                gun.y-=gunMovement;
                if(gun.y<-3.0)
                    gun.y+=gunMovement;
                break;

            case GLFW_KEY_A:
                (gun.rotationAngle)+=10;
                if((gun.rotationAngle)>90)
                    (gun.rotationAngle)-=10;
                break;
            case GLFW_KEY_D:
                gun.rotationAngle-=10;
                if(gun.rotationAngle<-80)
                    gun.rotationAngle+=10;
                break;

            case GLFW_KEY_N:
                manualTimeBrickFall+=0.1;
                if(manualTimeBrickFall>0.3)
                    manualTimeBrickFall-=0.1;
                break;

            case GLFW_KEY_M:
                manualTimeBrickFall-=0.1;
                if(manualTimeBrickFall<-0.3)
                    manualTimeBrickFall+=0.1;
                break;

            case GLFW_KEY_SPACE:
                shoot();
                break;

            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}


// Creates the triangle object used in this sample code
/*--void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  /*--static const GLfloat vertex_buffer_data [] = {
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
}--*/

// Creates the rectangle object used in this sample code
VAO* createRectangle (float x1, float y1,float x2, float y2,float x3, float y3,float x4, float y4,float rc,float gc, float bc)
{
  // GL3 accepts only Triangles. Quads are not supported
  GLfloat vertex_buffer_data [] = {
    x1,y1,0, // vertex 1
    x2,y2,0, // vertex 2
    x3, y3,0, // vertex 3

    x3, y3,0, // vertex 3
    x4, y4,0, // vertex 4
    x1,y1,0  // vertex 1
  };

  GLfloat color_buffer_data [] = {
    rc,gc,bc, // color 1
    rc,gc,bc, // color 2
    rc,gc,bc, // color 3

    rc,gc,bc, // color 3
    rc,gc,bc, // color 4
    rc,gc,bc  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
//--float rectangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClearColor(0.30f, 0.22f, 0.51f, 1.0f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model


  //DRAW BASKET1
  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */
  glm::mat4 translateBasket1 = glm::translate (glm::vec3(basket1.x,basket1.y, 0));
  Matrices.model *= translateBasket1;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(basket1.basketObject);


  //DRAW BASKET2
  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  //glPopMatrix();
  Matrices.model = glm::mat4(1.0f);

  //glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 translateBasket2 = glm::translate (glm::vec3(basket2.x,basket2.y, 0));
  Matrices.model *= translateBasket2;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(basket2.basketObject);

  //DRAW GUN
  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  //glPopMatrix();
  Matrices.model = glm::mat4(1.0f);

  //glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 translateGun = glm::translate (glm::vec3(gun.x,gun.y, 0));
  glm::mat4 rotateGun = glm::rotate((float)((gun.rotationAngle)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  Matrices.model *= translateGun*rotateGun;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(gun.gunObject);

  brick* node=brickListFirst;
  for(;node!=NULL;node=node->next)
  {
      //DRAW BRICK
      // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
      //glPopMatrix();
      if(node->hit==false)
      {
          Matrices.model = glm::mat4(1.0f);

      //glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
        glm::mat4 translateBrick = glm::translate (glm::vec3(node->x,node->y, 0));
        Matrices.model *= translateBrick;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

      // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(node->brickObject);
        }
    }

    //DRAW LASERS
    laserS *nodeL;
    for(nodeL=laserFirst;nodeL!=NULL;nodeL=nodeL->next)
    {
        if(nodeL->hit==false)
        {
            Matrices.model = glm::mat4(1.0f);

            //glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
            glm::mat4 translateLaserOtoGP = glm::translate (glm::vec3(gun.width,(gun.height)/2, 0));
            glm::mat4 translateLaserGPtoO = glm::translate (glm::vec3(-1*(gun.width),-1*((gun.height)/2), 0));
            glm::mat4 translateLaser = glm::translate (glm::vec3(nodeL->x,nodeL->y, 0));
            glm::mat4 rotateLaser = glm::rotate((float)((nodeL->rotationAngle)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
            Matrices.model *= translateLaser*translateLaserGPtoO*rotateLaser*translateLaserOtoGP;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

            // draw3DObject draws the VAO given to it using current MVP matrix
            draw3DObject(nodeL->laserObject);

        }
    }



  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  //--triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //--rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

void gamelost()
{
    glfwTerminate();
}

void shoot()
{
    laserS *newLaser=(laserS*)malloc(sizeof(laserS));
    newLaser->laserObject=createRectangle(0.0f,0.0f,0.0f,newLaser->height,newLaser->width,newLaser->height,newLaser->width,0.0f,0.0f,1.0f,0.0f);
    newLaser->rotationAngle=gun.rotationAngle;
    newLaser->x=gun.x+gun.width;
    newLaser->y=gun.y+(gun.height)/2;
    newLaser->forward=true;
    newLaser->next=NULL;

    if(laserFirst==NULL)
        laserFirst=newLaser;
    if(laserLast==NULL)
        laserLast=newLaser;
    else
    {
        laserLast->next=newLaser;
        laserLast=newLaser;
    }
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//basket1=createRectangle(-3.0,-4.0,-3.0,-5.0,-5.0,-5.0,-5.0,-4.0);
    //basket2=createRectangle(-2.0,-4.5,-2.0,-5.0,-1.0f,-5.0,-1.0,-4.5);
    basket1.basketObject=createRectangle(0.0f,0.0f,0.0f,basket1.height,basket1.width,basket1.height,basket1.width,0.0f,0.0f,1.0f,0.0f);
    basket2.basketObject=createRectangle(0.0f,0.0f,0.0f,basket2.height,basket2.width,basket2.height,basket2.width,0.0f,0.0f,0.0f,1.0f);
    gun.gunObject=createRectangle(0.0f,0.0f,0.0f,gun.height,gun.width,gun.height,gun.width,0.0f,0.0f,0.0f,1.0f);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void levelup()
{
    timeBetweenBrickFall+=0.5;
    score.level++;
}

void changeScore(int change)
{
    score.score+=5;
    if (score.score>50)
        levelup();
}

void checkCollisionBasketBrick(brick * fallenBrick)
{
    float brickleft=(fallenBrick->x),brickright=(fallenBrick->x)+0.25;
    //collision with basket1
    if(brickleft>=(basket1.x) && brickright<=(basket1.x)+(basket1.width))
    {
        fallenBrick->hit=true;
        if(fallenBrick->color==3)
        {
            basket1.bricks++;
            if(basket1.bricks>=3)
                gamelost();
        }
        else
        {
            changeScore(5);
            printf("%d\n",score.score);
        }
    }

    //collision with basket2
    if(brickleft>=(basket2.x) && brickright<=(basket2.x)+(basket2.width))
    {
        fallenBrick->hit=true;
        if(fallenBrick->color==3)
        {
            basket2.bricks++;
            if(basket2.bricks>=3)
                gamelost();
        }
        else
        {
            changeScore(5);
            printf("%d\n",score.score );
        }
    }
}

int main (int argc, char** argv)
{
	int width = 900;
	int height = 900;
    initialiseVariables();

    GLFWwindow* window = initGLFW(width, height);

	initGL(window, width, height);

    double last_update_time = glfwGetTime(), current_time,last_update_time_laser=last_update_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        timeBetweenBrickFall=1.0;
        manualTimeBrickFall=0.0;
        if ((current_time - last_update_time) >= timeBetweenBrickFall+manualTimeBrickFall) { // atleast 0.5s elapsed since last frame
            //make new brick
            brick *newBrick=(brick *)malloc(sizeof(brick));
            newBrick->color=rand()%4;
            newBrick->x=(rand()%80)/10-2;
            newBrick->y=4.25;
            newBrick->brickObject=createRectangle(0.0,0.0,0.0,-0.5,0.25,-0.5,0.25,0.0,colorList[newBrick->color][0],colorList[newBrick->color][1],colorList[newBrick->color][2]);
            newBrick->hit=false;
            newBrick->next=NULL;
            if(brickListLast!=NULL)
                brickListLast->next=newBrick;
            brickListLast=newBrick;
            if(brickListFirst==NULL)
                brickListFirst=newBrick;

            //all bricks fall down
            brick* node;
            for(node=brickListFirst;node!=NULL;node=node->next)
            {
                node->y=(node->y)-0.5;
                if((node->y)<=basket1.y+basket1.height)
                {
                    checkCollisionBasketBrick(node);
                    brickListFirst=node->next;
                }
            }
            last_update_time = current_time;
        }
        if(current_time-last_update_time_laser>=0.1)
        {
            laserS* nodeL,*PnodeL=laserFirst;
            for(nodeL=laserFirst;nodeL!=NULL;nodeL=nodeL->next,PnodeL=nodeL)
            {
                if(nodeL->forward)
                {
                    nodeL->x+=0.1*cos(nodeL->rotationAngle*M_PI/180);
                    nodeL->y+=0.1*sin(nodeL->rotationAngle*M_PI/180);
                    if((nodeL->x)+(nodeL->width)>4.0 || (nodeL->x)+(nodeL->width)<-4.0)
                        if(nodeL!=laserFirst)
                        {
                            PnodeL->next=nodeL->next;
                            nodeL=PnodeL;
                        }
                        else
                        {
                            
                        }

                }
            }
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
