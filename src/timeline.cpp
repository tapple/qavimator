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

Timeline::Timeline(QWidget *parent, const char *name)
 : QWidget(parent, name)
{
  animation=0;
  setCurrentFrame(0);
  setCaption(tr("Timeline"));

  leftMouseButton=false;
  shift=false;

  dragging=0;
  trackSelected=0;

  tracks.clear();
  setFocusPolicy(QWidget::StrongFocus);
}

Timeline::~Timeline()
{
}

void Timeline::paintEvent(QPaintEvent*)
{
//  qDebug(QString("repaint %1").arg(e->rect().x()));
  repaint();
}

void Timeline::repaint()
{
  if(isHidden()) return;
  if(!animation) return;

  QSize newSize=QSize(numOfFrames*KEY_WIDTH,(NUM_PARTS-2)*LINE_HEIGHT);

  resize(newSize);
  emit resized(newSize);

  for(int part=1;part<NUM_PARTS;part++)
  {
    drawTrack(part);
  } // for
  drawPosition();
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
  clearPosition();
  frameSelected=frame;
  drawPosition();
}

void Timeline::setAnimation(Animation* anim)
{
  // don't do anything if this is the same animation as before
  if(anim==animation) return;

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

  emit animationChanged(anim);

  repaint();
}

void Timeline::clearPosition()
{
  if(!animation) return;

  QPainter p(this);

  for(int track=1;track<NUM_PARTS;track++)
  {
    int x=frameSelected*KEY_WIDTH;
    int y=(track-1)*LINE_HEIGHT;
    int light=(frameSelected/10) % 2;

    p.fillRect(x,y,KEY_WIDTH,LINE_HEIGHT,palette().color(QPalette::Active,QColorGroup::Background).dark(100+5*light));

    QString trackName=animation->getPartName(track);
    if(trackName!="Site")
    {
      if(isKeyFrame(track,frameSelected)) drawKeyframe(track,frameSelected);
      else
      {
        QColor pen;
        if(!animation->compareFrames(trackName,previousKeyFrame(track,frameSelected),nextKeyFrame(track,frameSelected)))
          pen=palette().color(QPalette::Active,QColorGroup::Foreground);
        else
          pen=palette().color(QPalette::Active,QColorGroup::Background).dark(115);

        // using fillRect instead of drawLine here, because the windows build seems to
        // have problems with lines
        p.fillRect(x,y+LINE_HEIGHT/2,KEY_WIDTH,1,QColor(pen));
      }
    }
  } // for
}

void Timeline::drawPosition()
{
  emit positionCenter(frameSelected*KEY_WIDTH);

  QPainter p(this);
//  p->setRasterOp(Qt::XorROP);
  p.setPen(QColor("#ff0000"));
//  p->setPen(QColor("#00ffff"));

  int xpos=frameSelected*KEY_WIDTH+KEY_WIDTH/2;
  p.drawLine(xpos,0,xpos,height());

//  p->setRasterOp(Qt::CopyROP);
}

void Timeline::addKeyframe(int track,int frame)
{
  if(tracks.find(track)==tracks.end())
  {
    tracks[track]=KeyframeList();
  }
  tracks[track][frame]=1;

  drawTrack(track);
  drawPosition();
}

void Timeline::removeKeyframe(int track,int frame)
{
  tracks[track].erase(frame);

  if(!tracks[track].count()) tracks.erase(track);

  drawTrack(track);
  drawPosition();
}

