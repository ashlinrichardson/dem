/* this program is a good candidate for running on a hex grid */

// need to make a diagram of where the streams collect and separate, collect and separate..

// a stream can separate into "arbitarily many" directions
// arbitrarily many streams can join at one point

#define STR_MAX 1000
#include "newzpr.h"
#include "pthread.h"
#include "time.h"
#include "vec3d.h"
#include "misc.h"
#define MYFONT GLUT_BITMAP_HELVETICA_12
#include <stdio.h>
#include <stdlib.h>
#include<cfloat>

size_t nrow, ncol, nband;
float * dat; // dem data: nrow * ncol linear array of floats

float * label;
size_t next_label;
float * label_r;
float * label_g;
float * label_b;

vector<size_t> mins;

float zmax, zmin; // dem extrema
size_t kmax, kmin;

vec3d * points; // 3d points for visualization
size_t * down;
size_t * up;
vec3d rX;

size_t flood_step; // flood step variable
set<size_t> dry_list; // list of dry neighbours
set<size_t> wet_list; // list of wet neighbours

inline vector<size_t> nbhd(size_t i, size_t j){
  // printf("nbhd(%d, %d)\n", (int)i, (int)j);
  vector<size_t> r;
  long int m, n, di, dj;
  for(m = -1; m <= 1; m++){
    di = i + m;
    for(n = -1; n <= 1; n++){
      dj = j + n;
      if(di >= 0 && dj >= 0 && di < nrow && dj < ncol && (di != i || dj !=j)){
        size_t k = di * ncol + dj;
        r.push_back(k);
      }
    }
  }
  // cout << "nbhd(" << i *ncol + j << ") " << r << endl;
  return r;
}

GLint selected;
int special_key; // 114

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
  if(myPickNames.size() > 0){
    cout << "PickSet:";
    std::set<GLint>::iterator it;
    for(it=myPickNames.begin(); it!=myPickNames.end(); it++){
      cout << *it << "," ;
    }
    cout << endl;
    fflush(stdout);
  }
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

void color_wheel(){
  float r, g, b;
  size_t np = nrow * ncol;
  if(!label_r){
    label_r = falloc(np);
    label_g = falloc(np);
    label_b = falloc(np);
  }

  size_t i;
  size_t ci = 0;
  float label_max = 0.;
  for0(i, nrow * ncol){
    if(label[i] > label_max){
      label_max = label[i];
    }
  }
  map<float, float> l_r;
  map<float, float> l_g;
  map<float, float> l_b;

  l_r[0.] = 0.;
  l_g[0.] = 0.;
  l_b[0.] = 0.;

  vector<size_t> points;
  vector<size_t>::iterator it;
  for(i = 1; i <= (size_t) label_max; i++){
    points.push_back(i);
  }
  std::srand ( unsigned ( std::time(0) ) );
  std::random_shuffle(points.begin(), points.end());
  for(it = points.begin(); it != points.end(); it++){
    i = *it;
    float h, s, v;
    s = v = 1;
    h = 360. * (float)ci / (float)(label_max - 1);
    hsv_to_rgb(&r, &g, &b, h, s, v);
    l_r[i] = r;
    l_g[i] = g;
    l_b[i] = b;
    ci += 1;
  }

  for0(i, np){
    float d = label[i];
    if(d == 0.){
      label_r[i] = 0.;
      label_g[i] = 0.;
      label_b[i] = 0.;
    }
    else{
      label_r[i] = l_r[d];
      label_g[i] = l_g[d];
      label_b[i] = l_b[d];
      // printf("r %f g %f b %f\n", label_r[i], label_g[i], label_b[i]);
    }
  }
}

