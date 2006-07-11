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

#ifndef TIMELINE_H
#define TIMELINE_H

#include <qwidget.h>

#include "keyframelist.h"

/**
  @author Zi Ree
*/

class Animation;

class Timeline : public QWidget
{
  Q_OBJECT

  public:
    Timeline(QWidget *parent = 0, const char *name = 0);
    ~Timeline();

    virtual void repaint();

    void setAnimation(Animation* anim);

  signals:
    void positionCenter(int pos);

  public slots:
    void setCurrentFrame(int frame);

  protected slots:
    void addKeyframe(int bodypart,int frame);
    void removeKeyframe(int track,int frame);
    void setNumberOfFrames(int frames);

  protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);

    void drawPosition();
    void drawKeyframe(int track,int frame);
    void drawTrack(int track);

    QPainter* p;
    Animation* animation;
    int numOfFrames;
    int currentFrame;
    bool drawn;
    bool leftMouseButton;
    int trackSelected;
    int frameSelected;

    QMap<int,KeyframeList> tracks;
};

#endif
