/***************************************************************************
 *   Copyright (C) 2006 by Zi Ree   *
 *   Zi Ree @ SecondLife   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef MACOSX
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <qapplication.h>
#include <qstring.h>

#include "animationview.h"
#include "slparts.h"

#define SHIFT 1
#define CTRL  2
#define ALT   4

const char AnimationView::figureFiles[NUM_FIGURES][256] = {
  MALE_BVH,
  FEMALE_BVH
};

AnimationView::AnimationView(QWidget* parent,const char* name,Animation* anim)
 : QGLWidget(parent,name),
  animation(NULL), figType(FEMALE),
  skeleton(false), selecting(false),
  partHighlighted(0), partDragging(0), partSelected(0),
  dragX(0), dragY(0),
  changeX(0), changeY(0), changeZ(0),
  xSelect(false), ySelect(false), zSelect(false)
{
  QString execPath=qApp->applicationDirPath();

  QString limFile=execPath+"/"+LIMITS_FILE;

// FIXME:    mode(FL_DOUBLE | FL_MULTISAMPLE | FL_ALPHA | FL_DEPTH);

  for (int i=0; i<NUM_FIGURES; i++) {
    QString fileName=execPath+"/"+figureFiles[i];
    joints[i] = animRead(fileName, limFile);
  }

  leftMouseButton=false;
  frameProtected=false;
  modifier=0;
  setFigure(figType);
  if(anim) setAnimation(anim);
  setMouseTracking(1);
  setFocusPolicy(QWidget::StrongFocus);
}

AnimationView::~AnimationView()
{
  if(animation) delete(animation);
}

void AnimationView::setAnimation(Animation *anim)
{
  if(animation) delete animation;
  animation=anim;
  connect(animation,SIGNAL(frameChanged()),this,SLOT(repaint()));
  repaint();
}

void AnimationView::drawFloor()
{
  glDisable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_FLAT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBegin(GL_QUADS);
  for (int i=-10; i<10; i++) {
    for (int j=-10; j<10; j++) {
      if ((i+j) % 2)
	if(frameProtected) glColor4f(0.3, 0.0, 0.0, 1); else glColor4f(0.1, 0.1, 0.1, 1);
      else
	if(frameProtected) glColor4f(0.8, 0.0, 0.0, 1); else glColor4f(0.6, 0.6, 0.6, 1);
      glVertex3f(i*40, 0, j*40); glVertex3f(i*40, 0, (j+1)*40);
      glVertex3f((i+1)*40, 0, (j+1)*40); glVertex3f((i+1)*40, 0, j*40);
    }
  }
  glEnd();
}

void AnimationView::setProjection()
{
  gluPerspective(60.0, ((float)width())/height(), 0.01, 2000);
}

void AnimationView::setBodyMaterial()
{
  GLfloat ambientA[] = { 0.9, 0.667, 0.561, 1 };
  GLfloat diffuseA[] = { 0.9, 0.667, 0.561, 0 };
  GLfloat specularA[] = { 0.6, 0.6, 0.6, 0.0 };
  GLfloat shininessA = 100.0;

  glMaterialfv(GL_FRONT, GL_AMBIENT, ambientA);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseA);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specularA);
  glMaterialf(GL_FRONT, GL_SHININESS, shininessA);
}

double goalX = 0, goalY = 0;

void AnimationView::paintGL()
{
  draw();
}

void AnimationView::paintOverlayGL()
{
  draw();
}

void AnimationView::initializeGL()
{
  GLfloat position0 [] = { 0.0, 80.0, 100.0, 1.0 };
  GLfloat ambient0[] = { 0.2, 0.2, 0.2, 1 };
  GLfloat diffuse0[] = { .5, .5, .5, 0.2 };
  GLfloat specular0[] = { 0.5, 0.5, 0.2, 0.5 };

  GLfloat position1 [] = { 0.0, 80.0, -100.0, 1.0 };
  GLfloat ambient1[] = { 0.2, 0.2, 0.2, 1 };
  GLfloat diffuse1[] = { 0.5, 0.5, 0.5, 1 };
  GLfloat specular1[] = { 1, 1, 1, 1 };

    glViewport(0, 0, width(), height());

    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    //  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    //  glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);

    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);

    glEnable(GL_NORMALIZE);

    glEnable(GL_FOG);
    {
      GLfloat fogColor[4] = {0.5, 0.5, 0.5, 0.3};
      int fogMode = GL_EXP; // GL_EXP2, GL_LINEAR
      glFogi (GL_FOG_MODE, fogMode);
      glFogfv (GL_FOG_COLOR, fogColor);
      glFogf (GL_FOG_DENSITY, 0.005);
      glHint (GL_FOG_HINT, GL_DONT_CARE);
      glFogf (GL_FOG_START, 200.0);
      glFogf (GL_FOG_END, 2000.0);
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setProjection();
  glLightfv(GL_LIGHT0, GL_POSITION, position0);
  glLightfv(GL_LIGHT1, GL_POSITION, position1);
}

void AnimationView::draw()
{
  if (!isValid()) {
    initializeGL();
  }

  glClearColor(0.5, 0.5, 0.5, 0.3); /* fog color */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  camera.setModelView();
  drawFloor();
  if (animation) {
    drawFigure();
  }
}

