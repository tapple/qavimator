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
#include <OpenGL/glut.h>
#else
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <qapplication.h>
#include <qstring.h>
#include <qcursor.h>
#include <qtimer.h>

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
 : QGLWidget(parent,name)
{
  // Ptrlist properties
  animList.setAutoDelete(true);

  // fake glut initialization
  int args=1;
  char* arg[]={"qavimator"};
  glutInit(&args,arg);

  // init
  figType=FEMALE;
  skeleton=false;
  selecting=false;
  partHighlighted=0;
  propDragging=0;
  partSelected=0;
  dragX=0;
  dragY=0;
  changeX=0;
  changeY=0;
  changeZ=0;
  xSelect=false;
  ySelect=false;
  zSelect=false;
  nextPropId=OBJECT_START;

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
  objectNum=0;
  setFigure(figType);
  if(anim) setAnimation(anim);
  setMouseTracking(true);
  setFocusPolicy(QWidget::StrongFocus);
}

AnimationView::~AnimationView()
{
    clear();
    animList.clear();
}

void AnimationView::selectAnimation(unsigned int index)
{
    if (index < animList.count())
    {
	animation = animList.at(index);
	emit animationSelected(animation);
	repaint();
    }
}

void AnimationView::setAnimation(Animation *anim)
{
    clear();

    animation = anim;
    animList.append(anim);
    connect(anim,SIGNAL(frameChanged()),this,SLOT(repaint()));
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

      glVertex3f(i*40, 0, j*40);
      glVertex3f(i*40, 0, (j+1)*40);
      glVertex3f((i+1)*40, 0, (j+1)*40);
      glVertex3f((i+1)*40, 0, j*40);
    }
  }

  glEnd();
}

void AnimationView::drawProp(const Prop* prop) const
{
  Prop::State state=Prop::Normal;
  if(propSelected==prop->id)
    state=Prop::Selected;
  else if(partHighlighted==prop->id) state=Prop::Highlighted;
  prop->draw(state);
  if(propSelected==prop->id)
    drawDragHandles(prop);
}

void AnimationView::drawProps()
{
  for(unsigned int index=0;index<propList.count();index++)
  {
    Prop* prop=propList.at(index);
    if(!prop->isAttached()) drawProp(prop);
  }
}

// Adds a new animation without overriding others, and sets it current
void AnimationView::addAnimation(Animation *anim)
{
    if (!inAnimList(anim))
    {
	animList.append(anim);
	animation = anim; // set it as the current one
	if(animList.count() && anim != animList.first())
	    anim->setFrame(animList.first()->getFrame());
	connect(anim,SIGNAL(frameChanged()),this,SLOT(repaint()));
	repaint();
    }
}

void AnimationView::clear()
{
    animList.clear();
    animation = NULL;
}

void AnimationView::setFrame(int frame)
{
//  qDebug(QString("animationview->frame now %1").arg(frame));
    for (unsigned int i = 0; i < animList.count(); i++)
    {
	animList.at(i)->setFrame(frame);
    }
}

void AnimationView::stepForward()
{
    for (unsigned int i = 0; i < animList.count(); i++)
    {
	animList.at(i)->stepForward();
    }
}

void AnimationView::setFrameTime(double time)
{
    for (unsigned int i = 0; i < animList.count(); i++)
    {
	animList.at(i)->setFrameTime(time);
    }
}

const Prop* AnimationView::addProp(Prop::PropType type,double x,double y,double z,double xs,double ys,double zs,double xr,double yr,double zr,int attach)
{
  QString name;

  do
  {
    name="Object "+QString::number(objectNum++);
  } while(getPropByName(name));

  Prop* newProp=new Prop(nextPropId,type,name);

  nextPropId++;

  newProp->attach(attach);

  newProp->setPosition(x,y,z);
  newProp->setRotation(xr,yr,zr);
  newProp->setScale(xs,ys,zs);

  propList.append(newProp);
  repaint();

  return newProp;
}

