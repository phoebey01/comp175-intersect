#define NUM_OPENGL_LIGHTS 8

#include <iostream>
#include <fstream>
#include <string>
#include <GL/glui.h>
#include "Shape.h"
#include "Cube.h"
#include "Cylinder.h"
#include "Cone.h"
#include "Sphere.h"
#include "SceneParser.h"
#include "Camera.h"
#include "GraphicsObject.hpp"

using namespace std;

/** These are the live variables passed into GLUI ***/
int  isectOnly = 1;

int	 camRotU = 0;
int	 camRotV = 0;
int	 camRotW = 0;
int  viewAngle = 45;
float eyeX = 2;
float eyeY = 2;
float eyeZ = 2;
float lookX = -2;
float lookY = -2;
float lookZ = -2;

/** These are GLUI control panel objects ***/
int  main_window;
string filenamePath = "data/general/test.xml";
GLUI_EditText* filenameTextField = NULL;
GLubyte* pixels = NULL;
int pixelWidth = 0, pixelHeight = 0;
int screenWidth = 0, screenHeight = 0;

/** these are the global variables used for rendering **/
Cube* cube = new Cube();
Cylinder* cylinder = new Cylinder();
Cone* cone = new Cone();
Sphere* sphere = new Sphere();
SceneParser* parser = NULL;
Camera* camera = new Camera();
Shape* shape = NULL;
vector<GraphicsObject> objects;

void setupCamera();
void updateCamera();

void setPixel(GLubyte* buf, int x, int y, int r, int g, int b) {
	buf[(y*pixelWidth + x) * 3 + 0] = (GLubyte)r;
	buf[(y*pixelWidth + x) * 3 + 1] = (GLubyte)g;
	buf[(y*pixelWidth + x) * 3 + 2] = (GLubyte)b;
}

void setShape(int shapeType) {
    switch (shapeType) {
        case SHAPE_CUBE:
            shape = cube;
            break;
        case SHAPE_CYLINDER:
            shape = cylinder;
            break;
        case SHAPE_CONE:
            shape = cone;
            break;
        case SHAPE_SPHERE:
            shape = sphere;
            break;
        default:
            shape = sphere;
    }
}

/* Compute the ray in world coordinates */
Vector generateRay(int pixelX, int pixelY) {
    
    
    float px = (2.0 * pixelX / (double)pixelWidth) - 1;
    float py = 1 - (2.0 * pixelY / (double)pixelHeight);
    
    Point pointV = Point(px, py, -1.0);
    
    pointV = (camera -> getUvw2xyzMatrix()) * pointV;
    
    Vector rayV = pointV - (camera -> GetEyePoint());
    rayV.normalize();
    
    return rayV;
    
}

/* This function will use the illumination formula given in the spec:
 i_lambda = ka * Oa_lambda + (sum(from i=1 to i=m) of
 (l_i_lambda*(kd*Od_lambda*dot(N_hat, L_hat_i) + ks*Os_lambda*dot(R_hat_i, V_hat)^f))
 */