void Timeline::drawKeyframe(int track,int frame)
{
  int ypos=(track-1)*LINE_HEIGHT;

  QPainter p(this);
  // calculate x position
  int xpos=frame*KEY_WIDTH;

//  QColor color(Qt::black);
  QColor color(palette().color(QPalette::Active,QColorGroup::Foreground));
  // draw the key frame
  if(frame==0)
  {
    p.fillRect(xpos,ypos,KEY_WIDTH/2,KEY_HEIGHT,QBrush(color));
    p.fillRect(xpos,ypos+LINE_HEIGHT/2-2,KEY_WIDTH,4,QBrush(color));
  }
  else if(frame==(numOfFrames-1))
  {
    p.fillRect(xpos+KEY_WIDTH/2,ypos,KEY_WIDTH/2,KEY_HEIGHT,QBrush(color));
    p.fillRect(xpos,ypos+LINE_HEIGHT/2-2,KEY_WIDTH,4,QBrush(color));
  }
  else
  {
    // find out if we should paint in highlight or normal color
    if(frame && dragging==frame && trackSelected==track) color=Qt::red;

    QString trackName=animation->getPartName(track);

    bool ps=animation->compareFrames(trackName,previousKeyFrame(track,frame),frame);
    bool ns=animation->compareFrames(trackName,nextKeyFrame(track,frame),frame);

    p.setPen(color);
    p.setBrush(QBrush(color));

    if(ps && ns)
      p.drawEllipse(xpos,ypos,KEY_WIDTH,LINE_HEIGHT);

    else if(ns)
    {
      QPointArray triangle(3);
      triangle.putPoints(0,3, xpos,ypos+(LINE_HEIGHT-1)/2, xpos+KEY_WIDTH-1,ypos, xpos+KEY_WIDTH-1,ypos+LINE_HEIGHT-1);

      p.drawPolygon(triangle);
    }
    else if(ps)
    {
      QPointArray triangle(3);
      triangle.putPoints(0,3, xpos+KEY_WIDTH-1,ypos+(LINE_HEIGHT-1)/2, xpos,ypos, xpos,ypos+LINE_HEIGHT-1);

      p.drawPolygon(triangle);
    }
    else
      p.fillRect(xpos,ypos,KEY_WIDTH-1,KEY_HEIGHT,QBrush(color));
  }
}

int Timeline::isKeyFrame(int track,int frame)
{
  // first and last frames are always key frames
  if(frame==0 || frame==(numOfFrames-1)) return true;

  const int* keyFrames=animation->keyFrames(track);
  int numOfKeyFrames=animation->numKeyFrames(track);
  for(int index=0;index<numOfKeyFrames;index++)
  {
    int lookFrame=keyFrames[index];
    if(lookFrame==frame) return true;
  }
  return false;
}

int Timeline::previousKeyFrame(int track,int frame)
{
  int previous=0;
  const int* keyFrames=animation->keyFrames(track);
  int numOfKeyFrames=animation->numKeyFrames(track);
  for(int index=0;index<numOfKeyFrames;index++)
  {
    int lookFrame=keyFrames[index];
    if(lookFrame>=frame) return previous;
    previous=lookFrame;
  }
  return previous;
}

int Timeline::nextKeyFrame(int track,int frame)
{
  const int* keyFrames=animation->keyFrames(track);
  int numOfKeyFrames=animation->numKeyFrames(track);
  for(int index=0;index<numOfKeyFrames;index++)
  {
    int lookFrame=keyFrames[index];
    if(lookFrame>frame && index!=numOfKeyFrames) return keyFrames[index];
  }
  return numOfFrames-1;
}

void Timeline::drawTrack(int track)
{
  QPainter p(this);

  int light=0;
  int y=(track-1)*LINE_HEIGHT;

  // draw alternatingly light / dark background
  for(int x=0;x<width();x+=10*KEY_WIDTH)
  {
    // draw a filled rectangle in background color, alternatingly darkened by 5 %
    p.fillRect(x,y,x+10*KEY_WIDTH,LINE_HEIGHT,palette().color(QPalette::Active,QColorGroup::Background).dark(100+5*light));
    light=1-light;
  }

  // if this is a "Site end" marker (end of group of limbs) do nothing more
  QString trackName=animation->getPartName(track);
  if(trackName=="Site") return;

  // draw straight line as track marker
  p.fillRect(0,y+LINE_HEIGHT/2,width(),1,palette().color(QPalette::Active,QColorGroup::Background).dark(115));

  // get number of key frames in this track
  const int numKeyFrames=animation->numKeyFrames(track);

  // TODO: switch over to something like this
  /*  KeyframeList keyFrames=tracks[track];
  const int numKeyFrames=keyFrames.count();*/

  // first frame is always a key frame
  drawKeyframe(track,0);

  // start drawing rest of key frames
  if(numKeyFrames)
  {
    // get foreground color
    QColor color(palette().color(QPalette::Active,QColorGroup::Foreground));
    // set drawing pen to foreground color
    p.setPen(color);
    // get the list of key frames
    const int* keyFrames=animation->keyFrames(track);

    // reset previous frame marker
    int oldFrame=0;
    // iterate through list of key frames
    for(int key=0;key<numKeyFrames;key++)
    {
      // get frame number of key frame
      int frameNum=keyFrames[key];
      // if key frame is not out of animation length
      if(frameNum<numOfFrames)
      {
        // if this key frame is not at the first animation frame
        if(frameNum>0)
        {
          // check if it differs from the previous frame, if it does, draw a line there
          if(!animation->compareFrames(trackName,frameNum,oldFrame))
          {
            // we use fillRect instead of drawLine because for some reason the windows
            // version did not draw properly
            p.fillRect(oldFrame*KEY_WIDTH+KEY_WIDTH-1,
                       y+KEY_HEIGHT/2,
                       (frameNum-oldFrame)*KEY_WIDTH,
                       1,QBrush(color));
          }
        }
        // draw the key frame
        drawKeyframe(track,frameNum);
      }
      // remember this frame number as previous key frame
      oldFrame=frameNum;
    } // for

    // last frame is always a key frame
    drawKeyframe(track,numOfFrames-1);
  }
}