/* Callback function for drawing */
void display(void){
  size_t i, j, k;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /*
  vec3d X;
  X.x = X.y = X.z = 0.;
  X.axis(.01);

  X.x = X.y = 1.;
  X.axis(.01);

  X.x = 1; X.y = 0;
  X.axis(.01);

  X.x = 0; X.y = 1;
  X.axis(.01);
  */
  glPushMatrix();
  glTranslatef(-rX.x, -rX.y, -rX.z);

  // axes
  vec3d X;
  X.x = X.y = X.z = 0.;
  X.axis(.01);

  X.x = X.y = 1.;
  X.axis(.01);

  X.x = 1; X.y = 0;
  X.axis(.01);

  X.x = 0; X.y = 1;
  X.axis(.01);

  rX.axis(.1);

  //lowpoint
  points[kmin].axis(.2);

  bool path_mode = false;

  /*
  vector<size_t>::iterator vi;
  for(vi = mins.begin(); vi != mins.end(); vi++){
    size_t k = *vi;
    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(points[k].x, points[k].y, points[k].z);
    glutSolidSphere(0.01, 20, 20);
    glPopMatrix();
  }
  */

  // iterate over dataset
  GLint picked = -1;

  for0(i, nrow){
    for0(j, ncol){

      k = i * ncol + j;
      glPushName(k);

      if(path_mode) glColor3f(0, .5, 0);
      if(myPickNames.count(k)){
        //glColor3f(1, 0, 1);
        picked = k;
        selected = k;
        flood_step = 0; // right arrow should increase
        dry_list.clear();
        wet_list.clear();

        if(special_key == 114){
          rX = points[k];
          special_key = -1;
        }
      }
      if(selected == k && path_mode){
        glColor3f(1, 0, 1);
      }

      // turn this off later?
      if(!path_mode){
        if(label[k] != label[selected]){
          //picked])
          //selected])

          glColor3f(label_r[k], label_g[k], label_b[k]);
        }
        else{
          // label[j] == label[selected]

          glColor3f(1, 1, 1);
          /*
          if(selected != k){
            // picked
            glColor3f(1. - label_r[k], 1. - label_g[k], 1. - label_b[k]);
          }
          else{
            glColor3f(label_r[k], label_g[k], label_b[k]);
          }
          */
        }
      }

      glBegin(GL_LINES);
      points[k].vertex();
      points[down[k]].vertex();
      glEnd();
      glPopName();
    }
  }

  if(path_mode){
    // up lines
    printf("up\n");
    for0(i, nrow){
      for0(j, ncol){

        k = i * ncol + j;
        glPushName(k);

        glColor3f(.5, 0, .5);
        if(myPickNames.count(k)){
          picked = k;
          selected = k;
          flood_step = 0; // right arrow should increase
          dry_list.clear();
          wet_list.clear();

          if(special_key == 114){
            rX = points[k];
            special_key = -1;
          }
        }
        if(selected == k){
          glColor3f(0, 1, 0);
        }

        glBegin(GL_LINES);
        points[k].vertex();
        points[up[k]].vertex();
        glEnd();

        glPopName();
      }
    }

  }
  // draw the path down for the thing that gets picked!
  if(selected > 0 && path_mode){
    GLint last = -1;
    GLint pos = selected; //picked;
    while(last != pos){
      last = pos;
      pos = down[pos];

      // printf(" last %d pos %d\n", (int)last, (int)pos);
      cout << "\t" << (points[pos]) << endl;

      glColor3f(0, 0, 1);
      glLineWidth(5);
      glBegin(GL_LINES);
      points[pos].vertex();
      points[last].vertex();
      glEnd();
      glLineWidth(1);
    }
    size_t i, j;
    i = last / ncol;
    j = last % ncol;
    printf(" z %f\n", dat[last]);

    if(flood_step == 0){
      wet_list.insert(last);
      vector<size_t> n(nbhd(i, j));
      for(vector<size_t>::iterator it = n.begin(); it != n.end(); it++){
        dry_list.insert(*it);
      }
    }

    set<size_t>::iterator t;
    for(t = dry_list.begin(); t != dry_list.end(); t++){
      glColor3f(1, 0, 0);
      glPointSize(8);
      glBegin(GL_POINTS);
      points[*t].vertex();
      glEnd();
    }
    for(t = wet_list.begin(); t != wet_list.end(); t++){
      glColor3f(0, 0, 1);
      glPointSize(10);
      glBegin(GL_POINTS);
      points[*t].vertex();
      glEnd();
    }
  }

  vector<size_t>::iterator vi;
  for(vi = mins.begin(); vi != mins.end(); vi++){
    float size = 0.01;
    size_t k = *vi;
    if(label[k] == label[picked]){
      glColor3f(1, 0, 0.);
      size *= 3;
    }
    else{
      glColor3f(1, 1, 1);
    }

    glPushMatrix();
    glTranslatef(points[k].x, points[k].y, points[k].z);

    glutSolidSphere(size, 20, 20);
    glPopMatrix();
  }

  drawText();
  glPopMatrix();
  glutSwapBuffers();
  renderflag = false;
}