void illuminateSamples(GraphicsObject* closest, SceneGlobalData globals, Vector rayV, double t, int i, int j) {
        
    if (t != 1000000) {
        if (isectOnly) {
            setPixel(pixels, i, j, 255, 255, 255);
        }
        else {
            
            //Set the shape that we'll work with
            setShape((closest -> object()).type);
            
            //Get the normal vector in object space
            Point objEye = (closest -> invTrans()) * (camera -> GetEyePoint());
            Vector objRay = (closest -> invTrans()) * rayV;
            Vector objNorm = shape->findIsectNormal(objEye, objRay, t);
            
            //Calculating the normal in world space
            Vector N_hat = transpose(closest -> invTrans()) * objNorm;
            N_hat.normalize();
            
            
            /* ---------------------------------------------------------------------------
             Using the formula for the final intensity for wavelength (final RGB for the
             pixel we want)
             ---------------------------------------------------------------------------
             */
            
            SceneColor i_lambda = SceneColor();
            SceneLightData l_i_lambda;
            Vector L_hat_i;
            Vector R_hat_i;
            Vector V_hat = rayV;
            
             SceneColor kd;
             kd.r = (closest -> object()).material.cDiffuse.r * globals.kd;
             kd.g = (closest -> object()).material.cDiffuse.g * globals.kd;
             kd.b = (closest -> object()).material.cDiffuse.b * globals.kd;
             SceneColor ks;
             ks.r = (closest -> object()).material.cSpecular.r * globals.ks;
             ks.g = (closest -> object()).material.cSpecular.g * globals.ks;
             ks.b = (closest -> object()).material.cSpecular.r * globals.ks;
             SceneColor ka;
             ka.r = (closest -> object()).material.cAmbient.r * globals.ka;
             ka.g = (closest -> object()).material.cAmbient.g * globals.ka;
             ka.b = (closest -> object()).material.cAmbient.b * globals.ka;
             
             SceneColor Oa_lambda = (closest -> object()).material.cAmbient;
             SceneColor Od_lambda = (closest -> object()).material.cDiffuse;
             SceneColor Os_lambda = (closest -> object()).material.cSpecular;
             float f = (closest -> object()).material.shininess;
            
            //The summation bit of the formula
            for(int m = 0; m < parser -> getNumLights(); m++) {
                
                //Get the light data
                parser -> getLightData(m, l_i_lambda);
                
                //Get the point light vector (only dealing with point lights)
                L_hat_i = l_i_lambda.pos - ((camera -> GetEyePoint()) + rayV * t);
                L_hat_i.normalize();
                
                //First half of summation bit of the formula
                float dotProd = dot(L_hat_i, N_hat);
                if (dotProd < 0.0f)
                    dotProd = 0.0f;
                SceneColor contrib;
                contrib.r = (kd.r * Od_lambda.r * dotProd) * l_i_lambda.color.r;
                contrib.g = (kd.g * Od_lambda.g * dotProd) * l_i_lambda.color.g;
                contrib.b = (kd.b * Od_lambda.b * dotProd) * l_i_lambda.color.b;
                i_lambda.r = i_lambda.r + contrib.r;
                i_lambda.g = i_lambda.g + contrib.g;
                i_lambda.b = i_lambda.b + contrib.b;
                
                //Second half of summation bit of the formula
                R_hat_i = (2 * dotProd * N_hat) - L_hat_i;
                dotProd = dot(R_hat_i, rayV);
                if (dotProd < 0.0f)
                    dotProd = 0.0f;
                contrib.r = (ks.r * Os_lambda.r * pow(dotProd, f)) * l_i_lambda.color.r;
                contrib.g = (ks.g * Os_lambda.g * pow(dotProd, f)) * l_i_lambda.color.g;
                contrib.b = (ks.b * Os_lambda.b * pow(dotProd, f)) * l_i_lambda.color.b;
                i_lambda.r = i_lambda.r + contrib.r;
                i_lambda.g = i_lambda.g + contrib.g;
                i_lambda.b = i_lambda.b + contrib.b;
                
            }
            
            //Adding the final piece
            i_lambda.r = i_lambda.r + (ka.r * Oa_lambda.r);
            i_lambda.g = i_lambda.g + (ka.g * Oa_lambda.g);
            i_lambda.b = i_lambda.b + (ka.b * Oa_lambda.b);
            
            //Keeping the values within the correct range
            i_lambda.r = i_lambda.r * 255.0;
            i_lambda.g = i_lambda.g * 255.0;
            i_lambda.b = i_lambda.b * 255.0;
            if (i_lambda.r > 255)
                i_lambda.r = 255;
            if (i_lambda.g > 255)
                i_lambda.g = 255;
            if (i_lambda.b > 255)
                i_lambda.b = 255;
            
            //Set the pixel values
            setPixel(pixels, i, j, i_lambda.r, i_lambda.g, i_lambda.b);
        }
    }
    
}