Prop* AnimationView::getPropByName(const QString& lookName)
{
  for(unsigned int index=0;index<propList.count();index++)
  {
    Prop* prop=propList.at(index);
    if(prop->name()==lookName) return prop;
  }

  return 0;
}

Prop* AnimationView::getPropById(unsigned int id)
{
  for(unsigned int index=0;index<propList.count();index++)
  {
    Prop* prop=propList.at(index);
    if(prop->id==id) return prop;
  }

  return 0;
}

void AnimationView::deleteProp(Prop* prop)
{
  propList.remove(prop);
  delete prop;
  repaint();
}

void AnimationView::clearProps()
{
  while(propList.count())
  {
    Prop* prop=propList.at(0);
    propList.remove(prop);
    delete prop;
  }
  repaint();
}

bool AnimationView::inAnimList(Animation *anim)
{
    return (animList.find(anim) != -1);
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
//  GLfloat diffuse0[] = { .5, .5, .5, 0.2 };
//  GLfloat specular0[] = { 0.5, 0.5, 0.2, 0.5 };

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
  if (!isValid()) initializeGL();

  glClearColor(0.5, 0.5, 0.5, 0.3); /* fog color */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  camera.setModelView();
  drawFloor();
  if (!animList.isEmpty()) drawAnimations();
  drawProps();
}

void AnimationView::drawAnimations()
{
    for (unsigned int index = 0; index < animList.count(); index++)
    {
	drawFigure(animList.at(index),index);
    }
}

int AnimationView::pickPart(int x, int y)
{
  int bufSize = ((Animation::MAX_PARTS + 10)*animList.count() + propList.count()) * 4;

  GLuint* selectBuf=(GLuint*) malloc(bufSize);

//  GLuint selectBuf[bufSize];

  GLuint *p, num, name = 0;
  GLint hits;
  GLint viewport[4];
  GLuint depth = ~0;

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
  drawAnimations();
  drawProps();
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
    for (unsigned int j=0; j < num; j++) { *(p++); }
  }

  free(selectBuf);
  return name;
}

void AnimationView::mouseMoveEvent(QMouseEvent* event)
{
  if(leftMouseButton)
  {
    QPoint dragPos=QCursor::pos();
    QCursor::setPos(clickPos);

    dragX=dragPos.x()-clickPos.x();
    dragY=dragPos.y()-clickPos.y();

    if (partSelected) {

      if(partSelected<OBJECT_START)
      {
        changeX = changeY = changeZ = 0;
        if (modifier & SHIFT) { changeX = dragY; }
        if (modifier & ALT)   { changeY = dragX; }
        else if (modifier & CTRL) { changeZ = -dragX; }
        emit partDragged(getSelectedPart(),changeX,changeY,changeZ);
        repaint();
      }
    }
    else if(propDragging)
    {
      Prop* prop=getPropById(propSelected);

      if(propDragging==DRAG_HANDLE_X)
      {
        emit propDragged(prop,(double) dragX/10.0,0,0);
      }
      else if(propDragging==DRAG_HANDLE_Y)
      {
        emit propDragged(prop,0,(double) -dragY/10.0,0);
      }
      else if(propDragging==DRAG_HANDLE_Z)
      {
        emit propDragged(prop,0,0,(double) dragY/10.0);
      }
      else if(propDragging==SCALE_HANDLE_X)
      {
        emit propScaled(prop,(double) dragX/10.0,0,0);
      }
      else if(propDragging==SCALE_HANDLE_Y)
      {
        emit propScaled(prop,0,(double) -dragY/10.0,0);
      }
      else if(propDragging==SCALE_HANDLE_Z)
      {
        emit propScaled(prop,0,0,(double) dragY/10.0);
      }
      else if(propDragging==ROTATE_HANDLE_X)
      {
        emit propRotated(prop,(double) dragX/5.0,0,0);
      }
      else if(propDragging==ROTATE_HANDLE_Y)
      {
        emit propRotated(prop,0,(double) -dragY/5.0,0);
      }
      else if(propDragging==ROTATE_HANDLE_Z)
      {
        emit propRotated(prop,0,0,(double) dragY/5.0);
      }
      repaint();
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

      repaint();
    }
  }
  else
  {
    unsigned int oldPart=partHighlighted;
    partHighlighted=pickPart(event->x(),event->y());
    if(oldPart!=partHighlighted) repaint();
  }
}