/* Callback function for pick-event handling from ZPR */

void quitme(){
  exit(0);
}

vector<float> lookup(set<size_t> & x){
  set<size_t>::iterator t;
  vector<float> ret;
  for(t = x.begin(); t != x.end(); t++){
    ret.push_back(dat[*t]);
  }
  return ret;
}

void special(int key, int x, int y){
  printf("special %d\n", key);
  special_key = key;

  if(key == 102){
    flood_step += 1;

    printf("------------------------------------\n");
    cout << "dry: " << dry_list << endl;
    cout << lookup(dry_list) << endl;
    cout << "wet: " << wet_list << endl;
    cout << lookup(wet_list) << endl;
    printf("flood step: %d\n", (int)flood_step);

    float minz = FLT_MAX;
    set<size_t>::iterator t;
    priority_queue<f_idx> pq;

    // how high is the water?
    float maxz = FLT_MIN;
    for(t = wet_list.begin(); t != wet_list.end(); t++){
      if(dat[*t] > maxz){
        maxz = dat[*t];
      }
    }

    // raise the waterline a step (get the lowest neighbour that's not yet wet):
    // put the dry list in a queue:
    for(t = dry_list.begin(); t != dry_list.end(); t++){
      pq.push(f_idx(dat[*t], *t));
    }

    // pop off the elements that are wet already
    if(pq.top().d <= maxz){
      while(pq.top().d <= maxz){
        dry_list.erase(pq.top().idx);
        wet_list.insert(pq.top().idx);
        cout << "makewet z(" << pq.top().idx << ")=" << pq.top().d << endl;
        pq.pop();
      }
    }
    else{
      cout << "no already-wet elements to pop off" << endl;
    }

    // what if it's empty? can't flood but can add more neighbours
    // pop off the neighbours at lowest non-wet level, make them wet
    minz = pq.top().d;
    if(pq.top().d == minz){
      while(pq.top().d == minz){
        dry_list.erase(pq.top().idx);
        wet_list.insert(pq.top().idx);
        cout << "makewet2 z(" << pq.top().idx << ")=" << pq.top().d << endl;
        pq.pop();
      }
    }
    else{
      cout << "no minimal elements to pop off" << endl;
    }

    // anything below the waterline should no longer be dry

    // raise the water level

    printf(" zmin %f\n", minz);
    for(t = dry_list.begin(); t != dry_list.end(); t++){

    }

    // raise the water level

    // 1) get the smallest (z) dry element(s)

    // 2) raise the water level

    // 3) add new dry elements

    cout << "dry: " << dry_list << endl;
    cout << lookup(dry_list) << endl;
    cout << "wet: " << wet_list << endl;
    cout << lookup(wet_list) << endl;
    display();
  }
}

