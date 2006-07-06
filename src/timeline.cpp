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

#include <qpainter.h>

#include "timeline.h"
#include "animation.h"

#define KEY_WIDTH   10
#define KEY_HEIGHT  10
#define LINE_HEIGHT 11
#define LEFT_STRUT  55

// FIXME: find out at runtime
#define NUM_PARTS 25

Timeline::Timeline(QWidget *parent, const char *name)
 : QWidget(parent, name)
{
  p=new QPainter(this);

  resize(512,10);

  setAnimation(0);
  setCurrentFrame(0);
  setCaption(tr("Timeline"));
}

Timeline::~Timeline()
{
}

void Timeline::paintEvent(QPaintEvent*)
{
  repaint();
}

void Timeline::repaint()
{
  p->eraseRect(0,0,width(),height());

  if(!animation) return;
  if(isHidden()) return;

  p->setPen(QColor("#000000"));
  int numFrames=animation->getNumberOfFrames();
  resize(numFrames*KEY_WIDTH+LEFT_STRUT,height());

  int y=0;
  for(int part=1;part<NUM_PARTS;part++)
  {
    const int numKeyFrames=animation->numKeyFrames(part);

    if(numKeyFrames)
    {
      const int* keyFrames=animation->keyFrames(part);
      p->drawText(0,y+KEY_HEIGHT,animation->getPartName(part));
      for(int key=0;key<numKeyFrames;key++)
      {
        p->fillRect(keyFrames[key]*KEY_WIDTH+LEFT_STRUT,y,KEY_WIDTH,KEY_HEIGHT,QBrush(QColor("#000000")));
      } // for
      y+=LINE_HEIGHT;
    }
  } // for
  if(height()<y) resize(width(),y);
  drawPosition();
}

void Timeline::setCurrentFrame(int frame)
{
  drawPosition();
  currentFrame=frame;
  drawPosition();
}

void Timeline::setAnimation(Animation* anim)
{
  animation=anim;
  repaint();
}

void Timeline::drawPosition()
{
  p->setRasterOp(Qt::XorROP);
  p->setPen(QColor("#ffffff"));
  p->fillRect(currentFrame*KEY_WIDTH+LEFT_STRUT+KEY_WIDTH/2,0,1,height(),QBrush(QColor("#00ffff")));
  p->setRasterOp(Qt::CopyROP);
  emit positionCenter(currentFrame*KEY_WIDTH+LEFT_STRUT);
}