void findAndIlluminateShape(GraphicsObject* closest, SceneGlobalData globals, Vector rayV, int i, int j) {
    
    double t = 1000000;
    
    // For each object, calculate t and find object with the smallest non-negative t value
    
    for (int i = 0; i < objects.size(); i++) {
        //if (objects[i].object() != NULL) {
            setShape((objects[i].object()).type);
            
            double tempT = shape -> Intersect(camera -> GetEyePoint(), rayV, objects[i].transform());
            if ((tempT >= 0.0) && (tempT < t)) {
                t = tempT;
                closest = &objects[i];
            }
        //}
    }
    
    illuminateSamples(closest, globals, rayV, t, i, j);
    
}

void addToObjectList(SceneNode *node, const Matrix &parentTransform) {
    Matrix xf = parentTransform * collapseTransformations(node -> transformations);
    
    for (auto p : node -> primitives) {
        objects.push_back(GraphicsObject(p, xf));
    }
    for (auto c : node -> children) {
        addToObjectList(c, xf);
    }
}

void buildObjectList(SceneNode *root, const Matrix &parentTransform) {
    objects.clear();
    addToObjectList(root, parentTransform);
}

void callback_start(int id) {
	cout << "start button clicked!" << endl;

	if (parser == NULL) {
		cout << "no scene loaded yet" << endl;
		return;
	}

	pixelWidth = screenWidth;
	pixelHeight = screenHeight;

	updateCamera();

	if (pixels != NULL) {
		delete pixels;
	}
	pixels = new GLubyte[pixelWidth  * pixelHeight * 3];
	memset(pixels, 0, pixelWidth  * pixelHeight * 3);

	cout << "(w, h): " << pixelWidth << ", " << pixelHeight << endl;
    
    GraphicsObject* closest;
    SceneNode* root = parser -> getRootNode();
    buildObjectList(root, camera -> GetModelViewMatrix());
    for (int i = 0; i < objects.size(); i++) {
        objects[i].setInv(invert(objects[i].transform()));
    }
    SceneGlobalData globals;
    parser -> getGlobalData(globals);
    
    // for each pixel
	for (int i = 0; i < pixelWidth; i++) {
		for (int j = 0; j < pixelHeight; j++) {
            
            setPixel(pixels, i, j, 0, 0, 0);
            
            Vector rayV = generateRay(i, j);
            
            findAndIlluminateShape(closest, globals, rayV, i, j);
            
		}
	}
	glutPostRedisplay();
}

void callback_load(int id) {
	char curDirName [2048];
	if (filenameTextField == NULL) {
		return;
	}
	printf ("%s\n", filenameTextField->get_text());

	if (parser != NULL) {
		delete parser;
	}
	parser = new SceneParser (filenamePath);
	cout << "success? " << parser->parse() << endl;

	setupCamera();
}


/***************************************** myGlutIdle() ***********/

void myGlutIdle(void)
{
	/* According to the GLUT specification, the current window is
	undefined during an idle callback.  So we need to explicitly change
	it if necessary */
	if (glutGetWindow() != main_window)
		glutSetWindow(main_window);

	glutPostRedisplay();
}


/**************************************** myGlutReshape() *************/

void myGlutReshape(int x, int y)
{
	float xy_aspect;

	xy_aspect = (float)x / (float)y;
	glViewport(0, 0, x, y);
	camera->SetScreenSize(x, y);

	screenWidth = x;
	screenHeight = y;

	glutPostRedisplay();
}


/***************************************** setupCamera() *****************/
void setupCamera()
{
	SceneCameraData cameraData;
	parser->getCameraData(cameraData);

	camera->Reset();
	camera->SetViewAngle(cameraData.heightAngle);
    Point pos = cameraData.pos;
    Vector look = cameraData.look;
    Point lookAt = cameraData.lookAt;
    Vector up = cameraData.up;
	if (cameraData.isDir == true) {
		camera->Orient(pos, look, up);
	}
	else {
		camera->Orient(pos, lookAt, up);
	}

	viewAngle = camera->GetViewAngle();
	Point eyeP = camera->GetEyePoint();
	Vector lookV = camera->GetLookVector();
	eyeX = eyeP[0];
	eyeY = eyeP[1];
	eyeZ = eyeP[2];
	lookX = lookV[0];
	lookY = lookV[1];
	lookZ = lookV[2];
	camRotU = 0;
	camRotV = 0;
	camRotW = 0;
	GLUI_Master.sync_live_all();
}

