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
string filenamePath = "data/general/unit_sphere.xml";
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


void setupCamera();
void updateCamera();

/********* Scene Objects ***************/
Shape* shape = NULL;
/** these are the global variables used for rendering **/
struct SceneObj {
    Matrix transform; // for glMultMatrix
    ScenePrimitive *prim; // for drawing
};
vector<SceneObj> objs; // flattened scene from parser

void flattenScene(SceneNode *root, Matrix parent, vector<SceneObj> &objs);


void objectShape (int shapeType) {
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
		shape = cube;
	}
}

Vector generateRay(int pixelX, int pixelY) {
   
    float px = (2.0 * pixelX / (double)pixelWidth) - 1;
    float py = 1 - (2.0 * pixelY / (double)pixelHeight);
    
    Point pointV = Point(px, py, -1.0);
    
    pointV = (camera -> GetUvw2XyzMatrix()) * pointV;
    
    Vector rayV = pointV - (camera -> GetEyePoint());
    rayV.normalize();
    
    return rayV;
    
}

Point getEyePoint() {
	Point eye = camera->GetEyePoint();
	return eye;
}

Point getIsectPointWorldCoord(Point eye, Vector ray, double t) {
	Point p = eye + t * ray;
	return p;
}

void setPixel(GLubyte* buf, int x, int y, int r, int g, int b) {
	buf[(y*pixelWidth + x) * 3 + 0] = (GLubyte)r;
	buf[(y*pixelWidth + x) * 3 + 1] = (GLubyte)g;
	buf[(y*pixelWidth + x) * 3 + 2] = (GLubyte)b;
}

void illuminate(int obj, Vector N, Point isectPoint, int x, int y) {
	SceneGlobalData global;
	SceneLightData light;
	int m;
	double ka, kd, ks;
	double r, g, b;
	Vector L; Vector R; Vector V;

	parser -> getGlobalData(global);
	ka = global.ka;
	kd = global.kd;
	ks = global.ks;
	m = parser->getNumLights();
	float* O_a = objs[obj].prim->material.cAmbient.channels;
	float* O_d = objs[obj].prim->material.cDiffuse.channels;
	float* O_s = objs[obj].prim->material.cSpecular.channels;

	// cout << "ka: " << ka << "kd: " << kd << "ks: " << ks<< endl;

	r = ka * O_a[0];
	g = ka * O_a[1];
	b = ka * O_a[2];

 	for (int i=0; i<m; i++){
    	parser->getLightData(i, light);
    	float* light_rgb = light.color.channels;
    	// cout << "Light: " << light_rgb[0] << light_rgb[1] << light_rgb[2] << endl;
    	// cout << "O_D: "<< O_d[0] << O_d[1] << O_d[2] << endl;
    	// cout << "O_s: "<< O_s[0] << O_s[1] << O_s[2] << endl;

    	Vector p = (light.pos - isectPoint);
    	L = normalize(p);
    	R = L - 2*(dot(L, N)) * N;
    	V = camera->GetLookVector();


    	// cout << "R and V: " << dot(R,V) << " " << dot(N,L) << endl;

    	r += light_rgb[0]*kd*O_d[0]*dot(N,L) + ks*O_s[0]*dot(R,V);
    	g += light_rgb[1]*kd*O_d[1]*dot(N,L) + ks*O_s[1]*dot(R,V);
    	b += light_rgb[2]*kd*O_d[2]*dot(N,L) + ks*O_s[2]*dot(R,V);

    	setPixel(pixels, x, y, r*255, g*255, b*255);

    	cout << "R: " << r*255 << " G: " << g*255 << " B:" << b*255 << endl;
	}

	

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

	for (int i = 0; i < pixelWidth; i++) {
		for (int j = 0; j < pixelHeight; j++) {
			/* Pseudo:
			for primitives in the scene: 
				1) find out shape 
				2) get eyepoint, matrix, generate rays
				3) call intersect, get t
			 	4) check if t>0, then color w/ normal
			 	5) illuminate w/ r g b, setpixel color
			*/

			Vector rayV = generateRay(i, j);
			Point eyePointP = getEyePoint();
			Matrix m;

			double min_t = -1;
			int closest_obj = 0;

			for (int k=0; k<objs.size(); k++){
				ScenePrimitive *prim = objs[k].prim;
				objectShape(prim->type);

				double t = shape->Intersect(eyePointP, rayV, objs[k].transform);

				if (min_t == -1){
					min_t = t;
				} else if (t < min_t && t>0) {
					min_t = t;
					closest_obj = k;
					m = objs[k].transform;
				}
			}

			if (min_t > 0){
				// cout << "pixels: " << i << j << endl;
				// cout << "min_t: " << min_t << " obj: " << closest_obj << endl;

				Point isectPoint = getIsectPointWorldCoord(eyePointP, rayV, min_t);
				Vector normal = transpose(invert(m)) * shape->findIsectNormal(eyePointP, rayV, min_t);
				normal = normalize(normal);
				illuminate(closest_obj, normal, isectPoint, i, j);
			} 
			
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

	if (!objs.empty())
		objs.clear();

    flattenScene(parser->getRootNode(), Matrix(), objs);
}

/***************************************** Parsing scenenode  *****************/

// applyTransformation
//      - applies a scene transformation to the current matrix stack
inline Matrix applyTransformation(SceneTransformation *trans) {
    Vector v;
    switch (trans->type) {
        case TRANSFORMATION_TRANSLATE:
            return trans_mat(trans->translate);
        case TRANSFORMATION_SCALE:
            return scale_mat(trans->scale);
        case TRANSFORMATION_ROTATE:
            return rot_mat(trans->rotate, trans->angle);
        case TRANSFORMATION_MATRIX:
        	return trans->matrix;
    }
}


inline Matrix collapseTransformations(std::vector<SceneTransformation *> xfs) {
    Matrix M;
    for (auto xf : xfs) {
        M = M * applyTransformation(xf);
    }
    return M;
}

// flattenScene
//      - converts a parse tree to a 1d array of SceneObjs
void flattenScene(SceneNode *root, Matrix parent, vector<SceneObj> &objs) {
    Matrix transform; //current transform matrix

    // apply all transformations
    transform = parent * collapseTransformations(root->transformations);

    // push all primitives onto objs with transform
    size_t nprims = root->primitives.size();
    for (size_t i = 0; i < nprims; i++) {
        SceneObj obj = { transform, root->primitives[i] };
        objs.push_back(obj);
    }

    // recurse on children
    size_t nnodes = root->children.size();
    for (size_t i = 0; i < nnodes; i++)
        flattenScene(root->children[i], transform, objs);

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
	if (cameraData.isDir == true) {
		camera->Orient(cameraData.pos, cameraData.look, cameraData.up);
	}
	else {
		camera->Orient(cameraData.pos, cameraData.lookAt, cameraData.up);
	}
	camera->SetScreenSize(screenWidth, screenHeight);

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



