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

#include "bvhnode.h"

BVHNode::BVHNode()
{
  numChildren=0;
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
