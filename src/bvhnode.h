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

#define MAX_FRAMES 1800

typedef enum { BVH_ROOT=0, BVH_JOINT=1, BVH_END=2 } BVHNodeType;
typedef enum { BVH_XPOS, BVH_YPOS, BVH_ZPOS, BVH_XROT, BVH_YROT, BVH_ZROT } BVHChannelType;
typedef enum { BVH_XYZ=1, BVH_ZYX, BVH_XZY, BVH_YZX, BVH_YXZ, BVH_ZXY} BVHOrderType;

/* Not yet Functional
class Keyframe
{
  public:
    double xRot;
    double yRot;
    double zRot;

    double xPos;
    double yPos;
    double zPos;

    bool easeIn;
    bool easeOut;
};
*/

class BVHNode
{
  public:
    BVHNode();
    ~BVHNode();

    void setName(const QString& newName);
    const QString& name() const;
    int numChildren() const;
    BVHNode* child(int num);
    void addChild(BVHNode* newChild);

  public:
    BVHNodeType type;
    double offset[3];
    int numChannels;
    BVHChannelType channelType[6];
    BVHOrderType channelOrder;
    double channelMin[6];
    double channelMax[6];
    int numFrames;
    double frame[MAX_FRAMES][6];
    bool ikOn;
    double ikRot[3];
    double ikGoalPos[3];
    double ikGoalDir[3];
    double ikWeight;
    int numKeyFrames;
    int keyFrames[MAX_FRAMES];
    double frameTime;

  protected:
    QString m_name;
    QPtrList<BVHNode> children;
};

#endif
