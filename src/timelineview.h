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

#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <qframe.h>

/**
  @author Zi Ree
*/

class QScrollView;
class Timeline;
class Animation;

class TimelineTracks : public QWidget
{
  Q_OBJECT

  public:
    TimelineTracks(QWidget* parent=0,const char* name=0,WFlags f=0);
    ~TimelineTracks();

    virtual void repaint();
    virtual QSize sizeHint() const;

    void drawTrack(int track);
    void setAnimation(Animation* anim);

    void selectTrack(int track);

  protected:
    Animation* animation;
    virtual void paintEvent(QPaintEvent* event);

    int selectedTrack;
};

class TimelineView : public QFrame
{
  Q_OBJECT

  public:
    TimelineView(QWidget* parent=0,const char* name=0,WFlags f=0);
    ~TimelineView();

    Timeline* getTimeline() const;

  public slots:
    void selectTrack(int track);
    void backgroundClicked();

  protected slots:
    void scrollTo(int x);
    void doResize(const QSize& newSize);
    void setAnimation(Animation* anim);

  protected:
    QScrollView* view;
    Timeline* timeline;
    TimelineTracks* timelineTracks;
};

#endif