void AnimationView::mousePressEvent(QMouseEvent* event)
{
  if(event->button()==LeftButton)
  {
    leftMouseButton=true;
    // hide mouse cursor to avoid confusion
    setCursor(QCursor(Qt::BlankCursor));
    // remember mouse position for dragging
    clickPos=QCursor::pos();

    // check out which part or prop has been clicked
    unsigned int selected=pickPart(event->x(),event->y());

    // if another part than the current one has been clicked, switch off mirror mode
    if(selected!=partSelected) animation->setMirrored(false);

    // background clicked, reset all
    if(!selected)
    {
      partSelected=0;
      propSelected=0;
      propDragging=0;
      emit backgroundClicked();
    }
    // body part clicked
    else if(selected<OBJECT_START)
    {
      partSelected=selected;
      selectAnimation(selected/ANIMATION_INCREMENT);
      propSelected=0;
      propDragging=0;

      QString part=getSelectedPart();
      changeX = changeY = changeZ = 0;
      dragX = dragY = 0;

      emit partClicked(part,
                       Rotation(animation->getRotation(part)),
                       animation->getRotationLimits(part),
                       Position(animation->getPosition(part))
                      );
    }
    // drag handle clicked
    else if(selected==DRAG_HANDLE_X ||
            selected==DRAG_HANDLE_Y ||
            selected==DRAG_HANDLE_Z ||
            selected==SCALE_HANDLE_X ||
            selected==SCALE_HANDLE_Y ||
            selected==SCALE_HANDLE_Z ||
            selected==ROTATE_HANDLE_X ||
            selected==ROTATE_HANDLE_Y ||
            selected==ROTATE_HANDLE_Z)
    {
      propDragging=selected;
      changeX = changeY = changeZ = 0;
      dragX = dragY = 0;
    }
    else
    {
      emit propClicked(getPropById(selected));
      propSelected=selected;
    }
    repaint();
  }
}

void AnimationView::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button()==LeftButton)
  {
    // show mouse cursor again
    setCursor(Qt::ArrowCursor);
    leftMouseButton=false;
    propDragging=0;
  }
}

void AnimationView::mouseDoubleClickEvent(QMouseEvent* event)
{
  int selected = pickPart(event->x(),event->y());

  // no double clicks for props or drag handles
  if(selected>=OBJECT_START) return;

  if (modifier & SHIFT)
    animation->setMirrored(true);
  else if (selected && selected < OBJECT_START)
    animation->setIK(getPartName(selected),
                     !animation->getIK(getPartName(selected)));
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
  event->ignore();
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
  event->ignore();
}

void AnimationView::drawFigure(Animation* anim,unsigned int index)
{
    glShadeModel(GL_SMOOTH);
    setBodyMaterial();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glTranslatef(0, 2, 0);
    selectName = index*ANIMATION_INCREMENT;
    glEnable(GL_DEPTH_TEST);
    drawPart(anim, index, anim->getFrame(), anim->getMotion(), joints[figType],
	     MODE_PARTS);
    selectName = index*ANIMATION_INCREMENT;
    glEnable(GL_COLOR_MATERIAL);
    drawPart(anim, index, anim->getFrame(), anim->getMotion(), joints[figType],
	     MODE_ROT_AXES);
    selectName = index*ANIMATION_INCREMENT;
    glDisable(GL_DEPTH_TEST);
    drawPart(anim, index, anim->getFrame(), anim->getMotion(), joints[figType],
	     MODE_SKELETON);
}