int AnimationView::pickPart(int x, int y)
{
  static const int bufSize = (Animation::MAX_PARTS + 10) * 4;
  GLuint selectBuf[bufSize];
  GLuint *p, num, name = 0;
  GLint hits;
  GLint viewport[4];
  GLint depth = ~0;

  glGetIntegerv (GL_VIEWPORT, viewport);
  glSelectBuffer (bufSize, selectBuf);
  (void) glRenderMode (GL_SELECT);
  glInitNames();
  glPushName(0);
  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();
  gluPickMatrix (x, (viewport[3] - y), 5.0, 5.0, viewport);
  setProjection();
  camera.setModelView();
  drawFigure();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();
  glFlush ();
  hits = glRenderMode (GL_RENDER);
  p = selectBuf;
  for (int i=0; i < hits; i++) {
    num = *(p++);
    if (*p < depth) {
      depth = *p;
      name = *(p+2);
    }
    p+=2;
    for (int j=0; j < num; j++) { *(p++); }
  }
  return name;
}

void AnimationView::mouseMoveEvent(QMouseEvent* event)
{
  dragX=event->x()-last_x;
  dragY=event->y()-last_y;

  last_x=event->x();
  last_y=event->y();

  if(leftMouseButton)
  {
    if (partDragging) {
      changeX = changeY = changeZ = 0;
      if (modifier & SHIFT) { changeX = dragY; }
      if (modifier & ALT)   { changeY = dragX; }
      else if (modifier & CTRL) { changeZ = -dragX; }
      emit partDragged(getSelectedPart(),changeX,changeY,changeZ);
    }
    else {
      if (modifier & SHIFT)
        camera.pan(dragX/2, dragY/2,0);
      else if (modifier & ALT) {
        camera.pan(0, 0, dragY);
        camera.rotate(0, dragX);
      }
      else
        camera.rotate(dragY, dragX);
    }
  }
  else
  {
    partHighlighted = pickPart(last_x, last_y);
  }
  repaint();
}

void AnimationView::mousePressEvent(QMouseEvent* event)
{
  if(event->button()==LeftButton)
  {
    leftMouseButton=true;

    last_x=event->x();
    last_y=event->y();

    int selected = pickPart(last_x, last_y);
    if (selected != partSelected)
      animation->setMirrored(false);
    partDragging = partSelected = selected;
    if (selected) {
      QString part=getSelectedPart();
      changeX = changeY = changeZ = 0;
      dragX = dragY = 0;
      emit partClicked(part,
                       Rotation(animation->getRotation(part)),
                       animation->getRotationLimits(part),
                       Position(animation->getPosition(part))
                      );
    }
    else emit backgroundClicked();

    repaint();
  }
}