/* Keyboard functions */
void keyboard(unsigned char key, int x, int y){

  switch(key){
    /*case GLUT_SHIFT:
    shift_down = ! shift_down;
    cout << "shift_down " << shift_down << endl;
    break;*/

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

float climb(size_t i, size_t j, size_t nrow, size_t ncol, float * dat, float * label, size_t & next_label){
  size_t ni, nj, n;
  vector<size_t> nb;
  vector<size_t>::iterator it;

  // base case: this point already labelled
  n = i * ncol + j;
  if(label[n] > 0.) return label[n];

  // this point is not yet labelled

  size_t i_min = i;
  size_t j_min = j;
  float d = dat[n]; // i * ncol + j];
  float d_min = d;

  nb = nbhd(i, j);

  // find lowest-altitude neighbour, this point included
  for(it = nb.begin(); it != nb.end(); it ++){
    n = *it;
    ni = n / ncol;
    nj = n % ncol;

    float d = dat[n] ; //ni * ncol + nj];
    if(d < d_min){
      d_min = d;
      i_min = ni;
      j_min = nj;
    }
  }

  if(d == d_min){
    // case 1: this point minimal
    size_t lab = (float)next_label;
    label[i * ncol + j] = lab;
    next_label++;

    mins.push_back(i*ncol + j);
    printf("new label: %ld @ %ld\n", (long int)lab, i*ncol + j);
    return lab;
  }
  else{
    // case 2: recurse to lower neighbour
    return climb(i_min, j_min, nrow, ncol, dat, label, next_label);
  }

  // size_t k = di * ncol + dj;
  //exit(0);
}

int main(int argc, char ** argv){
  label_r = label_g = label_b = NULL;

  size_t i, j;
  rX.x = rX.y = rX.z = 0.;

  str fn("stack.bin");
  str hfn("stack.hdr");

  FILE * f = fopen(fn.c_str(), "wb");
  if(!f){
    err("expected input file: stack.bin");
  }
  fclose(f);

  hread(hfn, nrow, ncol, nband);
  dat = bread(fn, nrow, ncol, nband);
  size_t np = nrow * ncol;

  zmax = 0.;
  kmax = kmin = 0;
  zmin = (float)FLT_MAX;

  next_label = 1; // convention: 0 is unlabelled
  label = falloc(np);
  for0(i, np) label[i] = 0.;

  // should plot the neighbourhoods from the other direction (climb on -z, too!)

  for0(i, nrow){
    for0(j, ncol){
      size_t n = i * ncol + j;
      label[n] = climb(i, j, nrow, ncol, dat, label, next_label);
    }
  }
  cout << "mins" << mins << endl;
  color_wheel();

  points = new vec3d[nrow * ncol];
  for0(i, nrow){
    for0(j, ncol){
      size_t k = (i * ncol) + j;
      points[k].x = ((float)i) / ((float)nrow); //- .5;
      points[k].y = ((float)j) / ((float)ncol); //- .5;
      points[k].z = dat[k]; // / 300. ;
      if(dat[k] != 0.){
        // cout << points[k].x << " " << points[k].y << " " << points[k].z << endl;
      }
      if(points[k].z > zmax){
        zmax = points[k].z;
        kmax = k;
      }
      if(points[k].z < zmin){
        zmin = points[k].z;
        kmin = k;
      }
    }
  }

  for0(i, nrow){
    for0(j, ncol){
      size_t k = i *ncol + j;
      points[k].z -= zmin;
      points[k].z /= (zmax - zmin);
      points[k].z *= 0.6;
    }
  }

  down = (size_t *) (void *) alloc(nrow * ncol * sizeof(size_t));
  long int di, dj, m, n, u,v;
  for0(i, nrow){
    for0(j, ncol){
      u = i * ncol + j;
      down[u] = u;
      /*
      for0(m, 4)
      di = i;
      dj = j;
      if(m == 0) di = i - 1;
      else if (m == 1) di = i + 1;
      else if (m ==2) dj = j - 1;
      else dj = j + 1;
      */
      for(m = -1; m <= 1; m++){
        di = i + m;
        for(n = -1; n <= 1; n++){
          dj = j + n;
          if(di >= 0 && dj >= 0 && di < nrow && dj < ncol && (di != i || dj !=j)){
            // printf("i %d j %d di %d dj %d\n", i, j, di, dj);
            v = (di * ncol) + dj;
            if(dat[v] < dat[u]){
              down[u] = v;
            }
          }
        }
      }
    }
  }

  up = (size_t *) (void *) alloc(nrow * ncol * sizeof(size_t));
  for0(i, nrow){
    for0(j, ncol){
      u = i * ncol + j;
      up[u] = u;
      for(m = -1; m <= 1; m++){
        di = i + m;
        for(n = -1; n <= 1; n++){
          dj = j + n;
          if(di >= 0 && dj >= 0 && di < nrow && dj < ncol && (di != i || dj !=j)){
            // printf("i %d j %d di %d dj %d\n", i, j, di, dj);
            v = (di * ncol) + dj;
            if(dat[v] > dat[u]){
              up[u] = v;
            }
          }
        }
      }
    }
  }

  pick = _pick;

  special_key = selected = -1;
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
  glutSpecialFunc(special);
  // glutKeyboardUpFunc(keyboardup);

  glutIdleFunc(idle);

  glScalef(0.25,0.25,0.25);

  /* Configure ZPR module */
  // zprInit();
  zprSelectionFunc(display); //drawAxes); /* Selection mode draw function */
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