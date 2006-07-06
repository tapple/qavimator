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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bvh.h"

char tokenBuf[1024];
int nodeCount;

char *token(FILE *f)
{
  if (feof(f)) {
    fprintf(stderr, "Bad BVH file: Premature EOF\n");
    return "";
  }
  fscanf(f, "%s", tokenBuf);
  return tokenBuf;
}

int expect_token(FILE *f, char *name)
{
  if (strcmp(token(f), name)) {
    fprintf(stderr, "Bad BVH file: %s missing\n", name);
    return 0;
  }
  return 1;
}

BVHNode *bvhReadNode(FILE *f)
{
  BVHNode *node = (BVHNode *)malloc(sizeof(BVHNode));
  BVHNode *child;
  char order[4];
  char *type = token(f);
  int i;

  node->numChildren = node->numChannels = 0;
  node->numKeyFrames = 0;
  node->ikRot[0] = node->ikRot[1] = node->ikRot[2] = 0;
  node->ikGoalPos[0] = node->ikGoalPos[1] = node->ikGoalPos[2] = 0;
  node->ikGoalDir[0] = node->ikGoalDir[2] = 0;
  node->ikGoalDir[1] = 1;
  if (!strcasecmp(type, "ROOT")) { node->type = BVH_ROOT; }
  else if (!strcasecmp(type, "JOINT")) { node->type = BVH_JOINT; }
  else if (!strcasecmp(type, "END")) { node->type = BVH_END; }
  else if (!strcmp(type, "}")) { free(node); return NULL; }
  else {
    fprintf(stderr, "Bad BVH file: unknown node type: %s\n", type);
    return NULL;
  }
  strcpy(node->name, token(f));
  expect_token(f, "{");
  expect_token(f, "OFFSET");
  node->offset[0] = atof(token(f));
  node->offset[1] = atof(token(f));
  node->offset[2] = atof(token(f));
  node->ikOn = false;
  node->ikWeight = 0.0;
  if (node->type != BVH_END) {
    expect_token(f, "CHANNELS");
    node->numChannels = atoi(token(f));
    char *op = order;
    for (i=0; i<node->numChannels; i++) {
      node->channelMin[i] = -10000;
      node->channelMax[i] = 10000;
      type = token(f);
      if (!strcasecmp(type, "Xposition")) {node->channelType[i] = BVH_XPOS;}
      else if (!strcasecmp(type, "Yposition")) {node->channelType[i]=BVH_YPOS;}
      else if (!strcasecmp(type, "Zposition")) {node->channelType[i]=BVH_ZPOS;}
      else if (!strcasecmp(type, "Xrotation")) {
	node->channelType[i]=BVH_XROT;
	*(op++) = 'X';
      }
      else if (!strcasecmp(type, "Yrotation")) {
	node->channelType[i]=BVH_YROT;
	*(op++) = 'Y';
      }
      else if (!strcasecmp(type, "Zrotation")) {
	node->channelType[i]=BVH_ZROT;
	*(op++) = 'Z';
      }
    }
    *op = '\0';
    if (!strcmp(order, "XYZ")) node->channelOrder = BVH_XYZ;
    else if (!strcmp(order, "ZYX")) node->channelOrder = BVH_ZYX;
    else if (!strcmp(order, "YZX")) node->channelOrder = BVH_YZX;
    else if (!strcmp(order, "XZY")) node->channelOrder = BVH_XZY;
    else if (!strcmp(order, "YXZ")) node->channelOrder = BVH_YXZ;
    else if (!strcmp(order, "ZXY")) node->channelOrder = BVH_ZXY;
  }
  do {
    if ((child = bvhReadNode(f))) {
      node->child[node->numChildren++] = child;
    }
  } while (child != NULL);
  return node;
}

void assignChannels(BVHNode *node, FILE *f, int frame)
{
  int i;
  node->numFrames = frame + 1;
  for (i=0; i<node->numChannels; i++) {
    node->frame[frame][i] = atof(token(f));
  }

  for (i=0; i<node->numChildren; i++) {
    assignChannels(node->child[i], f, frame);
  }
}

void setChannelLimits(BVHNode *node,BVHChannelType type,double min,double max)
{
  int i;
  if (!node) return;
  for (i=0; i<node->numChannels; i++) {
    if (node->channelType[i] == type) {
      node->channelMin[i] = min;
      node->channelMax[i] = max;
      return;
    }
  }
}

