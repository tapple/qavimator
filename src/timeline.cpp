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
#include <qtimer.h>

#include "timeline.h"
#include "animation.h"
#include "keyframelist.h"

Timeline::Timeline(QWidget *parent, const char *name, WFlags)
  : QWidget(parent, name)
{
  resize(50,50);

  positionSync=false;
  positionBarX=0;
  backgroundBuffer.resize(KEY_WIDTH,50);

  animation=0;
  setCurrentFrame(0);
  setCaption(tr("Timeline"));

  leftMouseButton=false;
  shift=false;

  dragging=0;
  trackSelected=0;

  setFocusPolicy(QWidget::StrongFocus);
}

Timeline::~Timeline()
{
}

void Timeline::paintEvent(QPaintEvent* e)
{
  if(!animation) return;

  clearPosition();

  if(isHidden()) return;

  QSize newSize=QSize(numOfFrames*KEY_WIDTH,(NUM_PARTS-2)*LINE_HEIGHT);

  if(newSize!=size())
  {
    qDebug(QString("resizing timeline widget to %1x%1").arg(newSize.width()).arg(newSize.height()));
    backgroundBuffer.resize(KEY_WIDTH,newSize.height());
    resize(newSize);
    emit resized(newSize);
  }

  // clip keyframe drawing to "dirty" area of redraw
  firstVisibleKeyX=e->rect().x()/KEY_WIDTH;
  visibleKeysX=e->rect().width()/KEY_WIDTH;

  for(int part=1;part<NUM_PARTS;part++)
  {
    drawTrack(part);
  } // for

  // reset keyframe clipping to have new or updated transition lines painted properly
  firstVisibleKeyX=0;
  visibleKeysX=numOfFrames;

// draw updated rectangle as debug info
// QPainter p(this);
// p.drawRect(e->rect());

  // redraw position as soon as repaint event has ended, because inside this event, all painting is
  // clipped to e->region()
  QTimer::singleShot(0,this,SLOT(drawPosition()));
}

void Timeline::setNumberOfFrames(int frames)
{
  numOfFrames=frames;
  repaint();
}

void Timeline::setCurrentFrame(int frame)
{
  clearPosition();
  frameSelected=frame;
  emit positionCenter(frameSelected*KEY_WIDTH);
  drawPosition();
}

void Timeline::setAnimation(Animation* anim)
{
  // don't do anything if this is the same animation as before
  if(anim==animation) return;

  if(animation)
  {
    disconnect(animation,SIGNAL(numberOfFrames(int)),this,SLOT(setNumberOfFrames(int)));
    disconnect(animation,SIGNAL(redrawTrack(int)),this,SLOT(redrawTrack(int)));
    disconnect(this,SIGNAL(deleteKeyframe(int,int)),animation,SLOT(delKeyFrame(int,int)));
    disconnect(this,SIGNAL(insertFrame(int,int)),animation,SLOT(insertFrame(int,int)));
    numOfFrames=0;
  }
  animation=anim;

  if(animation)
  {
    connect(animation,SIGNAL(numberOfFrames(int)),this,SLOT(setNumberOfFrames(int)));
    connect(animation,SIGNAL(redrawTrack(int)),this,SLOT(redrawTrack(int)));
    connect(this,SIGNAL(deleteKeyframe(int,int)),animation,SLOT(delKeyFrame(int,int)));
    connect(this,SIGNAL(insertFrame(int,int)),animation,SLOT(insertFrame(int,int)));
    numOfFrames=animation->getNumberOfFrames();
  }

  emit animationChanged(anim);

  repaint();
}

void Timeline::clearPosition()
{
  if(!animation) return;

  if(!positionSync) // debug
  {
    qDebug("position bar has not been drawn, but asked to clear it!");
    return;
  }
  positionSync=false;

  bitBlt(this,positionBarX,0,&backgroundBuffer,0,0,KEY_WIDTH,height());
}

