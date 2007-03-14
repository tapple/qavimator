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

#include <qscrollview.h>
#include <qlayout.h>
#include <qpainter.h>

#include "timelineview.h"
#include "timeline.h"
#include "animation.h"

TimelineView::TimelineView(QWidget* parent,const char* name,WFlags f) : QFrame(parent,name,f)
{
  QHBoxLayout* layout=new QHBoxLayout(this);

  layout->setAutoAdd(TRUE);

  timelineTracks=new TimelineTracks(this,"timeline_tracks");

  view=new QScrollView(this,"timeline_inner_view",WNoAutoErase|WStaticContents);

  view->setVScrollBarMode(QScrollView::AlwaysOff);
//  view->setHScrollBarMode(QScrollView::AlwaysOff);

  timeline=new Timeline(view->viewport(),"timeline");

  connect(timeline,SIGNAL(resized(const QSize&)),this,SLOT(doResize(const QSize&)));
  connect(timeline,SIGNAL(animationChanged(Animation*)),this,SLOT(setAnimation(Animation*)));
  connect(timeline,SIGNAL(trackClicked(int)),this,SLOT(selectTrack(int)));

  view->addChild(timeline);
}

TimelineView::~TimelineView()
{
}

void TimelineView::scrollTo(int x)
{
  view->ensureVisible(x,0,view->visibleWidth()*1/4,view->visibleHeight());
}

void TimelineView::doResize(const QSize& newSize)
{
  view->resizeContents(newSize.width(),newSize.height());
}

void TimelineView::setAnimation(Animation* anim)
{
  timelineTracks->setAnimation(anim);
}

Timeline* TimelineView::getTimeline() const
{
  return timeline;
}

void TimelineView::selectTrack(int track)
{
  timeline->selectTrack(track);
  timelineTracks->selectTrack(track);
}

void TimelineView::backgroundClicked()
{
  timeline->selectTrack(0);
  timelineTracks->selectTrack(0);
}

// --------------

TimelineTracks::TimelineTracks(QWidget* parent,const char* name,WFlags f) : QWidget(parent,name,f)
{
  resize(sizeHint());
}

TimelineTracks::~TimelineTracks()
{
  selectedTrack=0;
}

QSize TimelineTracks::sizeHint() const
{
  return QSize(LEFT_STRUT,(NUM_PARTS-2)*LINE_HEIGHT+2);
}

void TimelineTracks::paintEvent(QPaintEvent*)
{
  repaint();
}

void TimelineTracks::drawTrack(int track)
{
  if(track==0) return;

  QString trackName=(animation) ? animation->getPartName(track) : "";
  if(trackName!="Site")
  {
    QPainter p(this);

    int y=(track-1)*LINE_HEIGHT+2;

    if(track==selectedTrack)
      p.fillRect(0,y,width(),LINE_HEIGHT,palette().color(QPalette::Active,QColorGroup::Highlight));
    else
      p.eraseRect(0,y,width(),LINE_HEIGHT);

    // draw track name
    p.setPen(palette().color(QPalette::Active,QColorGroup::Foreground));
    p.drawText(0,y+KEY_HEIGHT,trackName);
  }
}

void TimelineTracks::repaint()
{
  resize(sizeHint());
  if(!animation) return;

  for(int part=1;part<NUM_PARTS;part++) drawTrack(part);
}

void TimelineTracks::setAnimation(Animation* anim)
{
  animation=anim;
}

void TimelineTracks::selectTrack(int track)
{
  int oldTrack=selectedTrack;
  selectedTrack=track;
  drawTrack(oldTrack);
  drawTrack(track);
}
