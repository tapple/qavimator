/***************************************************************************
 *   Copyright (C) 2007 by Zi Ree   *
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

#ifndef BVHNODE_H
#define BVHNODE_H

#include <qstring.h>
#include <qptrlist.h>
#include <qmap.h>

#include "rotation.h"

#define MAX_FRAMES 1800

typedef enum { BVH_ROOT=0, BVH_JOINT=1, BVH_END=2 } BVHNodeType;
typedef enum { BVH_XPOS, BVH_YPOS, BVH_ZPOS, BVH_XROT, BVH_YROT, BVH_ZROT } BVHChannelType;
typedef enum { BVH_XYZ=1, BVH_ZYX, BVH_XZY, BVH_YZX, BVH_YXZ, BVH_ZXY} BVHOrderType;

class FrameData
{
  public:
    FrameData();
    FrameData(int frame,Position pos,Rotation rot);
    ~FrameData();

    int frameNumber() const;
    void setFrameNumber(int frame);

    Position position() const;
    Rotation rotation() const;
    void setPosition(const Position& pos);
    void setRotation(const Rotation& rot);

    bool easeIn() const;
    bool easeOut() const;
    void setEaseIn(bool state);
    void setEaseOut(bool state);

  protected:
    unsigned int m_frameNumber;

    Rotation m_rotation;
    Position m_position;

    bool m_easeIn;
    bool m_easeOut;
};

class BVHNode
{
  public:
    BVHNode(const QString& name);
    ~BVHNode();

    const QString& name() const;
    int numChildren() const;
    BVHNode* child(int num);
    void addChild(BVHNode* newChild);

    const FrameData frameData(int frame) const;
    const FrameData keyframeDataByIndex(int index) const;
    const QValueList<int> keyframeList() const;

    void addKeyframe(int frame,Position pos,Rotation rot);
    void deleteKeyframe(int frame);
    void setKeyframePosition(int frame,const Position& pos);
    void setKeyframeRotation(int frame,const Rotation& rot);
    void insertFrame(int frame); // moves all key frames starting at "frame" one frame further
    bool isKeyframe(int frame) const;
    int numKeyframes() const;

    const FrameData getKeyframeBefore(int frame) const;
    const FrameData getNextKeyframe(int frame) const;

    void setEaseIn(int frame,bool state);
    void setEaseOut(int frame,bool state);

    const Rotation* getCachedRotation(int frame);
    const Position* getCachedPosition(int frame);
    void cacheRotation(Rotation* rot);
    void cachePosition(Position* pos);
    void flushFrameCache();

    void dumpKeyframes();

    BVHNodeType type;
    double offset[3];
    int numChannels;
    BVHChannelType channelType[6];
    BVHOrderType channelOrder;
    double channelMin[6];
    double channelMax[6];

    bool ikOn;
    Rotation ikRot;
    double ikGoalPos[3];
    double ikGoalDir[3];
    double ikWeight;

    double frameTime;

  protected:
    void setName(const QString& newName);
    double interpolate(double from,double to,int steps,int pos,bool interpolationType) const;

    int getKeyframeNumberBefore(int frame) const;
    int getKeyframeNumberAfter(int frame) const;

    QString m_name;
    QPtrList<BVHNode> children;
    QMap<int,FrameData> keyframes;

    // rotation/position cache on load, will be cleared once the animation is loaded
    QPtrList<Rotation> rotations;
    QPtrList<Position> positions;
};

#endif