void Timeline::drawPosition()
{
  if(!animation) return;

  if(positionSync)
  {
    qDebug("position bar has not been cleared, but asked to draw it!");
    return;
  }
  positionSync=true;    // debug

  QPainter p(this);
  int xpos=frameSelected*KEY_WIDTH;

  bitBlt(&backgroundBuffer,0,0,this,xpos,0,KEY_WIDTH,height());
  positionBarX=xpos;

  p.setPen(QColor("#ff0000"));
  p.drawRect(xpos+KEY_WIDTH/2-1,0,2,height());
}

void Timeline::redrawTrack(int track)
{
  clearPosition();
  drawTrack(track);
  drawPosition();
}

void Timeline::drawKeyframe(int track,int frame)
{
//  qDebug(QString("drawKeyframe(%1,%2)").arg(track).arg(frame));
  // track==0 means an BVH_END track or something else that went wrong
  if(!track) return;

  int ypos=(track-1)*LINE_HEIGHT;

  QPainter p(this);
  // calculate x position
  int xpos=frame*KEY_WIDTH;

  QColorGroup::ColorRole keyColor=QColorGroup::Foreground;

#ifdef Q_OS_WIN32
  // on windows systems use text highlight color to contrast with track highlight
  if(trackSelected==track)
    keyColor=QColorGroup::HighlightedText;
#endif

  QColor color(palette().color(QPalette::Active,keyColor));
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

    // get the list of key frames
    const BVHNode* joint=animation->getNode(track);

    bool ps=animation->compareFrames(joint->name(),joint->getKeyframeBefore(frame).frameNumber(),frame);
    bool ns=animation->compareFrames(joint->name(),joint->getNextKeyframe(frame).frameNumber(),frame);

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

void Timeline::selectTrack(int track)
{
  int oldTrack=trackSelected;
  trackSelected=track;
  // if the user clicked on a different track
  if(oldTrack!=trackSelected)
  {
    clearPosition();
    // draw old track with old color
    drawTrack(oldTrack);
    // draw new track
    drawTrack(trackSelected);
    drawPosition();
  }
}

void Timeline::drawTrack(int track)
{
  // do not draw track number 0
  if(!track) return;
//  qDebug("first %d vis %d",firstVisibleKeyX,visibleKeysX);
//  qDebug(QString("drawTrack(%1)").arg(track));

  // if this is a "Site end" marker (end of group of limbs) do nothing more
  QString trackName=animation->getPartName(track);

  // normal tracks get default background color
  QColorGroup::ColorRole baseColor=QColorGroup::Background;
  // normal tracks get default foreground colors for keyframe transition lines
  QColorGroup::ColorRole lineColor=QColorGroup::Foreground;
  // selected tracks get other color, unless it's a "Site" track (end of limbs group)
  if(track==trackSelected && trackName!="Site")
  {
    baseColor=QColorGroup::Highlight;
#ifdef Q_OS_WIN32
    // additionally, set inverse highlight color for transition lines on windows for contrast
    lineColor=QColorGroup::HighlightedText;
#endif
  }

  QPainter p(this);

  int light=0;
  int y=(track-1)*LINE_HEIGHT;

  // find widget coordinates of clipped area and decide if we paint light or dark background
  int firstGreyShade=(firstVisibleKeyX/10);
  light=firstGreyShade % 2;
  firstGreyShade*=KEY_WIDTH*10;

  // draw alternatingly light / dark background
  for(int x=firstGreyShade;x<(firstGreyShade+visibleKeysX+10)*KEY_WIDTH;x+=10*KEY_WIDTH)
  {
    // draw a filled rectangle in background color, alternatingly darkened by 5 %
    p.fillRect(x,y,x+10*KEY_WIDTH,LINE_HEIGHT,palette().color(QPalette::Active,baseColor).dark(100+5*light));
    light=1-light;
  }

  // if this is a "Site end" marker (end of group of limbs) do nothing more
  if(trackName=="Site") return;

  // draw straight line as track marker within "dirty" redraw region
  p.fillRect(firstVisibleKeyX*KEY_WIDTH,y+LINE_HEIGHT/2,(visibleKeysX+1)*KEY_WIDTH,1,palette().color(QPalette::Active,baseColor).dark(115));
  p.fillRect(firstVisibleKeyX*KEY_WIDTH,y+LINE_HEIGHT/2+1,(visibleKeysX+1)*KEY_WIDTH,1,palette().color(QPalette::Active,baseColor).light(115));

  // get the list of key frames
  const BVHNode* joint=animation->getNode(track);
  // start drawing rest of key frames
  if(joint->numKeyframes())
  {
    // get foreground color
    QColor color(palette().color(QPalette::Active,QColorGroup::Foreground));
    // set drawing pen to foreground color
    p.setPen(color);

    // reset previous frame marker
    int oldFrame=0;
    // iterate through list of key frames
    for(int key=0;key<joint->numKeyframes();key++)
    {
      // get frame number of key frame
      const FrameData& frameData=joint->keyframeDataByIndex(key);
      int frameNum=frameData.frameNumber();

      // if key frame is not out of animation length
      if(frameNum<numOfFrames)
      {
        // if this key frame is not at the first animation frame
        if(frameNum>0)
        {
          // check if it differs from the previous frame, if it does, draw a line there
          if(!animation->compareFrames(trackName,frameNum,oldFrame))
          {
            const FrameData& oldFrameData=joint->keyframeDataByIndex(key-1);
            int frameDiff=(frameNum-oldFrame+1)*KEY_WIDTH/2;

            QColor pen1(palette().color(QPalette::Active,lineColor));
            QColor pen2(palette().color(QPalette::Active,baseColor).light(115));

            if(oldFrameData.easeOut())
            {
              QPoint oldPos(oldFrame*KEY_WIDTH+KEY_WIDTH-1,y+LINE_HEIGHT/2);
              for(int x=0;x<frameDiff-KEY_WIDTH+1;x++)
              {
                QPoint newPos(x+oldFrame*KEY_WIDTH+KEY_WIDTH-1,
                              (int)(y+sin(x*6.283/(frameDiff-KEY_WIDTH+1))*(LINE_HEIGHT/2-1)+LINE_HEIGHT/2)
                             );
                p.setPen(pen1);
                p.drawLine(oldPos,newPos);
                p.setPen(pen2);
                p.drawLine(oldPos+QPoint(0,1),newPos+QPoint(0,1));
                oldPos=newPos;
              }
            }
            else
            {
              // we use fillRect instead of drawLine because for some reason the windows
              // version did not draw properly
              p.fillRect(oldFrame*KEY_WIDTH+KEY_WIDTH-1,
                         y+KEY_HEIGHT/2,
                         frameDiff-KEY_WIDTH,
                         1,QBrush(pen1));
              p.fillRect(oldFrame*KEY_WIDTH+KEY_WIDTH-1,
                         y+KEY_HEIGHT/2+1,
                         frameDiff,
                         1,QBrush(pen2));
            }

            if(frameData.easeIn())
            {
              QPoint oldPos(oldFrame*KEY_WIDTH+frameDiff,y+LINE_HEIGHT/2);
              for(int x=0;x<frameDiff-KEY_WIDTH+1;x++)
              {
                QPoint newPos(x+oldFrame*KEY_WIDTH+frameDiff-1,
                              (int)(y+sin(x*6.283/(frameDiff-KEY_WIDTH+1))*(LINE_HEIGHT/2-1)+LINE_HEIGHT/2)
                             );
                p.setPen(pen1);
                p.drawLine(oldPos,newPos);
                p.setPen(pen2);
                p.drawLine(oldPos+QPoint(0,1),newPos+QPoint(0,1));
                oldPos=newPos;
              }
            }
            else
            {
              p.fillRect(oldFrame*KEY_WIDTH+frameDiff-1,
                         y+KEY_HEIGHT/2,
                         frameDiff,
                         1,QBrush(pen1));
              p.fillRect(oldFrame*KEY_WIDTH+frameDiff-1,
                         y+KEY_HEIGHT/2+1,
                         frameDiff,
                         1,QBrush(pen2));
            }
          }
        }
        // only draw keyframes if they are in "dirty" clipping region
        if(frameNum>=firstVisibleKeyX && frameNum<=(firstVisibleKeyX+visibleKeysX))
          // draw the key frame
          drawKeyframe(track,frameNum);
      }
      // remember this frame number as previous key frame
      oldFrame=frameNum;
    } // for

    // last frame is always a key frame, so draw it if it's inside the clipping region
    if(firstVisibleKeyX+visibleKeysX>=(numOfFrames-1)) drawKeyframe(track,numOfFrames-1);
  }
}

void Timeline::mousePressEvent(QMouseEvent* e)
{
  // get track and frame based on mouse coordinates
  selectTrack(e->y()/LINE_HEIGHT+1);
  frameSelected=e->x()/KEY_WIDTH;

  // set animation frame to where we clicked
  animation->setFrame(frameSelected);
  // check if we clicked on a key frame which is not the first or last one in animation
  if(frameSelected>0 && frameSelected<(numOfFrames-1) && animation->isKeyFrame(trackSelected,frameSelected))
  {
    // first switch on dragging state, so drawKeyframe() will work as expected
    dragging=frameSelected;
    // highlight keyframe
    drawKeyframe(trackSelected,frameSelected);
  }
  emit trackClicked(trackSelected);
  // remember mouse button state
  leftMouseButton=true;
}

void Timeline::mouseReleaseEvent(QMouseEvent*)
{
  clearPosition();
  leftMouseButton=false;
  if(dragging)
  {
    // first switch off dragging state, so drawKeyframe() will work as expected
    dragging=0;
    // remove highlight from keyframe
    drawKeyframe(trackSelected,frameSelected);
  }
  drawPosition();
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

      // make sure we don't overwrite already existing frames
      if(!animation->isKeyFrame(trackSelected,frame))
      {
        // check if the user holds the shift key
        if(shift)
        {
          // reset shift key status so we don't constantly create copies
          shift=false;
          // copy selected frame to new place
          animation->copyKeyFrame(trackSelected,frameSelected,frame);
        }
        else
          // move selected frame to new place
          animation->moveKeyFrame(trackSelected,frameSelected,frame);

        // remember new position
        frameSelected=frame;
      }
    }
  }
  // no dragging so check if new position would be out of bounds
  else if(frame>=0 && frame<numOfFrames)
  {
    // remember new position
    frameSelected=frame;
    // set the new position
    animation->setFrame(frame);
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

  // remember new position
  frameSelected=newFrame;
  // set the new position
  animation->setFrame(newFrame);
}

void Timeline::keyPressEvent(QKeyEvent* e)
{
  switch(e->key())
  {
    case Qt::Key_Shift:
      shift=true;
      break;
    case Qt::Key_Delete:
      emit deleteKeyframe(trackSelected,frameSelected);
      break;
    case Qt::Key_Insert:
      emit insertFrame(trackSelected,frameSelected);
      if(trackSelected)
      {
        clearPosition();
        drawTrack(trackSelected);
        drawPosition();
      }
      else repaint();
      break;
    default:
      e->ignore();
      return;
  }
  e->accept();
}

void Timeline::keyReleaseEvent(QKeyEvent* e)
{
  switch(e->key())
  {
    case Qt::Key_Shift:
      shift=false;
      break;
    default:
      e->ignore();
      return;
  }
  e->accept();
}