void AnimationView::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button()==LeftButton)
  {
    leftMouseButton=false;
    partDragging = 0;
  }
}

void AnimationView::mouseDoubleClickEvent(QMouseEvent* event)
{
  int selected = pickPart(last_x, last_y);
  if (modifier & SHIFT)
    animation->setMirrored(true);
  else if (selected)
    animation->setIK(animation->getPartName(selected),
                     !animation->getIK(animation->getPartName(selected)));
  repaint();
}

void AnimationView::wheelEvent(QWheelEvent* event)
{
  camera.pan(0,0,-event->delta()/12);
  repaint();
}

void AnimationView::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    case Qt::Key_Prior:
      camera.pan(0,0,-5);
      repaint();
      break;
    case Qt::Key_Next:
        camera.pan(0,0,5);
        repaint();
        break;
    case Qt::Key_Shift:
      modifier|=SHIFT;
      xSelect = true;
      repaint();
      break;
    case Qt::Key_Alt:
      modifier|=ALT;
      ySelect = true;
      repaint();
      break;
    case Qt::Key_Control:
      modifier|=CTRL;
      zSelect = true;
      repaint();
      break;
  }
}

void AnimationView::keyReleaseEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    case Qt::Key_Shift:
      modifier&=!SHIFT;
      xSelect = false;
      repaint();
      break;
    case Qt::Key_Alt:
      modifier&=!ALT;
      ySelect = false;
      repaint();
      break;
    case Qt::Key_Control:
      modifier&=!CTRL;
      zSelect = false;
      repaint();
      break;
  }
}

void AnimationView::drawFigure()
{
    glShadeModel(GL_SMOOTH);
    setBodyMaterial();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glTranslatef(0, 2, 0);
    selectName = 0;
    glEnable(GL_DEPTH_TEST);
    drawPart(animation->getFrame(), animation->getMotion(), joints[figType],
	     MODE_PARTS);
    selectName = 0;
    glEnable(GL_COLOR_MATERIAL);
    drawPart(animation->getFrame(), animation->getMotion(), joints[figType],
	     MODE_ROT_AXES);
    selectName = 0;
    glDisable(GL_DEPTH_TEST);
    drawPart(animation->getFrame(), animation->getMotion(), joints[figType],
	     MODE_SKELETON);
}