void AnimationView::drawPart(Animation* anim, unsigned int currentAnimationIndex, int frame, BVHNode *motion, BVHNode *joints, int mode)
{
  float value, color[4];

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
        glutSolidSphere(1, 16, 16);
    }
    if (joints->type == BVH_ROOT) {
      for (int i=0; i<motion->numChannels; i++) {
        value = motion->frame[frame][i];
        switch(motion->channelType[i]) {
          case BVH_XPOS: glTranslatef(value, 0, 0); break;
          case BVH_YPOS: glTranslatef(0, value, 0); break;
          case BVH_ZPOS: glTranslatef(0, 0, value); break;
          default: break;
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
        case BVH_YROT: glRotatef(value, 0, 1, 0); break;
        case BVH_ZROT: glRotatef(value, 0, 0, 1); break;
        default: break;
      }
      if (mode == MODE_ROT_AXES && !selecting && partSelected == selectName) {
        switch(motion->channelType[i]) {
          case BVH_XROT: drawCircle(0, 10, xSelect ? 4 : 1); break;
          case BVH_YROT: drawCircle(1, 10, ySelect ? 4 : 1); break;
          case BVH_ZROT: drawCircle(2, 10, zSelect ? 4 : 1); break;
          default: break;
        }
      }
    }
    if (mode == MODE_PARTS) {
      if (selecting) glLoadName(selectName);
      if (anim->getMirrored() &&
	  (anim->getPartMirror(partSelected) == selectName ||
	   partSelected == selectName))
	glColor4f(1.0, 0.635, 0.059, 1);
      else if (partSelected == selectName)
	glColor4f(0.6, 0.3, 0.3, 1);
      else if (partHighlighted == selectName)
	glColor4f(0.4, 0.5, 0.3, 1);
      else
	glColor4f(0.6, 0.5, 0.5, 1);
      if (anim->getIK(motion->name)) {
	glGetFloatv(GL_CURRENT_COLOR, color);
	glColor4f(color[0], color[1], color[2]+0.3, color[3]);
      }
      figType==MALE ? drawSLMalePart(motion->name):drawSLFemalePart(motion->name);

      for(unsigned int index=0;index<propList.count();index++)
      {
        Prop* prop=propList.at(index);
        if(prop->isAttached()==selectName) drawProp(prop);
      } // for
    }
    for (int i=0; i<motion->numChildren; i++) {
      drawPart(anim, currentAnimationIndex, frame, motion->child[i], joints->child[i], mode);
    }
    glPopMatrix();
  }
}

