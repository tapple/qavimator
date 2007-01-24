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

#include <qstringlist.h>

#include "bvhnode.h"

#ifndef BVH_H
#define BVH_H

class BVH
{
  public:

    BVH();
    ~BVH();

    BVHNode *bvhRead(const char *file) const;
    char* token(FILE *f,char* tokenBuf) const;
    int expect_token(FILE *f, char *name) const;
    BVHNode* bvhReadNode(FILE *f) const;
    void assignChannels(BVHNode *node, FILE *f, int frame) const;
    void setChannelLimits(BVHNode *node,BVHChannelType type,double min,double max) const;
    void parseLimFile(BVHNode *root, const char *limFile) const;
    void setNumFrames(BVHNode *node, int numFrames) const;
    void setAllKeyFrames(BVHNode *node) const;
    void avmReadKeyFrame(BVHNode *root, FILE *f) const;
    void bvhIndent(FILE *f, int depth);
    void bvhWriteNode(BVHNode *node, FILE *f, int depth);
    void bvhWriteFrame(BVHNode *node, int frame, FILE *f);
    void bvhWriteZeroFrame(BVHNode *node, FILE *f);
    void avmWriteKeyFrame(BVHNode *root, FILE *f);
    void bvhPrintNode(BVHNode *n, int depth);
    const char* bvhGetNameHelper(BVHNode *node, int index);
    int bvhGetIndexHelper(BVHNode *node, const char *name);

void bvhWrite(BVHNode *root, const char *file);
BVHNode *bvhFindNode(BVHNode *root, const char *name) const;
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
BVHNode *animRead(const char *file, const char *limFile) const;
BVHNode *avmRead(const char *file) const;
void avmWrite(BVHNode *root, const char *file);
void animWrite(BVHNode *root, const char *file);
void bvhDelete(BVHNode *node);
void bvhSetFrameTime(BVHNode *node, double frameTime);

QStringList bvhTypeName;
QStringList bvhChannelName;
int nodeCount;
};

#endif