void AnimationView::drawPart(int frame, BVHNode *motion, BVHNode *joints, int mode)
{
//  float x, y, z;
  float value, color[4];
//  int offset = 0;
  GLint renderMode;
  bool selecting;

  glGetIntegerv(GL_RENDER_MODE, &renderMode);
  selecting = (renderMode == GL_SELECT);
  if (motion && joints) {
    selectName++;
    glPushMatrix();
    glTranslatef(joints->offset[0], joints->offset[1], joints->offset[2]);
    if (!Animation::isSecondLifeJoint(motion)) {
      selectName++;
      motion = motion->child[0];
    }
    if (mode == MODE_SKELETON && skeleton && !selecting) {
      glColor4f(0,1,1,1);
      glLineWidth(1);
      glBegin(GL_LINES);
        glVertex3f(-joints->offset[0], -joints->offset[1], -joints->offset[2]);
        glVertex3f(0,0,0);
      glEnd();
      glColor4f(0, 1, 0, 1);
      if (joints->type != BVH_ROOT)

// FIXME:	glutSolidSphere(1, 16, 16)
        ;
    }
    if (joints->type == BVH_ROOT) {
      for (int i=0; i<motion->numChannels; i++) {
	value = motion->frame[frame][i];
	switch(motion->channelType[i]) {
	case BVH_XPOS: glTranslatef(value, 0, 0); break;
	case BVH_YPOS: glTranslatef(0, value, 0); break;
	case BVH_ZPOS: glTranslatef(0, 0, value); break;
	}
      }
    }
    for (int i=0; i<motion->numChannels; i++) {
      if (motion->ikOn)
	value = motion->frame[frame][i] + motion->ikRot[i];
      else
	value = motion->frame[frame][i];
      switch(motion->channelType[i]) {
      case BVH_XROT: glRotatef(value, 1, 0, 0); break;
      case BVH_YROT: glRotatef(value, 0, 1, 0);	break;
      case BVH_ZROT: glRotatef(value, 0, 0, 1);	break;
      }
      if (mode == MODE_ROT_AXES && !selecting && partSelected == selectName) {
	switch(motion->channelType[i]) {
	case BVH_XROT: drawCircle(0, 10, xSelect ? 4 : 1); break;
	case BVH_YROT: drawCircle(1, 10, ySelect ? 4 : 1); break;
	case BVH_ZROT: drawCircle(2, 10, zSelect ? 4 : 1); break;
	}
      }
    }
    if (mode == MODE_PARTS) {
      if (selecting) glLoadName(selectName);
      if (animation->getMirrored() &&
	  (animation->getPartMirror(partSelected) == selectName ||
	   partSelected == selectName))
	glColor4f(1.0, 0.635, 0.059, 1);
      else if (partSelected == selectName)
	glColor4f(0.6, 0.3, 0.3, 1);
      else if (partHighlighted == selectName)
	glColor4f(0.4, 0.5, 0.3, 1);
      else
	glColor4f(0.6, 0.5, 0.5, 1);
      if (animation->getIK(motion->name)) {
	glGetFloatv(GL_CURRENT_COLOR, color);
	glColor4f(color[0], color[1], color[2]+0.3, color[3]);
      }
      figType==MALE ? drawSLMalePart(motion->name):drawSLFemalePart(motion->name);
    }
    for (int i=0; i<motion->numChildren; i++) {
      drawPart(frame, motion->child[i], joints->child[i], mode);
    }
    glPopMatrix();
  }
}

void AnimationView::drawCircle(int axis, float radius, int width)
{
  GLint circle_points = 100;
  float angle;

  glLineWidth(width);
  switch (axis) {
  case 0: glColor4f(1,0,0,1);break;
  case 1: glColor4f(0,1,0,1);break;
  case 2: glColor4f(0,0,1,1);break;
  }
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < circle_points; i++) {
    angle = 2*M_PI*i/circle_points;
    switch (axis) {
    case 0: glVertex3f(0, radius*cos(angle), radius*sin(angle));break;
    case 1: glVertex3f(radius*cos(angle), 0, radius*sin(angle));break;
    case 2: glVertex3f(radius*cos(angle), radius*sin(angle), 0);break;
    }
  }
  glEnd();
  glBegin(GL_LINES);
  switch (axis) {
  case 0: glVertex3f(-10, 0, 0); glVertex3f(10, 0, 0); break;
  case 1: glVertex3f(0, -10, 0); glVertex3f(0, 10, 0); break;
  case 2: glVertex3f(0, 0, -10); glVertex3f(0, 0, 10); break;
  }
  glEnd();

}

void AnimationView::setFigure(FigureType type)
{
  if (type >= 0 && type < NUM_FIGURES)
    figType = type;
}

const char *AnimationView::getSelectedPart()
{
  return animation->getPartName(partSelected);
}

void AnimationView::selectPart(const char *part)
{
  partSelected = animation->getPartIndex(part);
  emit partClicked(part,
                   Rotation(animation->getRotation(part)),
                   animation->getRotationLimits(part),
                   Position(animation->getPosition(part))
                  );
  repaint();
}

void AnimationView::resetCamera()
{
  camera.reset();
  repaint();
}

// handle widget resizes
void AnimationView::resizeEvent(QResizeEvent* newSize)
{
  int w=newSize->size().width();
  int h=newSize->size().height();
  float aspect=1.0*w/h;

  // reset coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // resize GL viewport
  glViewport(0,0,w,h);

  // set up aspect ratio
  setProjection();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// set the current frame's visual protection status
void AnimationView::protectFrame(bool on)
{
  // only redraw if we need to
  if(frameProtected!=on)
  {
    frameProtected=on;
    repaint();
  }
}