void AnimationView::drawDragHandles(const Prop* prop) const
{
  // get prop's position
  double x=prop->x;
  double y=prop->y;
  double z=prop->z;
 // get prop's scale
  double xs=prop->xs/2.0;
  double ys=prop->ys/2.0;
  double zs=prop->zs/2.0;

  // remember drawing matrix on stack
  glPushMatrix();
  // set matrix origin to our object's center
  glTranslatef(x,y,z);

  if(modifier & SHIFT)
  {
    // now draw the scale cubes with proper depth sorting
    glEnable(GL_DEPTH_TEST);

    glRotatef(prop->xr,1,0,0);
    glRotatef(prop->yr,0,1,0);
    glRotatef(prop->zr,0,0,1);

    glLoadName(SCALE_HANDLE_X);
    glColor4f(1,0,0,1);
    glTranslatef(-xs,0,0);
    glutSolidCube(2);
    glTranslatef(xs*2,0,0);
    glutSolidCube(2);

    glLoadName(SCALE_HANDLE_Y);
    glColor4f(0,1,0,1);
    glTranslatef(-xs,-ys,0);
    glutSolidCube(2);
    glTranslatef(0,ys*2,0);
    glutSolidCube(2);

    glLoadName(SCALE_HANDLE_Z);
    glColor4f(0,0,1,1);
    glTranslatef(0,-ys,-zs);
    glutSolidCube(2);
    glTranslatef(0,0,zs*2);
    glutSolidCube(2);
  }
  else if(modifier & CTRL)
  {
    // now draw the rotate spheres with proper depth sorting
    glEnable(GL_DEPTH_TEST);

    glLoadName(ROTATE_HANDLE_X);
    glColor4f(1,0,0,1);
    glTranslatef(-xs-5,0,0);
    glutSolidSphere(1, 16, 16);
    glTranslatef(2*(xs+5),0,0);
    glutSolidSphere(1, 16, 16);

    glLoadName(ROTATE_HANDLE_Y);
    glColor4f(0,1,0,1);
    glTranslatef(-xs-5,-ys-5,0);
    glutSolidSphere(1, 16, 16);
    glTranslatef(0,2*(ys+5),0);
    glutSolidSphere(1, 16, 16);

    glLoadName(ROTATE_HANDLE_Z);
    glColor4f(0,0,1,1);
    glTranslatef(0,-ys-5,-zs-5);
    glutSolidSphere(1, 16, 16);
    glTranslatef(0,0,2*(zs+5));
    glutSolidSphere(1, 16, 16);
  }
  else
  {
    // first draw the crosshair lines without depth testing, so they are always visible
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1);

    glBegin(GL_LINES);
    glColor4f(1,0,0,1);
    glVertex3f(-xs-5, 0, 0);
    glVertex3f( xs+5, 0, 0);
    glEnd();

    glBegin(GL_LINES);
    glColor4f(0,1,0,1);
    glVertex3f(0,-ys-5, 0);
    glVertex3f(0, ys+5, 0);
    glEnd();

    glBegin(GL_LINES);
    glColor4f(0,0,1,1);
    glVertex3f(0, 0,-zs-5);
    glVertex3f(0, 0, zs+5);
    glEnd();

    // now draw the drag handle arrows with proper depth sorting
    glEnable(GL_DEPTH_TEST);

    glLoadName(DRAG_HANDLE_X);
    glColor4f(1,0,0,1);
    glTranslatef(-xs-5,0,0);
    glRotatef(270,0,1,0);
    glutSolidCone(1, 3, 16, 16);
    glRotatef(90,0,1,0);
    glTranslatef(2*(xs+5),0,0);
    glRotatef(90,0,1,0);
    glutSolidCone(1, 3, 16, 16);
    glRotatef(270,0,1,0);

    glLoadName(DRAG_HANDLE_Y);
    glColor4f(0,1,0,1);
    glTranslatef(-xs-5,-ys-5,0);
    glRotatef(90,1,0,0);
    glutSolidCone(1, 3, 16, 16);
    glRotatef(270,1,0,0);
    glTranslatef(0,2*(ys+5),0);
    glRotatef(270,1,0,0);
    glutSolidCone(1, 3, 16, 16);
    glRotatef(90,1,0,0);

    glLoadName(DRAG_HANDLE_Z);
    glColor4f(0,0,1,1);
    glTranslatef(0,-ys-5,-zs-5);
    glRotatef(180,1,0,0);
    glutSolidCone(1, 3, 16, 16);
    glRotatef(180,1,0,0);
    glTranslatef(0,0,2*(zs+5));
    glutSolidCone(1, 3, 16, 16);
  }
  // restore drawing matrix
  glPopMatrix();
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
  return getPartName(partSelected);
}

const char* AnimationView::getPartName(int index)
{
  // get part name from animation, with respect to multiple animations in view
  return animation->getPartName(index % ANIMATION_INCREMENT);
}

const QString& AnimationView::getSelectedPropName()
{
  for(unsigned int index=0;index<propList.count();index++)
    if(propList.at(index)->id==partSelected) return propList.at(index)->name();
  return QString::null;
}

void AnimationView::selectPart(const char *part)
{
  // make sure no prop is selected anymore
  propSelected=0;
  propDragging=0;
  partSelected=animation->getPartIndex(part);
  emit partClicked(part,
                   Rotation(animation->getRotation(part)),
                   animation->getRotationLimits(part),
                   Position(animation->getPosition(part))
                  );
  repaint();
}

void AnimationView::selectProp(const QString& propName)
{
  // make sure no part is selected anymore
  partSelected=0;
  Prop* prop=getPropByName(propName);
  if(prop) propSelected=prop->id;
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
