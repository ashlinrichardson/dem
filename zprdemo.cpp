#define STR_MAX 1000
#include "newzpr.h"
#include "pthread.h"
#include "time.h"
#include "vec3d.h"
#include "misc.h"
#define MYFONT GLUT_BITMAP_HELVETICA_12
#include <stdio.h>
#include <stdlib.h>

size_t nrow, ncol, nband;
float * dat;
float zmax;
vec3d * points;

/* Draw axes */
#define STARTX 500
#define STARTY 500
int fullscreen;
clock_t start_time;
clock_t stop_time;
#define SECONDS_PAUSE 0.4
char console_string[STR_MAX];
int console_position;
int renderflag;

void _pick(GLint name){
  cout << "PickSet:";
  std::set<GLint>::iterator it;
  for(it=myPickNames.begin(); it!=myPickNames.end(); it++){
    cout << *it << "," ;
  }
  cout << endl;
  fflush(stdout);
}

void renderBitmapString(float x, float y, void *font, char *string){
  char *c;
  glRasterPos2f(x,y);
  for (c=string; *c != '\0'; c++){
    glutBitmapCharacter(font, *c);
  }
}

//http://www.codeproject.com/Articles/80923/The-OpenGL-and-GLUT-A-Powerful-Graphics-Library-an
void setOrthographicProjection(){
  int h = WINDOWY;
  int w = WINDOWX;
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glScalef(1, -1, 1);
  glTranslatef(0, -h, 0);
  glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection(){
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void drawText(){
  glColor3f(0.0f,1.0f,0.0f);
  setOrthographicProjection();
  glPushMatrix();
  glLoadIdentity();
  int lightingState = glIsEnabled(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  renderBitmapString(3,WINDOWY-3,(void *)MYFONT,console_string);
  if(lightingState) glEnable(GL_LIGHTING);
  glPopMatrix();
  resetPerspectiveProjection();
}

float a1, a2, a3;

class point{
  public:
  point(){
  }
};

void drawAxes(void){
  /* Name-stack manipulation for the purpose of
  selection hit processing when mouse button
  is pressed. Names are ignored in normal
  OpenGL rendering mode. */

  glColor3f(0, 0, 1);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, 1);
  glEnd();

  glColor3f(0, 1, 0);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 1, 0);
  glEnd();

  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(1, 0, 0);
  glEnd();


  /*
  glPushMatrix();
  glPushName(0);
  glTranslatef( a1, a2, a3);
  glColor3f(1,0,0);

  if(myPickNames.count(0)){
    glColor3f(0,1,1);
  }
  glutSolidSphere(0.5, 8,8 );
  glPopName();
  glPopMatrix();

  glPushMatrix();
  glPushName(4);
  glColor3f(1,0,0);

  if(myPickNames.count(4)){
    glColor3f( 0,1,1);
  }

  glutSolidSphere(0.7, 20, 20);
  glPopName();
  glPopMatrix();

  glPushMatrix();
  glPushName(1);
  glColor3f(0,0,1);

  if(myPickNames.count(1)){
    glColor3f(1,1,0);
  }

  glRotatef(90.*a1,0,1,0);
  glutSolidCone(0.6, 4.0, 20, 20);
  glPopName();
  glPopMatrix();

  glPushMatrix ();
  glPushName(2);

  glColor3f(0,0,1);

  if(myPickNames.count(2)){
    glColor3f(1,1,0);
  }

  glRotatef(-90.*a1,1,0,0);
  glutSolidCone(0.6, 4.0, 20, 20);
  glPopName();
  glPopMatrix();

  glPushMatrix();
  glColor3f(0,0,1);

  if(myPickNames.count(3)){
    glColor3f(1,1,0);
  }
  glPushName(3);
  glutSolidCone(0.6, 4.0, 20, 20);
  glPopName();
  glPopMatrix();
  */

}

/* Callback function for drawing */
void display(void){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawAxes();
  drawText();
  size_t i, j, k;
  glPointSize(2.);
  glColor3f(0, 0, 1.);
  for0(i, nrow){
    for0(j, ncol){
      k = i * ncol + j;
      glColor3f(0, (points[k].z / zmax) , 1. - (points[k].z / zmax));
      glPushName(k);
      glBegin(GL_POINTS);
      points[k].vertex();
      glEnd();
      glPopName();
    }
  }
  glutSwapBuffers();
  renderflag = false;
}

/* Callback function for pick-event handling from ZPR */

void quitme(){
  exit(0);
}

/* Keyboard functions */
void keyboard(unsigned char key, int x, int y){
  switch(key){
    // Backspace

    /*case GLUT_F1:
    //if( stop_time > clock())
    // break;
    if(!fullscreen){
      fullscreen=1;
      glutFullScreen();
    }
    else{
      fullscreen=0;
      glutReshapeWindow(STARTX, STARTY);
      glutPositionWindow(0, 0);
    }
    glutPostRedisplay();
    //display();
    //start_time = clock();
    //stop_time = start_time + CLOCKS_PER_SEC;
    //while(clock() < stop_time){
    //}

    break;
    */

    case 8 :
    case 127:
    if(console_position>0){
      console_position --;
      console_string[console_position]='\0';
      printf("STRING: %s\n", &console_string[0]);
      //printf( "%d Pressed Backspace\n",(char)key);
      display();
    }
    break;

    // Enter
    case 13 :
    //printf( "%d Pressed RETURN\n",(char)key);
    console_string[0]='\0';
    console_position=0;
    display();
    break;

    // Escape
    case 27 :
    quitme();
    exit(0);
    //printf( "%d Pressed Esc\n",(char)key);
    break;

    // Delete
    /* case 127 :
    printf( "%d Pressed Del\n",(char)key);
    break;
    */
    default:
    //printf( "Pressed key %c AKA %d at position %d % d\n",(char)key, key, x, y);
    console_string[console_position++] = (char)key;
    console_string[console_position]='\0';
    printf("STRING: %s\n", &console_string[0]);
    display();
    break;
  }
}

static GLfloat light_ambient[] = {
0.0, 0.0, 0.0, 1.0 };
static GLfloat light_diffuse[] = {
1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = {
1.0, 1.0, 1.0, 1.0 };
static GLfloat light_position[] = {
1.0, 1.0, 1.0, 0.0 };

static GLfloat mat_ambient[] = {
0.7, 0.7, 0.7, 1.0 };
static GLfloat mat_diffuse[] = {
0.8, 0.8, 0.8, 1.0 };
static GLfloat mat_specular[] = {
1.0, 1.0, 1.0, 1.0 };
static GLfloat high_shininess[] = {
100.0 };

// https://computing.llnl.gov/tutorials/pthreads/

void idle(){
  if( renderflag ){
    glFlush();
    glutPostRedisplay();
  }
}

int main(int argc, char ** argv){

  str fn("out.bin");
  str hfn("out.hdr");
  hread(hfn, nrow, ncol, nband);
  dat = bread(fn, nrow, ncol, nband);
  zmax = 0.;

  points = new vec3d[nrow * ncol];
  size_t i, j;
  for0(i, nrow) for0(j, ncol){
    size_t k = (i * ncol) + j;
    points[k].x = ((float)i) / ((float)nrow) - .5;
    points[k].y = ((float)j) / ((float)ncol) - .5;
    points[k].z = dat[k] / 10000.; // / 300. ;
    if(dat[k] != 0.) cout << points[k].x << " " << points[k].y << " " << points[k].z << endl;
    if(points[k].z > zmax) zmax = points[k].z;
  }

  pick = _pick;

  printf("main()\n");
  renderflag = false;
  a1=a2=a3=1;
  console_position = 0;
  //Py_Initialize();
  //printf("Py_init()\n");

  fullscreen=0;

  /* Initialise olLUT and create a window */

  printf("try glut init...\n");

  glutInit(&argc, argv);
  printf("glutInit()\n");

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(STARTX,STARTY);
  glutCreateWindow("");
  zprInit();

  printf("glutCreateWindow()\n");

  /* Configure GLUT callback functions */

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  // glutKeyboardUpFunc(keyboardup);

  glutIdleFunc(idle);

  glScalef(0.25,0.25,0.25);

  /* Configure ZPR module */
  // zprInit();
  zprSelectionFunc(drawAxes); /* Selection mode draw function */
  zprPickFunc(pick); /* Pick event client callback */

  /* Initialise OpenGL */

  /*
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  */

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);

  // pthread_t thread;
  // pthread_create(&thread, NULL, &threadfun, NULL);
  /* Enter GLUT event loop */
  glutMainLoop();
  return 0;
}
