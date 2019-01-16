#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>
#include "image.h"

#define TIMER_ID 0
#define TIMER_INTERVAL 16


typedef struct strelica{
  float x, y, z;
  float vx, vy, vz;
  int bacena;
  int stiglaDoTable;
  float ugaoX, ugaoY;
}Strelica;

Strelica strelice[3];
int igraci[8];
int brojIgraca;
int poeniTokomPoteza;
int indikatorIgraca;

int trenutniIgrac;
int trenutnaStrelica;
int trenutniKrug;

int animParametar = 0;

float jacina = 0.1;

const float G = 0.00981;

int pocetnoX, pocetnoY, krajnjeX, krajnjeY, timerZaBacanje;

static void on_display(void);
static void on_reshape(int width, int height);
static void on_keyboard(unsigned char key, int x, int y);
static void on_mouse(int button, int state, int x, int y);
static void on_timer(int value);
static int animation_ongoing;

static GLuint textureNames[2];
void UcitajTeksture();

float distance(float x1, float y1, float x2, float y2){
  return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

float pogodjeniUgao(float x, float y);
int racunajPoene(float x1, float y1);

void kreniIgru();
void promeniIgraca();
void resetujStrelice();
void ispisiRezultate();

void drawDart1();
void drawDart2();

void CrtajText(char text[], float x, float y, float z){
  //Crtam tekst
  glRasterPos3f(x, y, z);
  for (int i=0; text[i]; i++) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
  }
}

int main(int argc, char **argv)
{
  /* Inicijalizuje se GLUT. */
  do{
    printf("Unesite broj igraca (max 2)\n");
    scanf("%d", &brojIgraca);
  } while(brojIgraca <= 0 || brojIgraca > 2);

  kreniIgru();

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

  /* Kreira se prozor. */
  glutInitWindowSize(500, 600);
  glutInitWindowPosition(250, 100);
  glutCreateWindow(argv[0]);

  /* Registruju se funkcije za obradu dogadjaja. */
  glutDisplayFunc(on_display);
  glutReshapeFunc(on_reshape);
  glutKeyboardFunc(on_keyboard);
  glutMouseFunc(on_mouse);

  /* Na pocetku je animacija neaktivna */
  animation_ongoing = 0;

  /* Obavlja se OpenGL inicijalizacija. */
  glClearColor(0, 0, 0, 0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_TEXTURE_2D);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
              GL_MODULATE);

  UcitajTeksture();

  float pos[] = { 0, 1, 1, 0 };
  float col[] = { 1, 1, 1, 1 };
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

    /* Ulazi se u glavnu petlju. */
  glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
  animation_ongoing = 1;
  glutMainLoop();

  return 0;
}

void Baci(){
  if (!strelice[trenutnaStrelica].bacena){
    // printf("Vektor bacanja %d %d\n", krajnjeX-pocetnoX, krajnjeY-pocetnoY);
    double deltaX = krajnjeX-pocetnoX;
    double deltaY = krajnjeY-pocetnoY;
    double korekcijaZbogDuzine = 10/timerZaBacanje;
    deltaX *= 0.001 * korekcijaZbogDuzine;
    deltaY *= 0.001 * korekcijaZbogDuzine;
    strelice[trenutnaStrelica].vx = 0 - deltaX;
    strelice[trenutnaStrelica].vy = 0.1 - deltaY;
    strelice[trenutnaStrelica].vz = 0.5;
    strelice[trenutnaStrelica].bacena = 1;
  }
}

static void on_mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    pocetnoX = x;
    pocetnoY = y;
    timerZaBacanje = 0;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
  {
    krajnjeX = x;
    krajnjeY = y;
    Baci();
  }
}

static void on_keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 27:
      /* Zavrsava se program. */
      exit(0);
      break;

    /* Stopira se animacija */
    case 's':
      animation_ongoing = 0;
      break;

  }
}


static void on_reshape(int width, int height){
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60,  (float)width/height, 0.01 ,  1000);

}