void parseLimFile(BVHNode *root, const char *limFile)
{
  FILE *f = fopen(limFile, "rt");
  char name[32];
  char channel[5];
  double min, max;
  double weight;
  BVHNode *node;

  if (!f) {
    fprintf(stderr, "Limits file not found: %s\n", limFile);
    return;
  }

  while (!feof(f)) {
    int i;
    fscanf(f, "%s %lf", name, &weight);
    node = bvhFindNode(root, name);
    node->ikWeight = weight;
    for (i=0; i<3; i++) {
      fscanf (f, "%s %lf %lf", channel, &min, &max);
      switch (channel[0]) {
      case 'X': setChannelLimits(node, BVH_XROT, min, max); break;
      case 'Y': setChannelLimits(node, BVH_YROT, min, max); break;
      case 'Z': setChannelLimits(node, BVH_ZROT, min, max); break;
      }
    }
  }
  fclose(f);
}

void setNumFrames(BVHNode *node, int numFrames) {
  int i;
  node->numFrames = numFrames;

  for (i=0;i<node->numChildren;i++)
    setNumFrames(node->child[i], numFrames);
}

// in BVH files, this is necessary so that
// all frames but the start and last aren't
// blown away by interpolation
void setAllKeyFrames(BVHNode *node) {
  int i;
  // skip first and last frames, they are automatic key frames
  for (i=1;i<node->numFrames;i++) {
    node->keyFrames[node->numKeyFrames] = i;
    node->numKeyFrames++;
  }

  for (i=0;i<node->numChildren;i++)
    setAllKeyFrames(node->child[i]);
}

BVHNode *bvhRead(const char *file)
{
  FILE *f = fopen(file, "rt");
  BVHNode *root;
  BVHNode *node[128];
  int numNodes, numFrames;
  int i;

  if (!f) {
    fprintf(stderr, "BVH File not found: %s\n", file);
    return NULL;
  }

  expect_token(f, "HIERARCHY");
  root = bvhReadNode(f);
  expect_token(f, "MOTION");
  expect_token(f, "Frames:");
  numFrames = atoi(token(f));
  setNumFrames(root, numFrames);
  expect_token(f, "Frame");
  expect_token(f, "Time:");
  root->frameTime = atof(token(f));
  for (i=0; i<numFrames; i++) {
    assignChannels(root, f, i);
  }
  setAllKeyFrames(root);
  fclose(f);

  return(root);
}

void avmReadKeyFrame(BVHNode *root, FILE *f) {
  int i;
  root->numKeyFrames = atoi(token(f));

  for (i=0;i<root->numKeyFrames;i++) {
    root->keyFrames[i] = atoi(token(f));
  }

  for (i=0;i<root->numChildren;i++) {
    avmReadKeyFrame(root->child[i], f);
  }
}

/* .avm files look suspiciously like .bvh files, except
   with keyframe data tacked at the end -- Lex Neva */
BVHNode *avmRead(const char *file)
{
  FILE *f = fopen(file, "rt");
  BVHNode *root;
  BVHNode *node[128];
  int numNodes, numFrames;
  int i;

  if (!f) {
    fprintf(stderr, "BVH File not found: %s\n", file);
    return NULL;
  }

  expect_token(f, "HIERARCHY");
  root = bvhReadNode(f);
  expect_token(f, "MOTION");
  expect_token(f, "Frames:");
  numFrames = atoi(token(f));
  setNumFrames(root, numFrames);
  expect_token(f, "Frame");
  expect_token(f, "Time:");
  root->frameTime = atof(token(f));
  for (i=0; i<numFrames; i++) {
    assignChannels(root, f, i);
  }

  avmReadKeyFrame(root, f);

  fclose(f);
  return(root);
}

BVHNode *animRead(const char *file, const char *limFile) {
  char *fileType;
  char *extension;
  BVHNode *root;

  // rudimentary file type identification from filename
  fileType = strdup(file);
  extension = fileType + strlen(fileType) - 4;

  if (strcasecmp(extension, ".bvh") == 0) {
    root = bvhRead(file);
  } else if (strcasecmp(extension, ".avm") == 0) {
    root = avmRead(file);
  } else {
    free(fileType);
    return NULL;
  }

  free(fileType);

  if (limFile) {
    parseLimFile(root, limFile);
  }

  return root;
}

