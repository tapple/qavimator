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
#include <qapplication.h>

#include "timeline.h"
#include "animation.h"
#include "keyframelist.h"

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

  animation=0;
  setCurrentFrame(0);
  setCaption(tr("Timeline"));
  leftMouseButton=false;
  dragging=0;
  trackSelected=0;
  frameSelected=0;

  tracks.clear();
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
  if(isHidden()) return;
  p->eraseRect(0,0,width(),height());

  if(!animation) return;

  resize(numOfFrames*KEY_WIDTH+LEFT_STRUT,(NUM_PARTS-1)*LINE_HEIGHT);

  drawPosition();
  for(int part=1;part<NUM_PARTS;part++)
  {
    drawTrack(part);
  } // for
}

void Timeline::setNumberOfFrames(int frames)
{
  // TODO: this works, but the animation class seems not to delete keyframes after maxframe, so
  //       for now we should keep them around as well
  /*  if(frames<numOfFrames)
  {
    qDebug(QString("new %1, old %2").arg(frames).arg(numOfFrames));
    QValueList<int> keys=tracks.keys();
    for(unsigned int index=0;index<keys.count();index++)
    {
      for(unsigned int count=frames;count<numOfFrames;count++)
      {
        removeKeyframe(keys[index],count);
      } // for
    } // for
  }*/
  numOfFrames=frames;
  repaint();
}

void Timeline::setCurrentFrame(int frame)
{
  drawPosition();
  currentFrame=frame;
  drawPosition();
}

void Timeline::setAnimation(Animation* anim)
{
  if(animation)
  {
    disconnect(animation,SIGNAL(numberOfFrames(int)),this,SLOT(setNumberOfFrames(int)));
    disconnect(animation,SIGNAL(keyframeAdded(int,int)),this,SLOT(addKeyframe(int,int)));
    disconnect(animation,SIGNAL(keyframeRemoved(int,int)),this,SLOT(removeKeyframe(int,int)));
    numOfFrames=0;
  }
  animation=anim;

  if(animation)
  {
    connect(animation,SIGNAL(numberOfFrames(int)),this,SLOT(setNumberOfFrames(int)));
    connect(animation,SIGNAL(keyframeAdded(int,int)),this,SLOT(addKeyframe(int,int)));
    connect(animation,SIGNAL(keyframeRemoved(int,int)),this,SLOT(removeKeyframe(int,int)));
    numOfFrames=animation->getNumberOfFrames();
  }

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

void Timeline::addKeyframe(int track,int frame)
{
  if(tracks.find(track)==tracks.end())
  {
    tracks[track]=KeyframeList();
  }
  tracks[track][frame]=1;

  drawPosition();
  drawKeyframe(track,frame);
  drawPosition();
}

void Timeline::removeKeyframe(int track,int frame)
{
  tracks[track].erase(frame);

  if(!tracks[track].count()) tracks.erase(track);
  drawTrack(track);
}

void Timeline::drawKeyframe(int track,int frame)
{
  p->setPen(Qt::black);

  QValueList<int> keys=tracks.keys();

  int ypos=(track-1)*LINE_HEIGHT;
  for(unsigned int index=0;index<keys.count();index++)
  {
    if(keys[index]==track)
    {
      p->setPen(Qt::black);

      int xpos=frame*KEY_WIDTH+LEFT_STRUT;

      p->fillRect(xpos,ypos,KEY_WIDTH-1,KEY_HEIGHT,QBrush(Qt::black));
      if(frame>0 && frame!=(numOfFrames-1))
        p->drawLine(LEFT_STRUT,ypos+KEY_HEIGHT/2,xpos,ypos+KEY_HEIGHT/2);
    }
  }
}

void Timeline::drawTrack(int track)
{
  drawPosition();

  int y=(track-1)*LINE_HEIGHT;

  p->setPen(QColor("#000000"));
  p->eraseRect(0,y,width(),LINE_HEIGHT);

  // always draw track name
  p->drawText(0,y+KEY_HEIGHT,animation->getPartName(track));

  const int numKeyFrames=animation->numKeyFrames(track);

  // TODO: switch over to something like this
  /*  KeyframeList keyFrames=tracks[track];
  const int numKeyFrames=keyFrames.count();*/
  if(numKeyFrames)
  {
    const int* keyFrames=animation->keyFrames(track);

    // first frame is always a key frame
    p->fillRect(LEFT_STRUT,y,KEY_WIDTH-1,KEY_HEIGHT,QBrush(Qt::black));

    for(int key=0;key<numKeyFrames;key++)
    {
      int frameNum=keyFrames[key];
      if(frameNum<numOfFrames)
      {
        int xpos=frameNum*KEY_WIDTH+LEFT_STRUT;
        p->fillRect(xpos,y,KEY_WIDTH-1,KEY_HEIGHT,QBrush(Qt::black));
        if(frameNum>0 && frameNum!=(numOfFrames-1))
          p->drawLine(LEFT_STRUT,y+KEY_HEIGHT/2,xpos,y+KEY_HEIGHT/2);
      }
    } // for

    // last frame is always a key frame
    p->fillRect((numOfFrames-1)*KEY_WIDTH+LEFT_STRUT,y,KEY_WIDTH-1,KEY_HEIGHT,QBrush(Qt::black));
  }
  drawPosition();
}

void Timeline::mousePressEvent(QMouseEvent* e)
{
  // get track and frame based on mouse coordinates
  trackSelected=e->y()/LINE_HEIGHT+1;
  frameSelected=(e->x()-LEFT_STRUT)/KEY_WIDTH;

  // set animation frame to where we clicked
  animation->setFrame(frameSelected);
  // check if we clicked on a key frame which is not the first or last one in animation
  if(animation->isKeyFrame(animation->getPartName(trackSelected)) &&
     frameSelected>0 && frameSelected<(numOfFrames-1)) dragging=true;
  // remember mouse button state
  leftMouseButton=true;
}

void Timeline::mouseReleaseEvent(QMouseEvent*)
{
  leftMouseButton=false;
  dragging=false;
}

void Timeline::mouseMoveEvent(QMouseEvent* e)
{
  // calculate new position (in frames)
  int frame=(e->x()-LEFT_STRUT)/KEY_WIDTH;
  // special treatment if user is dragging a keyframe
  if(dragging)
  {
    // if user is dragging a keyframe, move it within more restrictive bounds
    if(frame>0 && frame<(numOfFrames-1))
    {
      animation->moveKeyframe(trackSelected,frameSelected,frame);
      // remember new position
      frameSelected=frame;
    }
  }
  // no dragging so check if new position would be out of bounds or has changed at all
  else if(frame>=0 && frame<numOfFrames && frame!=frameSelected)
  {
    // set the new position
    animation->setFrame(frame);
    // remember new position
    frameSelected=frame;
  }
}
