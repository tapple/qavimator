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

BVHNode::BVHNode(const QString& name)
{
//  qDebug(QString("BVHNode::BVHNode(%1)").arg(name));
  setName(name);

  // clean out lists
  keyframes.clear();
  children.clear();

  // have clean one-time cache
  flushFrameCache();
  // make sure all rotations and positions from the one-time cache get deleted on .clear()
  rotations.setAutoDelete(true);
  positions.setAutoDelete(true);

  numChannels=0;

  ikRot.x=0;
  ikRot.y=0;
  ikRot.z=0;

  ikGoalPos[0]=0;
  ikGoalPos[1]=0;
  ikGoalPos[2]=0;

  ikGoalDir[0]=0;
  ikGoalDir[1]=1;
  ikGoalDir[2]=0;
}

BVHNode::~BVHNode()
{
//  qDebug(QString("BVHNode::~BVHNode(%1)").arg(name()));
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
//  qDebug(QString("addKeyframe(%1)").arg(frame));
  keyframes[frame]=FrameData(frame,pos,rot);
//  if(frame==0 && name()=="hip") qDebug(QString("BVHNode::addKeyframe(%1,<%2,%3,%4>,<%5,%6,%7>) %8").arg(frame).arg(pos.x).arg(pos.y).arg(pos.z).arg(rot.x).arg(rot.y).arg(rot.z).arg(pos.bodyPart));
}

void BVHNode::setKeyframePosition(int frame,const Position& pos)
{
//  qDebug(QString("setKeyframePosition(%1)").arg(frame));
  if(!isKeyframe(frame)) qDebug(QString("setKeyframePosition(%1): not a keyframe!").arg(frame));
  else
  {
    FrameData& key=keyframes[frame];
    key.setPosition(pos);
  }
}

void BVHNode::setKeyframeRotation(int frame,const Rotation& rot)
{
//  qDebug(QString("setKeyframeRotation(%1)").arg(frame));
  if(!isKeyframe(frame)) qDebug(QString("setKeyframeRotation(%1): not a keyframe!").arg(frame));
  else
  {
    FrameData& key=keyframes[frame];
    key.setRotation(rot);
  }
}

void BVHNode::deleteKeyframe(int frame)
{
  keyframes.erase(frame);
}

bool BVHNode::isKeyframe(int frame) const
{
  return keyframes.contains(frame);
}

double BVHNode::interpolate(double from,double to,int steps,int pos,bool /* type */) const
{
//  qDebug(QString("interpolate %1 %2 %3 %4").arg(from).arg(to).arg(steps).arg(pos));
  double distance=to-from;
  double increment=distance/(double) steps;
  return from+increment*(double) pos;
}

const FrameData BVHNode::frameData(int frame) const
{
  // return empty frame data on end site nodes
  if(type==BVH_END) return FrameData();
  // if the keyframe exists, return the data
  if(isKeyframe(frame)) return keyframes[frame];

  // get keyframes before and after desired frame
  FrameData before=getKeyframeBefore(frame);
  FrameData after=getNextKeyframe(before.frameNumber());

  int frameBefore=before.frameNumber();
  int frameAfter=after.frameNumber();

  // if before and after frames are the same, there are no more keyframes left, so
  // we return the last keyframe data
  if(frameBefore==frameAfter) return before;

  Rotation rotBefore=before.rotation();
  Rotation rotAfter=after.rotation();
  Position posBefore=before.position();
  Position posAfter=after.position();

  Rotation iRot;
  Position iPos;

  iRot.x=interpolate(rotBefore.x,rotAfter.x,frameAfter-frameBefore,frame-frameBefore,true);
  iRot.y=interpolate(rotBefore.y,rotAfter.y,frameAfter-frameBefore,frame-frameBefore,true);
  iRot.z=interpolate(rotBefore.z,rotAfter.z,frameAfter-frameBefore,frame-frameBefore,true);

  iPos.x=interpolate(posBefore.x,posAfter.x,frameAfter-frameBefore,frame-frameBefore,true);
  iPos.y=interpolate(posBefore.y,posAfter.y,frameAfter-frameBefore,frame-frameBefore,true);
  iPos.z=interpolate(posBefore.z,posAfter.z,frameAfter-frameBefore,frame-frameBefore,true);

// qDebug(QString("iRot.x %1 frame %2: %3").arg(rotBefore.bodyPart).arg(before.frameNumber()).arg(iRot.x));

  // return interpolated frame data here
  return FrameData(frame,iPos,iRot);
}

