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

#include <qapplication.h>

#include "bvhnode.h"

BVHNode::BVHNode()
{
//  keyframes.setAutoDelete(true);
  keyframes.clear();
  children.clear();

  numChannels=0;
  numKeyFrames=0;

  ikRot[0]=0;
  ikRot[1]=0;
  ikRot[2]=0;

  ikGoalPos[0]=0;
  ikGoalPos[1]=0;
  ikGoalPos[2]=0;

  ikGoalDir[0]=0;
  ikGoalDir[1]=1;
  ikGoalDir[2]=0;
}

BVHNode::~BVHNode()
{
}

const QString& BVHNode::name() const
{
  return m_name;
}

void BVHNode::setName(const QString& newName)
{
  m_name=newName;
}

int BVHNode::numChildren() const
{
  return children.count();
}

BVHNode* BVHNode::child(int num)
{
  return children.at(num);
}

void BVHNode::addChild(BVHNode* newChild)
{
  children.append(newChild);
}

void BVHNode::addKeyframe(int frame,Position pos,Rotation rot)
{
//  FrameData(frame,pos,rot);
  keyframes[frame]=FrameData(frame,pos,rot); // .append(key);
}

// ************************************************************************

FrameData::FrameData()
{
  qDebug(QString("FrameData(%1)").arg((unsigned long)this));
  frameNumber=0;
  easeIn=false;
  easeOut=false;
}

FrameData::FrameData(int frame,Position pos,Rotation rot)
{
  qDebug(QString("FrameData(%1): frame %2  pos %3,%4,%5 rot %6,%7,%8").arg((unsigned long) this).arg(frame).arg(pos.x).arg(pos.y).arg(pos.z).arg(rot.x).arg(rot.y).arg(rot.z));
  frameNumber=frame;
  easeIn=false;
  easeOut=false;
}

FrameData::~FrameData()
{
  qDebug(QString("~FrameData(%1)").arg((unsigned long) this));
}