static void on_timer(int value)
{
  /*
   * Proverava se da li callback dolazi od odgovarajuceg tajmera.
   */
  if (value != TIMER_ID)
      return;

  //Fizika
  if (strelice[trenutnaStrelica].z < 20 && strelice[trenutnaStrelica].bacena){
    strelice[trenutnaStrelica].vy -= G;
    strelice[trenutnaStrelica].x += strelice[trenutnaStrelica].vx;
    strelice[trenutnaStrelica].y += strelice[trenutnaStrelica].vy;
    strelice[trenutnaStrelica].z += strelice[trenutnaStrelica].vz;
  }
  else if(strelice[trenutnaStrelica].z >= 20 && strelice[trenutnaStrelica].bacena && !strelice[trenutnaStrelica].stiglaDoTable){
    //Trenutak kada stigne do table
    strelice[trenutnaStrelica].stiglaDoTable = 1;
    poeniTokomPoteza += racunajPoene(strelice[trenutnaStrelica].x, strelice[trenutnaStrelica].y);

    //printf("%d \n", poeniTokomPoteza);
    //printf("Ugao %lf \n", pogodjeniUgao(strelice[0].x, strelice[0].y));
  }
  else if (strelice[trenutnaStrelica].stiglaDoTable && animParametar<120){
    //Izvrsava se nekih 2sek dok je strelica u tabli
    animParametar += 1;
  }
  else if (animParametar >= 120){
    //Prebacivanje na sledecu strelicu ili igraca
    animParametar = 0;
    if (trenutnaStrelica == 2 || igraci[trenutniIgrac]-poeniTokomPoteza <= 0){
      promeniIgraca();
    }
    else{
      trenutnaStrelica++;
    }
  }
  timerZaBacanje++;

  glutPostRedisplay();

  /* Po potrebi se ponovo postavlja tajmer. */
  if (animation_ongoing) {
      glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
  }
}

void promeniIgraca(){
  igraci[trenutniIgrac] -= poeniTokomPoteza;
  if (igraci[trenutniIgrac] < 0){
    igraci[trenutniIgrac] *= -1;
  }
  if (igraci[trenutniIgrac] == 0){
    printf("Igrac %d je pobedio!\n", trenutniIgrac+1);
    kreniIgru();
  }
  trenutnaStrelica = 0;
  trenutniIgrac++;
  if (trenutniIgrac == brojIgraca){
    trenutniIgrac = 0;
    trenutniKrug++;
    if (trenutniKrug == 11){
      printf("Game over");
      //obradi pobedu
    }
  }
  resetujStrelice();
  poeniTokomPoteza = 0;
  ispisiRezultate();
}

void ispisiRezultate(){
  printf("\nPoeni\n");
  for (int i=0; i<brojIgraca; i++){
    printf("Igrac %d: %d\n", i, igraci[i]);
  }
  printf("\n");
}

void drawBoard(){
  glBindTexture(GL_TEXTURE_2D, textureNames[0]);

  glPushMatrix();
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5, 0.5);
    glNormal3f(0, 0, -1);
    glVertex3f(0, 0, 0);

    for (float fi = 0; fi < 2*M_PI +0.01; fi += M_PI/20) {
      glTexCoord2f(1-(cos(fi)+1)/2, (sin(fi)+1)/2);
      glVertex3f(cos(fi), sin(fi), 0);
    }
    glEnd();
  glPopMatrix();

    /* Crta se zid */
  glBindTexture(GL_TEXTURE_2D, textureNames[1]);
  glBegin(GL_QUADS);
      glNormal3f(0, 0, -1);

      glTexCoord2f(0, 0);
      glVertex3f(7, -7, 0.011);

      glTexCoord2f(0, 6);
      glVertex3f(7, 7, 0.011);

      glTexCoord2f(6, 6);
      glVertex3f(-7, 7, 0.011);

      glTexCoord2f(6, 0);
      glVertex3f(-7, -7, 0.011);
  glEnd();

  /* Iskljucujemo aktivnu teksturu */
  glBindTexture(GL_TEXTURE_2D, 0);


  /* tabla za poene */
  glPushMatrix();
    glTranslatef(0, 2.5, 0);
    glColor3f(0, 0, 0);
    glBegin(GL_POLYGON);
      glVertex3f(-1.5, 0, 0.01);
      glVertex3f(1.5, 0, 0.01);
      glVertex3f(1.5, 1, 0.01);
      glVertex3f(-1.5, 1, 0.01);
    glEnd();
  glPopMatrix();


}