void updateCamera()
{
	camera->Reset();

	Point guiEye (eyeX, eyeY, eyeZ);
	Point guiLook(lookX, lookY, lookZ);
	Vector up = camera->GetUpVector();

	camera->SetViewAngle(viewAngle);
	camera->Orient(guiEye, guiLook, up);
	camera->RotateU(camRotU);
	camera->RotateV(camRotV);
	camera->RotateW(camRotW);
}

/***************************************** myGlutDisplay() *****************/

void myGlutDisplay(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (parser == NULL) {
		return;
	}

	if (pixels == NULL) {
		return;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(pixelWidth, pixelHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glutSwapBuffers();
}

void onExit()
{
	delete cube;
	delete cylinder;
	delete cone;
	delete sphere;
	delete camera;
	if (parser != NULL) {
		delete parser;
	}
	if (pixels != NULL) {
		delete pixels;
	}
}

/**************************************** main() ********************/

int main(int argc, char* argv[])
{
	atexit(onExit);

	/****************************************/
	/*   Initialize GLUT and create window  */
	/****************************************/

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);

	main_window = glutCreateWindow("COMP 175 Assignment 4");
	glutDisplayFunc(myGlutDisplay);
	glutReshapeFunc(myGlutReshape);

	/****************************************/
	/*         Here's the GLUI code         */
	/****************************************/

	GLUI* glui = GLUI_Master.create_glui("GLUI");

	filenameTextField = new GLUI_EditText( glui, "Filename:", filenamePath);
	filenameTextField->set_w(300);
	glui->add_button("Load", 0, callback_load);
	glui->add_button("Start!", 0, callback_start);
	glui->add_checkbox("Isect Only", &isectOnly);
	
	GLUI_Panel *camera_panel = glui->add_panel("Camera");
	(new GLUI_Spinner(camera_panel, "RotateV:", &camRotV))
		->set_int_limits(-179, 179);
	(new GLUI_Spinner(camera_panel, "RotateU:", &camRotU))
		->set_int_limits(-179, 179);
	(new GLUI_Spinner(camera_panel, "RotateW:", &camRotW))
		->set_int_limits(-179, 179);
	(new GLUI_Spinner(camera_panel, "Angle:", &viewAngle))
		->set_int_limits(1, 179);

	glui->add_column_to_panel(camera_panel, true);

	GLUI_Spinner* eyex_widget = glui->add_spinner_to_panel(camera_panel, "EyeX:", GLUI_SPINNER_FLOAT, &eyeX);
	eyex_widget->set_float_limits(-10, 10);
	GLUI_Spinner* eyey_widget = glui->add_spinner_to_panel(camera_panel, "EyeY:", GLUI_SPINNER_FLOAT, &eyeY);
	eyey_widget->set_float_limits(-10, 10);
	GLUI_Spinner* eyez_widget = glui->add_spinner_to_panel(camera_panel, "EyeZ:", GLUI_SPINNER_FLOAT, &eyeZ);
	eyez_widget->set_float_limits(-10, 10);

	GLUI_Spinner* lookx_widget = glui->add_spinner_to_panel(camera_panel, "LookX:", GLUI_SPINNER_FLOAT, &lookX);
	lookx_widget->set_float_limits(-10, 10);
	GLUI_Spinner* looky_widget = glui->add_spinner_to_panel(camera_panel, "LookY:", GLUI_SPINNER_FLOAT, &lookY);
	looky_widget->set_float_limits(-10, 10);
	GLUI_Spinner* lookz_widget = glui->add_spinner_to_panel(camera_panel, "LookZ:", GLUI_SPINNER_FLOAT, &lookZ);
	lookz_widget->set_float_limits(-10, 10);

	glui->add_button("Quit", 0, (GLUI_Update_CB)exit);

	glui->set_main_gfx_window(main_window);

	/* We register the idle callback with GLUI, *not* with GLUT */
	GLUI_Master.set_glutIdleFunc(myGlutIdle);

	glutMainLoop();

	return EXIT_SUCCESS;
}



