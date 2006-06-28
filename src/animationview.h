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
 ***************************************************************************

 * Copyright (C) 2006 by Vinay Pulim.
 * ported to Qt by Zi Ree
 * All rights reserved.             */

/**
  @author Vinay Pulim
  @author Zi Ree <Zi Ree @ SecondLife>
*/

#ifndef ANIMATIONVIEW_H
#define ANIMATIONVIEW_H

#include <qgl.h>
#include <qobject.h>

#include "animation.h"
#include "camera.h"
#include "rotation.h"

#define MALE_BVH   "data/SLMale.bvh"
#define FEMALE_BVH "data/SLFemale.bvh"

class AnimationView : public QGLWidget
{
  Q_OBJECT

  public:

    typedef enum
    {
      MALE,
      FEMALE,
      NUM_FIGURES
    } FigureType;

    AnimationView(QWidget* parent=0,const char* name=0,Animation* anim=0);
    ~AnimationView();

    void setAnimation(Animation *anim);
    Animation *getAnimation() { return animation; }
    bool isSkeletonOn() { return skeleton; }
    FigureType getFigure() { return figType; }
    void setFigure(FigureType type);
    void showSkeleton() { skeleton = true; }
    void hideSkeleton() { skeleton = false; }
    void selectPart(const char *part);
    const char *getSelectedPart();
    void getChangeValues(double *x, double *y, double *z);

  signals:
    void partClicked(const QString& partName,Rotation rot,RotationLimits rotLimit,Position pos);
    void partDragged(const QString&,double changeX,double changeY,double changeZ);
    void backgroundClicked();

  protected:
    bool leftMouseButton;
    char modifier;

    virtual void paintGL();
    virtual void paintOverlayGL();
    virtual void initializeGL();
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

    void draw();

  private:
    typedef enum
    {
      MODE_PARTS,
      MODE_SKELETON,
      MODE_ROT_AXES
    };

    static const char figureFiles[NUM_FIGURES][256];
    Animation *animation;
    int last_x, last_y;
    Camera camera;
    double changeX, changeY, changeZ;
    BVHNode *joints[NUM_FIGURES];
    bool skeleton;
    bool selecting;
    int selectName;
    int partHighlighted;
    int partSelected;
    int partDragging;
    int dragX, dragY;
    int drawMode;
    bool xSelect, ySelect, zSelect;
    FigureType figType;

    void drawFloor();
    void drawFigure();
    void drawPart(int frame, BVHNode *motion, BVHNode *joints, int mode);
    void setProjection();
    void setModelView();
    void setBodyMaterial();
    void resetCamera();
    void clearSelected();
    int pickPart(int x, int y);
    void drawCircle(int axis, float radius, int width);
};

#endif