static void on_display(void)
{
  //glClearColor(backgroundComboColor, backgroundComboColor, backgroundComboColor, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (strelice[trenutnaStrelica].z < 15){
    gluLookAt(
              0, 0, -3,
              0, 0, 0,
              0, 1, 0
            );
  }
  else{
    gluLookAt(
              -2, 2, 13,
              0, 0, 20,
              0, 1, 0
            );
  }


  glPushMatrix();
    glTranslatef(0, 0, 20);
    glScalef(2, 2, 1);
    drawBoard();
    if (brojIgraca == 1) {
      glTranslatef(2, 0, 0);
      drawDart1();
    }
    else {
      glTranslatef(2, 0, 0);
      drawDart1();
      glTranslatef(-6, 0, 0);
      drawDart2();
    }
  glPopMatrix();


  for (int i=0; i<3; i++){
    glPushMatrix();
      glTranslatef(strelice[i].x, strelice[i].y, strelice[i].z-0.25);
      float xAngle = atan2(strelice[i].vy, strelice[i].vz)/M_PI*180;
      // if (strelice[i].vy <= 0){
      //   xAngle *= -1;
      // }
      float yAngle = atan2(strelice[i].vz, strelice[i].vx)/M_PI*180 + 90;
      if (strelice[i].bacena)
        glRotatef(-yAngle, 0, 1, 0);
      glRotatef(xAngle, 1, 0, 0);

      glScalef(1, 1, 5);
      //printf("%lf %lf\n", xAngle, yAngle);

      /* telo strelice */
      if (brojIgraca == 1) {
        glColor3f(1,1,1);
      }
      else {
        if (indikatorIgraca == 0)
          glColor3f(1,1,1);
        else
          glColor3f(1, 0.3, 1);
      }

      glutSolidSphere(0.05, 10, 10);

      /* pera na strelici */
      glPushMatrix();

        glTranslatef(0, 0, 0.03);
        glBegin(GL_TRIANGLES);
          if (brojIgraca == 1) {
            glColor3f(1, 0, 0);
          }
          else {
            if (indikatorIgraca == 0)
              glColor3f(1, 0, 0);
            else
              // glColor3f(0.2, 0.2, 0.9);
              glColor3f(0, 0.2, 1);
          }

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.05);
          glVertex3f(0, 0.2, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.05);
          glVertex3f(0, -0.2, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.05);
          glVertex3f(0.2, 0, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.05);
          glVertex3f(-0.2, 0, 0.08);
        glEnd();

      glPopMatrix();


    glPopMatrix();
  }

  glPushMatrix();
    glTranslatef(0, 0, 20);
    //glutSolidSphere(0.95, 20, 20);
  glPopMatrix();

  char poeni[5];
  //Poeni
  glPushMatrix();
  glTranslatef(0, 2.45, 0);

  for (int i=0; i<brojIgraca; i++){
    glDisable(GL_LIGHTING);
    glColor3f(1, 0, 0);
    indikatorIgraca = 0;

    //glTranslatef((i%2) - 0.5, 2.5-(i/2), 20);
    sprintf(poeni, "%03d", igraci[i]);
    float ypos = 3-(i/2);
    if (i>=4){
      ypos -= 4;
    }
    if (i == trenutniIgrac) {
      glColor3f(0, 1, 0);
      indikatorIgraca = 1;
    }
    CrtajText(poeni, 2.5-(i%2)*3.5, ypos, 19);

    glEnable(GL_LIGHTING);

    //glutSolidSphere(0.95, 20, 20);
  }
  glColor3f(1, 0, 0);
  //Poeni tokom poteza
  sprintf(poeni, "%3d", poeniTokomPoteza);
  CrtajText(poeni, 2.5-1.75, 2.5, 19);
  //Trenutni krug
  sprintf(poeni, "%3d", trenutniKrug);
  CrtajText(poeni, 2.5-1.75, 3.5, 19);

  glPopMatrix();

  glColor3f(1, 1, 1);


  glutSwapBuffers();
}