void Timeline::mousePressEvent(QMouseEvent* e)
{
  clearPosition();

  // get track and frame based on mouse coordinates
  trackSelected=e->y()/LINE_HEIGHT+1;
  frameSelected=e->x()/KEY_WIDTH;

  // set animation frame to where we clicked
  animation->setFrame(frameSelected);
  // check if we clicked on a key frame which is not the first or last one in animation
  if(frameSelected>0 && frameSelected<(numOfFrames-1) && isKeyFrame(trackSelected,frameSelected))
  {
    // first switch on dragging state, so drawKeyframe() will work as expected
    dragging=frameSelected;
    // highlight keyframe
    drawKeyframe(trackSelected,frameSelected);
  }
  emit trackClicked(trackSelected);
  // remember mouse button state
  leftMouseButton=true;
  drawPosition();
/*
  int p=previousKeyFrame(trackSelected,frameSelected);
  int n=nextKeyFrame(trackSelected,frameSelected);
  int k=isKeyFrame(trackSelected,frameSelected);
  qDebug(QString("prev of %1 is %2 (%3)").arg(frameSelected).arg(p).arg(k));
  qDebug(QString("next of %1 is %2 (%3)").arg(frameSelected).arg(n).arg(k));
*/
}

void Timeline::mouseReleaseEvent(QMouseEvent*)
{
  leftMouseButton=false;
  if(dragging)
  {
    // first switch off dragging state, so drawKeyframe() will work as expected
    dragging=0;
    // remove highlight from keyframe
    drawKeyframe(trackSelected,frameSelected);
    drawPosition();
  }
}

void Timeline::mouseMoveEvent(QMouseEvent* e)
{
  // calculate new position (in frames)
  int frame=e->x()/KEY_WIDTH;

  // if frame position has not changed, do nothing
  if(frame==frameSelected) return;

  // special treatment if user is dragging a keyframe
  if(dragging)
  {
    // if user is dragging a keyframe, move it within more restrictive bounds
    if(frame>0 && frame<(numOfFrames-1))
    {
      // update dragging position
      dragging=frame;

      if(shift)
      {
        shift=false;
        animation->copyKeyFrame(trackSelected,frameSelected,frame);
      }
      else
        animation->moveKeyFrame(trackSelected,frameSelected,frame);

      // remember new position
      frameSelected=frame;
    }
  }
  // no dragging so check if new position would be out of bounds
  else if(frame>=0 && frame<numOfFrames)
  {
    // set the new position
    animation->setFrame(frame);
    // remember new position
    frameSelected=frame;
  }
}

void Timeline::wheelEvent(QWheelEvent* e)
{
  // get curernt frame position
  int newFrame=frameSelected;

  // check if we should move forward or backwards
  if(e->delta()>0) newFrame-=5;
  else newFrame+=5;

  // check if new position would be out of bounds
  if(newFrame>=numOfFrames) newFrame=numOfFrames-1;
  else if(newFrame<0) newFrame=0;

  // set the new position
  animation->setFrame(newFrame);
  // remember new position
  frameSelected=newFrame;
}

void Timeline::keyPressEvent(QKeyEvent* e)
{
  if(e->key()==Qt::Key_Shift) shift=true;
  e->ignore();
}

void Timeline::keyReleaseEvent(QKeyEvent* e)
{
  if(e->key()==Qt::Key_Shift) shift=false;
  e->ignore();
}
