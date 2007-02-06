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
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <qstring.h>
#include <qfile.h>
#include <qmessagebox.h>

#include "bvh.h"

BVH::BVH()
{
  bvhTypeName.append("ROOT");
  bvhTypeName.append("JOINT");
  bvhTypeName.append("End");
  bvhChannelName.append("Xposition");
  bvhChannelName.append("Yposition");
  bvhChannelName.append("Zposition");
  bvhChannelName.append("Xrotation");
  bvhChannelName.append("Yrotation");
  bvhChannelName.append("Zrotation");
}

BVH::~ BVH()
{
}

char* BVH::token(FILE *f,char* tokenBuf) const
{
  if (feof(f)) {
    fprintf(stderr, "Bad BVH file: Premature EOF\n");
    tokenBuf[0]='\0';
    return "";
  }

  if(fscanf(f, "%s", tokenBuf)); // fix compiler warning
  return tokenBuf;
}

int BVH::expect_token(FILE *f, char *name) const
{
  char tokenbuf[1024];

  if (strcmp(token(f,tokenbuf), name)) {
    fprintf(stderr, "Bad BVH file: %s missing\n", name);
    return 0;
  }
  return 1;
}

BVHNode* BVH::bvhReadNode(FILE *f) const
{
  char buffer[1024];
  const char *type = token(f,buffer);
  if (!strcmp(type, "}")) { return NULL; }

  BVHNode* node = new BVHNode();
  BVHNode* child = NULL;
  char order[4];
  int i;

  if (!strcasecmp(type, "ROOT")) { node->type = BVH_ROOT; }
  else if (!strcasecmp(type, "JOINT")) { node->type = BVH_JOINT; }
  else if (!strcasecmp(type, "END")) { node->type = BVH_END; }
  else {
    fprintf(stderr, "Bad BVH file: unknown node type: %s\n", type);
    delete node;
    return NULL;
  }
  node->setName(token(f,buffer));
//  strcpy(node->name, token(f,buffer));
  expect_token(f, "{");
  expect_token(f, "OFFSET");
  node->offset[0] = atof(token(f,buffer));
  node->offset[1] = atof(token(f,buffer));
  node->offset[2] = atof(token(f,buffer));
  node->ikOn = false;
  node->ikWeight = 0.0;
  if (node->type != BVH_END) {
    expect_token(f, "CHANNELS");
    node->numChannels = atoi(token(f,buffer));
    char *op = order;
    for (i=0; i<node->numChannels; i++) {
      node->channelMin[i] = -10000;
      node->channelMax[i] = 10000;
      type = token(f,buffer);
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
      node->addChild(child);
    }
  } while (child != NULL);
  return node;
}

void BVH::assignChannels(BVHNode *node, FILE *f, int frame) const
{
  int i;
  char buffer[1024];
  node->numFrames = frame + 1;
  for (i=0; i<node->numChannels; i++) {
    node->frame[frame][i] = atof(token(f,buffer));
  }

  for (i=0; i<node->numChildren(); i++) {
    assignChannels(node->child(i), f, frame);
  }
}

void BVH::setChannelLimits(BVHNode *node,BVHChannelType type,double min,double max) const
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

// read joint limits file
void BVH::parseLimFile(BVHNode *root, const char *limFile) const
{
  QFile f(limFile);
  BVHNode *node;

  if (!f.open(IO_ReadOnly))
  {
    QMessageBox::critical(0,QObject::tr("Missing Limits File"),QObject::tr("<qt>Limits file not found at:<br>%1</qt>").arg(limFile));
    return;
  }

  while (!f.atEnd())
  {
    QString line;
    if(!f.readLine(line,4096))
    {
      QMessageBox::critical(0,QObject::tr("Error reading limits file"),QObject::tr("Error reading limits file."));
      return;
    }

    QStringList parameters=QStringList::split(' ',line);
    QString name=parameters[0];
    double weight=parameters[1].toDouble();

    node = bvhFindNode(root, name);
    node->ikWeight = weight;

    for(int i=0;i<3;i++)
    {
      QString channel=parameters[i*3+2];
      double min=parameters[i*3+3].toDouble();
      double max=parameters[i*3+4].toDouble();

      if(channel.startsWith("X")) setChannelLimits(node, BVH_XROT, min, max);
      else if(channel.startsWith("Y")) setChannelLimits(node, BVH_YROT, min, max);
      else if(channel.startsWith("Z")) setChannelLimits(node, BVH_ZROT, min, max);
    }
  }
  f.close();
}