void bvhIndent(FILE *f, int depth)
{
  int i;
  for (i=0; i<depth; i++) {
    fprintf(f, "\t");
  }
}

void bvhWriteNode(BVHNode *node, FILE *f, int depth)
{
  int i;
  bvhIndent(f, depth);
  fprintf(f, "%s %s\n", bvhTypeName[node->type], node->name);
  bvhIndent(f, depth);
  fprintf(f, "{\n");
  bvhIndent(f, depth+1);
  fprintf(f, "OFFSET %.6f %.6f %.6f\n",
	  node->offset[0],
	  node->offset[1],
	  node->offset[2]);
  if (node->type != BVH_END) {
    int i;
    bvhIndent(f, depth+1);
    fprintf(f, "CHANNELS %d ", node->numChannels);
    for (i=0; i<node->numChannels; i++) {
      fprintf(f, "%s ", bvhChannelName[node->channelType[i]]);
    }
    fprintf(f, "\n");
  }
  for (i=0; i<node->numChildren; i++) {
    bvhWriteNode(node->child[i], f, depth+1);
  }
  bvhIndent(f, depth);
  fprintf(f, "}\n");
}

void bvhWriteFrame(BVHNode *node, int frame, FILE *f)
{
  int i;
  for (i=0; i<node->numChannels; i++) {
    fprintf(f, "%f ", node->frame[frame][i]);
  }
  for (i=0; i<node->numChildren; i++) {
    bvhWriteFrame(node->child[i], frame, f);
  }
}

void bvhWriteZeroFrame(BVHNode *node, FILE *f)
{
  int i;
  for (i=0; i<node->numChannels; i++) {
    if (node->channelType[i] == BVH_XPOS ||
	node->channelType[i] == BVH_YPOS ||
	node->channelType[i] == BVH_ZPOS) {
      fprintf(f, "%f ", node->frame[0][i]);
    }
    else {
      fprintf(f, "0.000000 ");
    }
  }
  for (i=0; i<node->numChildren; i++) {
    bvhWriteZeroFrame(node->child[i], f);
  }
}

void bvhWrite(BVHNode *root, const char *file)
{
  int i;
  FILE *f = fopen(file, "wt");

  fprintf(f, "HIERARCHY\n");
  bvhWriteNode(root, f, 0);
  fprintf(f, "MOTION\n");
  fprintf(f, "Frames:\t%d\n", root->numFrames);
  fprintf(f, "Frame Time:\t%f\n", root->frameTime);
  for (i=0; i<root->numFrames; i++) {
    bvhWriteFrame(root, i, f);
    fprintf(f, "\n");
  }
  fclose(f);
}

void avmWriteKeyFrame(BVHNode *root, FILE *f) {
  int i;
  fprintf(f, "%d ", root->numKeyFrames);

  for (i=0; i<root->numKeyFrames; i++) {
    fprintf(f, "%d ", root->keyFrames[i]);
  }
  fprintf(f, "\n");

  for (i=0;i<root->numChildren;i++) {
    avmWriteKeyFrame(root->child[i], f);
  }
}

void avmWrite(BVHNode *root, const char *file)
{
  int i;
  FILE *f = fopen(file, "wt");

  fprintf(f, "HIERARCHY\n");
  bvhWriteNode(root, f, 0);
  fprintf(f, "MOTION\n");
  fprintf(f, "Frames:\t%d\n", root->numFrames);
  fprintf(f, "Frame Time:\t%f\n", root->frameTime);
  for (i=0; i<root->numFrames; i++) {
    bvhWriteFrame(root, i, f);
    fprintf(f, "\n");
  }

  avmWriteKeyFrame(root, f);

  fclose(f);
}

void animWrite(BVHNode *root, const char *file) {
  char *fileType;
  char *extension;

  // rudimentary file type identification from filename
  fileType = strdup(file);
  extension = fileType + strlen(fileType) - 4;

  if (strcasecmp(extension, ".bvh") == 0) {
    bvhWrite(root, file);
  } else if (strcasecmp(extension, ".avm") == 0) {
    avmWrite(root, file);
  }

  free(fileType);
}