void UcitajTeksture(){
  glGenTextures(2, textureNames);
  Image* image;
  image = image_init(0, 0);



  /* Generisu se identifikatori tekstura. */
  /* Kreira se prva tekstura. */

  image_read(image, "tabla.bmp");
  glBindTexture(GL_TEXTURE_2D, textureNames[0]);
  glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
               image->width, image->height, 0,
               GL_RGB, GL_UNSIGNED_BYTE, image->pixels);


  /* Kreira se druga tekstura. */
  image_read(image, "wood.bmp");

  glBindTexture(GL_TEXTURE_2D, textureNames[1]);
  glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,
                  GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
               image->width, image->height, 0,
               GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

  /* Iskljucujemo aktivnu teksturu */
  glBindTexture(GL_TEXTURE_2D, 0);

  /* Unistava se objekat za citanje tekstura iz fajla. */
  image_done(image);


}

float pogodjeniUgao(float x, float y){
    float ugao = atan2(y, -x);
    if (y < 0){
      ugao = 2*M_PI + ugao;
    }
    return ugao;
}

int brojevi[] = {6, 13, 4, 18, 1, 20, 5, 12, 9,
14, 11, 8, 16, 7, 19, 3, 17, 2, 15, 10};

int racunajPoene(float x1, float y1){
  float d = distance(x1, y1, 0, 0);
  //printf("Distance %lf\n", d);
  if (d < 0.125){
    return 50;
  }
  if (d < 0.25){
    return 25;
  }
  if (d > 1.6){
    return 0;
  }

  float ugao = pogodjeniUgao(x1, y1);
  ugao += M_PI/20;
  if (ugao >= M_PI*2){
    ugao -= M_PI*2;
  }

  int indeksBroja = ugao/(M_PI/10);
  int broj = brojevi[indeksBroja];
  printf("broj %d\n", broj);
  //printf("Broj: %d\n", brojevi[indeksBroja]);

  if (d > 1.6-0.125){
    return 2*broj;
  }
  else if (d < 0.95 && d > 0.95-0.125){
    return 3*broj;
  }


  return broj;
}

void kreniIgru(){
  for (int i=0; i<brojIgraca; i++){
    igraci[i] = 301;
  }
  trenutniKrug = 1;
  trenutniIgrac = 0;
  trenutnaStrelica = 0;

  resetujStrelice();
}

void resetujStrelice(){
  for (int i=0; i<3; i++){
    strelice[i].x = strelice[i].y = strelice[i].z = 0;
    strelice[i].vx = strelice[i].vy = strelice[i].vz = 0;
    strelice[i].bacena = 0;
    strelice[i].stiglaDoTable = 0;
  }
}


void drawDart1() {
  /* strelica ispod table */

    glPushMatrix();

      glTranslatef(1, -3, -5);
      glRotatef(90, 1, 0, 0);
      glScalef(0.6, 0.6, 0.6);

      /* telo strelice */
      glPushMatrix();
        glColor3f(1, 1, 1);
        glScalef(2, 2, 10);
        glutSolidSphere(0.05, 10, 10);
      glPopMatrix();

      /* pera na strelici */
      glPushMatrix();
        glTranslatef(0, 0, -1);
        glBegin(GL_TRIANGLES);
          glColor3f(1, 0, 0);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.5);
          glVertex3f(0, 0.4, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.6);
          glVertex3f(0, -0.4, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.6);
          glVertex3f(0.4, 0, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.6);
          glVertex3f(-0.4, 0, 0.08);
        glEnd();

      glPopMatrix();

    glPopMatrix();
}

void drawDart2() {
  /* strelica ispod table */
    glPushMatrix();

      glTranslatef(1, -3, -5);
      glRotatef(90, 1, 0, 0);
      glScalef(0.6, 0.6, 0.6);

      /* telo strelice */
      glPushMatrix();
        glColor3f(1, 0.3, 1);
        glScalef(2, 2, 10);
        glutSolidSphere(0.05, 10, 10);
      glPopMatrix();

      /* pera na strelici */
      glPushMatrix();
        glTranslatef(0, 0, -1);
        glBegin(GL_TRIANGLES);
          glColor3f(0, 0.2, 1);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.5);
          glVertex3f(0, 0.4, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.6);
          glVertex3f(0, -0.4, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.6);
          glVertex3f(0.4, 0, 0.08);

          glVertex3f(0, 0, 0);
          glVertex3f(0, 0, 0.6);
          glVertex3f(-0.4, 0, 0.08);
        glEnd();

      glPopMatrix();

    glPopMatrix();
}