void BVH::setNumFrames(BVHNode *node, int numFrames) const {
  int i;
  node->numFrames = numFrames;

  for (i=0;i<node->numChildren();i++)
    setNumFrames(node->child(i), numFrames);
}

// in BVH files, this is necessary so that
// all frames but the start and last aren't
// blown away by interpolation
void BVH::setAllKeyFrames(BVHNode *node) const {
  int i;
  // skip first and last frames, they are automatic key frames
  for (i=1;i<node->numFrames;i++) {
    node->keyFrames[node->numKeyFrames] = i;
    node->numKeyFrames++;
  }

  for (i=0;i<node->numChildren();i++)
    setAllKeyFrames(node->child(i));
}

BVHNode* BVH::bvhRead(const char *file) const
{
  char buffer[1024];
  FILE *f = fopen(file, "rt");
  BVHNode *root;
//  BVHNode *node[128];
//  int numNodes;
  int numFrames;
  int i;

  if (!f) {
    fprintf(stderr, "BVH File not found: %s\n", file);
    return NULL;
  }

  expect_token(f, "HIERARCHY");
  root = bvhReadNode(f);
  expect_token(f, "MOTION");
  expect_token(f, "Frames:");
  numFrames = atoi(token(f,buffer));
  setNumFrames(root, numFrames);
  expect_token(f, "Frame");
  expect_token(f, "Time:");
  root->frameTime = atof(token(f,buffer));
  for (i=0; i<numFrames; i++) {
    assignChannels(root, f, i);
  }
  setAllKeyFrames(root);
  fclose(f);

  return(root);
}

void BVH::avmReadKeyFrame(BVHNode *root, FILE *f) const {
  int i;
  char buffer[1024];
  root->numKeyFrames = atoi(token(f,buffer));

  for (i=0;i<root->numKeyFrames;i++) {
    root->keyFrames[i] = atoi(token(f,buffer));
  }

  for (i=0;i<root->numChildren();i++) {
    avmReadKeyFrame(root->child(i), f);
  }
}

/* .avm files look suspiciously like .bvh files, except
   with keyframe data tacked at the end -- Lex Neva */
BVHNode* BVH::avmRead(const char *file) const
{
  FILE *f = fopen(file, "rt");
  char buffer[1024];
  BVHNode *root;
//  BVHNode *node[128];
//  int numNodes;
  int numFrames;
  int i;

  if (!f) {
    fprintf(stderr, "AVM File not found: %s\n", file);
    return NULL;
  }

  expect_token(f, "HIERARCHY");
  root = bvhReadNode(f);
  expect_token(f, "MOTION");
  expect_token(f, "Frames:");
  numFrames = atoi(token(f,buffer));
  setNumFrames(root, numFrames);
  expect_token(f, "Frame");
  expect_token(f, "Time:");
  root->frameTime = atof(token(f,buffer));
  for (i=0; i<numFrames; i++) {
    assignChannels(root, f, i);
  }

  avmReadKeyFrame(root, f);

  fclose(f);
  return(root);
}

BVHNode* BVH::animRead(const char *file, const char *limFile) const {
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

void BVH::bvhIndent(FILE *f, int depth)
{
  int i;
  for (i=0; i<depth; i++) {
    fprintf(f, "\t");
  }
}

void BVH::bvhWriteNode(BVHNode *node, FILE *f, int depth)
{
  int i;
  bvhIndent(f, depth);
  QString line=QString("%1 %2\n").arg(bvhTypeName[node->type]).arg(node->name());
  fprintf(f,line);
//  fprintf(f, "%s %s\n", bvhTypeName[node->type].latin1(), node->name);
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
      fprintf(f, "%s ", bvhChannelName[node->channelType[i]].latin1());
    }
    fprintf(f, "\n");
  }
  for (i=0; i<node->numChildren(); i++) {
    bvhWriteNode(node->child(i), f, depth+1);
  }
  bvhIndent(f, depth);
  fprintf(f, "}\n");
}

void BVH::bvhWriteFrame(BVHNode *node, int frame, FILE *f)
{
  int i;
  for (i=0; i<node->numChannels; i++) {
    fprintf(f, "%f ", node->frame[frame][i]);
  }
  for (i=0; i<node->numChildren(); i++) {
    bvhWriteFrame(node->child(i), frame, f);
  }
}

void BVH::bvhWriteZeroFrame(BVHNode *node, FILE *f)
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
  for (i=0; i<node->numChildren(); i++) {
    bvhWriteZeroFrame(node->child(i), f);
  }
}

void BVH::bvhWrite(BVHNode *root, const char *file)
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