void bvhPrintNode(BVHNode *n, int depth)
{
  int i;
  for (i=0; i<depth*4; i++) { printf(" "); }
  printf("%s (%lf %lf %lf)\n", n->name, n->offset[0], n->offset[1], n->offset[2]);
  for (i=0; i<n->numChildren; i++) {
    bvhPrintNode(n->child[i], depth+1);
  }
}

BVHNode *bvhFindNode(BVHNode *root, const char *name)
{
  int i;
  BVHNode *node;
  if (!root) return NULL;
  if (!strcmp(root->name, name))
    return root;

  for (i=0; i<root->numChildren; i++) {
    if ((node=bvhFindNode(root->child[i], name)))
      return node;
  }

  return NULL;
}

void bvhSetChannel(BVHNode *node, int frame, BVHChannelType type, double val)
{
  int i;
  if (!node) return;
  for (i=0; i<node->numChannels; i++) {
    if (node->channelType[i] == type) {
      node->frame[frame][i] = val;
      return;
    }
  }
}

double bvhGetChannel(BVHNode *node, int frame, BVHChannelType type)
{
  int i;
  if (!node) return 0;
  for (i=0; i<node->numChannels; i++) {
    if (node->channelType[i] == type) {
      return node->frame[frame][i];
    }
  }
  return 0;
}

void bvhGetChannelLimits(BVHNode *node, BVHChannelType type,
			 double *min, double *max)
{
  int i;
  if (!node) {
    *min = -10000;
    *max = 10000;
    return;
  }
  for (i=0; i<node->numChannels; i++) {
    if (node->channelType[i] == type) {
      *min = node->channelMin[i];
      *max = node->channelMax[i];
    }
  }
}

void bvhResetIK(BVHNode *root)
{
  int i;
  if (root) {
    root->ikOn = false;
    for (i=0; i<root->numChildren; i++) {
      bvhResetIK(root->child[i]);
    }
  }
}

const char *bvhGetNameHelper(BVHNode *node, int index)
{
  int i;
  const char *val;

  nodeCount++;
  if (nodeCount == index)
    return (const char *)(node->name);
  for (i=0; i<node->numChildren; i++) {
    if ((val = bvhGetNameHelper(node->child[i], index)))
      return val;
  }
  return NULL;
}

const char *bvhGetName(BVHNode *node, int index)
{
  nodeCount = 0;
  return bvhGetNameHelper(node, index);
}

int bvhGetIndexHelper(BVHNode *node, const char *name)
{
  int i;
  int val;

  nodeCount++;
  if (!strcmp(node->name, name))
    return nodeCount;
  for (i=0; i<node->numChildren; i++) {
    if ((val = bvhGetIndexHelper(node->child[i], name)))
      return val;
  }
  return 0;
}

int bvhGetIndex(BVHNode *node, const char *name)
{
  nodeCount = 0;
  return bvhGetIndexHelper(node, name);
}

void bvhCopyOffsets(BVHNode *dst,BVHNode *src)
{
  int i;
  if (!dst || !src) return;
  dst->offset[0] = src->offset[0];
  dst->offset[1] = src->offset[1];
  dst->offset[2] = src->offset[2];
  for (i=0; i<src->numChildren; i++) {
    bvhCopyOffsets(dst->child[i], src->child[i]);
  }
}

int bvhGetFrameData(BVHNode *node, int frame, double *data)
{
  int n = 0;
  int i;

  if (!node) return 0;
  for (i=0; i<node->numChannels; i++) {
    data[n++] = node->frame[frame][i];
  }
  for (i=0; i<node->numChildren; i++) {
    n += bvhGetFrameData(node->child[i], frame, data + n);
  }
  return n;
}

int bvhSetFrameData(BVHNode *node, int frame, double *data)
{
  int n = 0;
int i;

  if (!node) return 0;
  for (i=0; i<node->numChannels; i++) {
    node->frame[frame][i] = data[n++];
  }
  for (i=0; i<node->numChildren; i++) {
    n += bvhSetFrameData(node->child[i], frame, data + n);
  }
  return n;
}

void bvhDelete(BVHNode *node) {
  int i;
  if (node) {
    for (i=0;i<node->numChildren;i++)
      bvhDelete(node->child[i]);

    free(node);
  }
}

void bvhSetFrameTime(BVHNode *node, double frameTime) {
  int i;
  node->frameTime = frameTime;

  for (i=0;i<node->numChildren;i++)
      bvhSetFrameTime(node->child[i], frameTime);
}
