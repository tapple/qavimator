/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Copyright (C) 2006 by Vinay Pulim.
 * All rights reserved.
 *
 */

#ifndef BVH_H
#define BVH_H

#define MAX_FRAMES 1800

#define BOOL int
#define false 0
#define true 1

typedef enum { BVH_ROOT=0, BVH_JOINT=1, BVH_END=2 } BVHNodeType;
typedef enum { BVH_XPOS, BVH_YPOS, BVH_ZPOS, BVH_XROT, BVH_YROT, BVH_ZROT } BVHChannelType;
typedef enum { BVH_XYZ=1, BVH_ZYX, BVH_XZY, BVH_YZX, BVH_YXZ, BVH_ZXY} BVHOrderType;

static char bvhTypeName[3][6] = { "ROOT", "JOINT", "End" };
static char bvhChannelName[6][10] = 
  { "Xposition", "Yposition", "Zposition", 
    "Xrotation", "Yrotation", "Zrotation"};

typedef struct _BVHNode {
  char name[64];
  BVHNodeType type;
  double offset[3];
  int numChannels;
  BVHChannelType channelType[6];
  BVHOrderType channelOrder;
  double channelMin[6];
  double channelMax[6];
  int numFrames;
  double frame[MAX_FRAMES][6];
  BOOL ikOn;
  double ikRot[3];
  double ikGoalPos[3];
  double ikGoalDir[3];
  double ikWeight;
  int numKeyFrames;
  int keyFrames[MAX_FRAMES];
  double frameTime;
  int numChildren;  
  struct _BVHNode *child[16];  
} BVHNode;

BVHNode *bvhRead(const char *file);
void bvhWrite(BVHNode *root, const char *file);
BVHNode *bvhFindNode(BVHNode *root, const char *name);
void bvhSetChannel(BVHNode *node, int frame, BVHChannelType type, double val);
double bvhGetChannel(BVHNode *node, int frame, BVHChannelType type);
void bvhGetChannelLimits(BVHNode *node, BVHChannelType type, 
			 double *min, double *max);
void bvhResetIK(BVHNode *root);
//double bvhGetOffset(BVHNode *node, double *x, double *y, double *z);
//double bvhGetLength(BVHNode *node, double *x, double *y, double *z);
const char *bvhGetName(BVHNode *node, int index);
int bvhGetIndex(BVHNode *node, const char *name);
void bvhCopyOffsets(BVHNode *dst,BVHNode *src);
int bvhGetFrameData(BVHNode *node, int frame, double *data);
int bvhSetFrameData(BVHNode *node, int frame, double *data);

// lex neva's stuff:
BVHNode *animRead(const char *file, const char *limFile);
BVHNode *avmRead(const char *file);
void avmWrite(BVHNode *root, const char *file);
void animWrite(BVHNode *root, const char *file);
void bvhDelete(BVHNode *node);
void bvhSetFrameTime(BVHNode *node, double frameTime);

#endif