void BVH::avmWriteKeyFrame(BVHNode *root, FILE *f) {
  int i;
  fprintf(f, "%d ", root->numKeyFrames);

  for (i=0; i<root->numKeyFrames; i++) {
    fprintf(f, "%d ", root->keyFrames[i]);
  }
  fprintf(f, "\n");

  for (i=0;i<root->numChildren();i++) {
    avmWriteKeyFrame(root->child(i), f);
  }
}

void BVH::avmWrite(BVHNode *root, const char *file)
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

void BVH::animWrite(BVHNode *root, const char *file) {
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

void BVH::bvhPrintNode(BVHNode *n, int depth)
{
  int i;
  for (i=0; i<depth*4; i++) { printf(" "); }
  QString line=QString("%1 (%2 %3 %4)").arg(n->name()).arg(n->offset[0]).arg(n->offset[1]).arg(n->offset[2]);
  printf(line);
//  printf("%s (%lf %lf %lf)\n", n->name, n->offset[0], n->offset[1], n->offset[2]);
  for (i=0; i<n->numChildren(); i++) {
    bvhPrintNode(n->child(i), depth+1);
  }
}

BVHNode* BVH::bvhFindNode(BVHNode *root, const char *name) const
{
  int i;
  BVHNode *node;
  if (!root) return NULL;
  if (!strcmp(root->name(), name))
    return root;

  for (i=0; i<root->numChildren(); i++) {
    if ((node=bvhFindNode(root->child(i), name)))
      return node;
  }

  return NULL;
}

void BVH::bvhSetChannel(BVHNode *node, int frame, BVHChannelType type, double val)
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

double BVH::bvhGetChannel(BVHNode *node, int frame, BVHChannelType type)
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

void BVH::bvhGetChannelLimits(BVHNode *node, BVHChannelType type,
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

void BVH::bvhResetIK(BVHNode *root)
{
  int i;
  if (root) {
    root->ikOn = false;
    for (i=0; i<root->numChildren(); i++) {
      bvhResetIK(root->child(i));
    }
  }
}

const char* BVH::bvhGetNameHelper(BVHNode *node, int index)
{
  int i;
  const char *val;

  nodeCount++;
  if (nodeCount == index)
    return (const char *)(node->name());
  for (i=0; i<node->numChildren(); i++) {
    if ((val = bvhGetNameHelper(node->child(i), index)))
      return val;
  }
  return NULL;
}

const char* BVH::bvhGetName(BVHNode *node, int index)
{
  nodeCount = 0;
  return bvhGetNameHelper(node, index);
}

int BVH::bvhGetIndexHelper(BVHNode *node, const char *name)
{
  int i;
  int val;

  nodeCount++;
  if (!strcmp(node->name(), name))
    return nodeCount;
  for (i=0; i<node->numChildren(); i++) {
    if ((val = bvhGetIndexHelper(node->child(i), name)))
      return val;
  }
  return 0;
}

int BVH::bvhGetIndex(BVHNode *node, const char *name)
{
  nodeCount = 0;
  return bvhGetIndexHelper(node, name);
}

void BVH::bvhCopyOffsets(BVHNode *dst,BVHNode *src)
{
  int i;
  if (!dst || !src) return;
  dst->offset[0] = src->offset[0];
  dst->offset[1] = src->offset[1];
  dst->offset[2] = src->offset[2];
  for (i=0; i<src->numChildren(); i++) {
    bvhCopyOffsets(dst->child(i), src->child(i));
  }
}

int BVH::bvhGetFrameData(BVHNode *node, int frame, double *data)
{
  int n = 0;
  int i;

  if (!node) return 0;
  for (i=0; i<node->numChannels; i++) {
    data[n++] = node->frame[frame][i];
  }
  for (i=0; i<node->numChildren(); i++) {
    n += bvhGetFrameData(node->child(i), frame, data + n);
  }
  return n;
}

int BVH::bvhSetFrameData(BVHNode *node, int frame, double *data)
{
  int n = 0;
int i;

  if (!node) return 0;
  for (i=0; i<node->numChannels; i++) {
    node->frame[frame][i] = data[n++];
  }
  for (i=0; i<node->numChildren(); i++) {
    n += bvhSetFrameData(node->child(i), frame, data + n);
  }
  return n;
}

void BVH::bvhDelete(BVHNode *node) {
  int i;
  if (node) {
    for (i=0;i<node->numChildren();i++)
      bvhDelete(node->child(i));

    free(node);
  }
}

void BVH::bvhSetFrameTime(BVHNode *node, double frameTime) {
  int i;
  node->frameTime = frameTime;

  for (i=0;i<node->numChildren();i++)
      bvhSetFrameTime(node->child(i), frameTime);
}
