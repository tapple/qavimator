/***************************************************************************
 *   Copyright (C) 2008 by Zi Ree   *
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
#include "track.h"

Track::Track(BVHNode* trackNode,int trackNum,TrackType trackType)
{
  node=trackNode;
  num=trackNum;
  type=trackType;
  endSite=false;

  if(trackNode->numChildren()==0)
    qDebug("Track::Track(): node does not have any child nodes! Potential problem!");

  else if(trackNode->child(0)->type==BVH_END)
   endSite=true;
}

Track::~Track()
{
}

BVHNode* Track::getNode()
{
  return node;
}

int Track::getNum()
{
  return num;
}

bool Track::isEndSite()
{
  return endSite;
}