const FrameData BVHNode::getKeyframeBefore(int frame) const
{
  if(frame==0)
  {
    // should never happen
    qDebug("BVHNode::getKeyframeBefore(int frame): frame==0!");
    return keyframes[0];
  }
  return frameData(getKeyframeNumberBefore(frame));
}

const FrameData BVHNode::getNextKeyframe(int frame) const
{
  QMap<int,FrameData>::const_iterator itCurrent=keyframes.find(frame);
  itCurrent++;
  // if we are asked for a keyframe past the last one, return the last one
  if(itCurrent==keyframes.end()) --itCurrent;
  return (*itCurrent);
}

int BVHNode::getKeyframeNumberBefore(int frame) const
{
  if(frame==0)
  {
    // should never happen
    qDebug("BVHNode::getKeyframeNumberBefore(int frame): frame==0!");
    return 0;
  }

  // get a list of all keyframe numbers
  QValueList<int> keys=keyframeList();

  // find previous key
  while(--frame && !isKeyframe(frame));

  return frame;
}

const FrameData BVHNode::keyframeDataByIndex(int index) const
{
  // get a list of all keyframe numbers
  QValueList<int> keys=keyframeList();
  // get frame number of keyframe at given index
  int number=keys[index];
  // return keyframe data
  return keyframes[number];
}

const QValueList<int> BVHNode::keyframeList() const
{
  return keyframes.keys();
}

int BVHNode::numKeyframes() const
{
  return keyframes.count();
}

const Rotation* BVHNode::getCachedRotation(int frame)
{
  return rotations.at(frame);
}

const Position* BVHNode::getCachedPosition(int frame)
{
  return positions.at(frame);
}

void BVHNode::cacheRotation(Rotation* rot)
{
  rotations.append(rot);
}

void BVHNode::cachePosition(Position* pos)
{
  positions.append(pos);
}

void BVHNode::flushFrameCache()
{
  rotations.clear();
  positions.clear();
}

void BVHNode::dumpKeyframes()
{
  QValueList<int> keys=keyframeList();
  for(unsigned int index=0;index<keyframes.count();index++)
  {
    Rotation rot=frameData(keys[index]).rotation();
    Position pos=frameData(keys[index]).position();

    qDebug(QString("%1: %2 - Pos: <%3,%4,%5> Rot: <%6,%7,%8>").arg(name()).arg(keys[index]).arg(pos.x).arg(pos.y).arg(pos.z).arg(rot.x).arg(rot.y).arg(rot.z));
  }
}

// ************************************************************************

FrameData::FrameData()
{
//  qDebug(QString("FrameData(%1)").arg((unsigned long)this));
  m_frameNumber=0;
  m_easeIn=false;
  m_easeOut=false;
}

FrameData::FrameData(int num,Position pos,Rotation rot)
{
//  qDebug(QString("FrameData(%1): frame %2  pos %3,%4,%5 rot %6,%7,%8").arg((unsigned long) this).arg(frame).arg(pos.x).arg(pos.y).arg(pos.z).arg(rot.x).arg(rot.y).arg(rot.z));
  m_frameNumber=num;
  m_rotation=rot;
  m_position=pos;
  m_easeIn=false;
  m_easeOut=false;
}

int FrameData::frameNumber() const
{
  return m_frameNumber;
}

Position FrameData::position() const
{
  return m_position;
}

Rotation FrameData::rotation() const
{
  return m_rotation;
}

void FrameData::setPosition(const Position& pos)
{
  m_position=pos;
}

void FrameData::setRotation(const Rotation& rot)
{
//  qDebug(QString("FrameData::setRotation(<%1,%2,%3>)").arg(m_rotation.x).arg(m_rotation.y).arg(m_rotation.z));
//  qDebug(QString("FrameData::setRotation(<%1,%2,%3>)").arg(rot.x).arg(rot.y).arg(rot.z));
  m_rotation.x=rot.x;
  m_rotation.y=rot.y;
  m_rotation.z=rot.z;
//  qDebug(QString("FrameData::setRotation(<%1,%2,%3>)").arg(m_rotation.x).arg(m_rotation.y).arg(m_rotation.z));
//  m_rotation=rot;
}

FrameData::~FrameData()
{
//  qDebug(QString("~FrameData(%1)").arg((unsigned long) this));
}
