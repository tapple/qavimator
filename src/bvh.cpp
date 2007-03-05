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
  tokenBuf[0]='\0';

  if (feof(f)) {
    fprintf(stderr, "Bad BVH file: Premature EOF\n");
    return tokenBuf;
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

  char order[4];
  int i;

  // check for node type first
  BVHNodeType nodeType;
  if (!strcasecmp(type, "ROOT")) { nodeType = BVH_ROOT; }
  else if (!strcasecmp(type, "JOINT")) { nodeType = BVH_JOINT; }
  else if (!strcasecmp(type, "END")) { nodeType = BVH_END; }
  else {
    fprintf(stderr, "Bad BVH file: unknown node type: %s\n", type);
    return NULL;
  }

  // add node with name
  BVHNode* node = new BVHNode(token(f,buffer));
  // set node type
  node->type=nodeType;

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
  BVHNode* child = NULL;
  do {
    if ((child = bvhReadNode(f))) {
      node->addChild(child);
    }
  } while (child != NULL);
  return node;
}

void BVH::assignChannels(BVHNode *node, FILE *f, int frame)
{
  // create new rotation and position objects
  Rotation* rot=new Rotation();
  Position* pos=new Position();

  char buffer[1024];

  for (int i=0; i<node->numChannels; i++)
  {
    double value=atof(token(f,buffer));
    BVHChannelType type=node->channelType[i];
    if(type==BVH_XPOS) pos->x=value;
    else if(type==BVH_YPOS) pos->y=value;
    else if(type==BVH_ZPOS) pos->z=value;
    else if(type==BVH_XROT) rot->x=value;
    else if(type==BVH_YROT) rot->y=value;
    else if(type==BVH_ZROT) rot->z=value;
    else qDebug("unknown channel type");
  }

  // put rotation and position into the node's cache for later keyframe referencing
  node->cacheRotation(rot);
  node->cachePosition(pos);

  for (int i=0; i<node->numChildren(); i++) {
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

void BVH::setNumFrames(int numFrames)
{
  totalFrames=numFrames;
}

int BVH::numFrames() const
{
  return totalFrames;
}

// in BVH files, this is necessary so that
// all frames but the start and last aren't
// blown away by interpolation
void BVH::setAllKeyFrames(BVHNode *node) const
{
  for (int i=0;i<totalFrames;i++)
  {
    const Rotation* rot=node->getCachedRotation(i);
    const Position* pos=node->getCachedPosition(i);

    if(node->type!=BVH_END)
      node->addKeyframe(i,Position(pos->x,pos->y,pos->z),Rotation(rot->x,rot->y,rot->z));
  }

  for (int i=0;i<node->numChildren();i++)
    setAllKeyFrames(node->child(i));
}

BVHNode* BVH::bvhRead(const char *file)
{
  qDebug(QString("BVH::bvhRead(%1)").arg(file));

  char buffer[1024];
  FILE *f = fopen(file, "rt");
  BVHNode *root;

  if (!f) {
    fprintf(stderr, "BVH File not found: %s\n", file);
    return NULL;
  }

  expect_token(f, "HIERARCHY");
  root = bvhReadNode(f);

  expect_token(f, "MOTION");
  expect_token(f, "Frames:");
  totalFrames = atoi(token(f,buffer));

  expect_token(f, "Frame");
  expect_token(f, "Time:");
  root->frameTime = atof(token(f,buffer));

  for (int i=0; i<totalFrames; i++) {
    assignChannels(root, f, i);
  }

  setAllKeyFrames(root);
  fclose(f);

  return(root);
}

void BVH::avmReadKeyFrame(BVHNode *root, FILE *f)
{
  // NOTE: new system needs frame 0 as key frame
  // FIXME: find a better way without code duplication
  const Rotation* rot=root->getCachedRotation(0);
  const Position* pos=root->getCachedPosition(0);
  root->addKeyframe(0,Position(pos->x,pos->y,pos->z),Rotation(rot->x,rot->y,rot->z));

  char buffer[1024];
  int numKeyFrames=atoi(token(f,buffer));
  for (int i=0;i<numKeyFrames;i++)
  {
    token(f,buffer);
    int key=atoi(buffer);

    const Rotation* rot=root->getCachedRotation(key);
    const Position* pos=root->getCachedPosition(key);
    root->addKeyframe(key,Position(pos->x,pos->y,pos->z),Rotation(rot->x,rot->y,rot->z));
  }

  // all keyframes are found, flush the node's cache to free up memory
  root->flushFrameCache();
//  root->dumpKeyframes();

  for (int i=0;i<root->numChildren();i++) {
    avmReadKeyFrame(root->child(i), f);
  }
}

// reads ease in / out data
void BVH::avmReadKeyFrameProperties(BVHNode *root, FILE *f)
{
  // NOTE: key frame properties save key 0, too, so numKeyFrames here will be one higher than before
  char buffer[1024];

  token(f,buffer);
  if(!strlen(buffer)) return;

  int numKeyFrames=atoi(buffer);
  for (int i=0;i<numKeyFrames;i++)
  {
    token(f,buffer);
    int key=atoi(buffer);

    if(key & 1) root->setEaseIn(root->keyframeDataByIndex(i).frameNumber(),true);
    if(key & 2) root->setEaseOut(root->keyframeDataByIndex(i).frameNumber(),true);
  }

  // all keyframes are found, flush the node's cache to free up memory
  root->flushFrameCache();
//  root->dumpKeyframes();

  for (int i=0;i<root->numChildren();i++) {
    avmReadKeyFrameProperties(root->child(i), f);
  }
}

/* .avm files look suspiciously like .bvh files, except
   with keyframe data tacked at the end -- Lex Neva */
BVHNode* BVH::avmRead(const char *file)
{
  qDebug(QString("BVH::avmRead(%1)").arg(file));

  FILE *f = fopen(file, "rt");
  char buffer[1024];
  BVHNode *root;

  if (!f) {
    fprintf(stderr, "AVM File not found: %s\n", file);
    return NULL;
  }

  expect_token(f, "HIERARCHY");
  root = bvhReadNode(f);

  expect_token(f, "MOTION");
  expect_token(f, "Frames:");
  totalFrames = atoi(token(f,buffer));

  expect_token(f, "Frame");
  expect_token(f, "Time:");
  root->frameTime = atof(token(f,buffer));

  for (int i=0; i<totalFrames; i++) {
    assignChannels(root, f, i);
  }

  avmReadKeyFrame(root, f);
  if (!feof(f)) {
    if(expect_token(f, "Properties"))
      avmReadKeyFrameProperties(root, f);
  }
  fclose(f);

  return(root);
}

BVHNode* BVH::animRead(const char *file, const char *limFile)
{
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
  for (int i=0; i<depth; i++) {
    fprintf(f, "\t");
  }
}

void BVH::bvhWriteNode(BVHNode *node, FILE *f, int depth)
{
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
    bvhIndent(f, depth+1);
    fprintf(f, "CHANNELS %d ", node->numChannels);
    for (int i=0; i<node->numChannels; i++) {
      fprintf(f, "%s ", bvhChannelName[node->channelType[i]].latin1());
    }
    fprintf(f, "\n");
  }
  for (int i=0; i<node->numChildren(); i++) {
    bvhWriteNode(node->child(i), f, depth+1);
  }
  bvhIndent(f, depth);
  fprintf(f, "}\n");
}

void BVH::bvhWriteFrame(BVHNode *node, int frame, FILE *f)
{
  Rotation rot=node->frameData(frame).rotation();
  Position pos=node->frameData(frame).position();

  // preserve channel order while writing
  for (int i=0; i<node->numChannels; i++)
  {
    float value=0.0;
    BVHChannelType type=node->channelType[i];

    if     (type==BVH_XPOS) value=pos.x;
    else if(type==BVH_YPOS) value=pos.y;
    else if(type==BVH_ZPOS) value=pos.z;

    else if(type==BVH_XROT) value=rot.x;
    else if(type==BVH_YROT) value=rot.y;
    else if(type==BVH_ZROT) value=rot.z;

    fprintf(f, "%f ",value);
  }

  for (int i=0; i<node->numChildren(); i++) {
    bvhWriteFrame(node->child(i), frame, f);
  }
}

void BVH::bvhWrite(BVHNode *root, const char *file)
{
  int i;
  FILE *f = fopen(file, "wt");

  fprintf(f, "HIERARCHY\n");
  bvhWriteNode(root, f, 0);
  fprintf(f, "MOTION\n");
  fprintf(f, "Frames:\t%d\n", totalFrames);
  fprintf(f, "Frame Time:\t%f\n", root->frameTime);
  for (i=0; i<totalFrames; i++) {
    bvhWriteFrame(root, i, f);
    fprintf(f, "\n");
  }
  fclose(f);
}

void BVH::avmWriteKeyFrame(BVHNode *root, FILE *f)
{
  const QValueList<int> keys=root->keyframeList();
  fprintf(f, "%d ", keys.count()-1);

  // skip frame 0 (always key frame) while saving
  for (unsigned int i=1; i<keys.count(); i++) {
    fprintf(f, "%d ", keys[i]);
  }
  fprintf(f, "\n");

  for (int i=0;i<root->numChildren();i++) {
    avmWriteKeyFrame(root->child(i), f);
  }
}

// writes ease in / out data
void BVH::avmWriteKeyFrameProperties(BVHNode *root, FILE *f)
{
  const QValueList<int> keys=root->keyframeList();

  // NOTE: remember, ease in/out data always takes first frame into account
  fprintf(f, "%d ", keys.count());

  // NOTE: remember, ease in/out data always takes first frame into account
  for (unsigned int i=0; i<keys.count(); i++) {
    int type=0;

    if(root->keyframeDataByIndex(i).easeIn()) type|=1;
    if(root->keyframeDataByIndex(i).easeOut()) type|=2;

    fprintf(f, "%d ", type);
  }
  fprintf(f, "\n");

  for (int i=0;i<root->numChildren();i++) {
    avmWriteKeyFrameProperties(root->child(i), f);
  }
}

void BVH::avmWrite(BVHNode *root, const char *file)
{
  FILE *f = fopen(file, "wt");

  fprintf(f, "HIERARCHY\n");
  bvhWriteNode(root, f, 0);
  fprintf(f, "MOTION\n");
  fprintf(f, "Frames:\t%d\n", totalFrames);
  fprintf(f, "Frame Time:\t%f\n", root->frameTime);
  for (int i=0; i<totalFrames; i++) {
    bvhWriteFrame(root, i, f);
    fprintf(f, "\n");
  }

  avmWriteKeyFrame(root, f);
  fprintf(f, "Properties\n");
  avmWriteKeyFrameProperties(root, f);

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

const QString& BVH::bvhGetNameHelper(BVHNode* node,int index)
{
  nodeCount++;
  if(nodeCount==index) return node->name();

  for(int i=0;i<node->numChildren();i++)
  {
    const QString& val=bvhGetNameHelper(node->child(i),index);
    if(!val.isEmpty()) return val;
  }
  return QString::null;
}

const QString& BVH::bvhGetName(BVHNode* node,int index)
{
  nodeCount = 0;
  return bvhGetNameHelper(node, index);
}

int BVH::bvhGetIndexHelper(BVHNode* node,const QString& name)
{
  nodeCount++;
  if(node->name()==name) return nodeCount;

  for(int i=0;i<node->numChildren();i++)
  {
    int val;
    if ((val = bvhGetIndexHelper(node->child(i), name)))
      return val;
  }
  return 0;
}

int BVH::bvhGetIndex(BVHNode* node,const QString& name)
{
  nodeCount=0;
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

void BVH::bvhGetFrameDataHelper(BVHNode* node,int frame)
{
  if(node->type!=BVH_END)
  {
    positionCopyBuffer.append(node->frameData(frame).position());
    rotationCopyBuffer.append(node->frameData(frame).rotation());

//    rotationCopyBuffer[rotationCopyBuffer.count()-1].bodyPart=node->name(); // not necessary but nice for debugging
//    qDebug(QString("copying frame data for %1 frame number %2 (%3)")
//          .arg(node->name()).arg(frame).arg(positionCopyBuffer[rotationCopyBuffer.count()-1].bodyPart));
  }


  for(int i=0;i<node->numChildren();i++)
    bvhGetFrameDataHelper(node->child(i),frame);
}

void BVH::bvhGetFrameData(BVHNode* node,int frame)
{
  if(!node) return;

  rotationCopyBuffer.clear();
  positionCopyBuffer.clear();

  bvhGetFrameDataHelper(node,frame);
}

void BVH::bvhSetFrameDataHelper(BVHNode* node,int frame)
{
  // if this node is not the end of a joint child structure
  if(node->type!=BVH_END)
  {
    // add the node as key frame
    node->addKeyframe(frame,positionCopyBuffer[pasteIndex],rotationCopyBuffer[pasteIndex]);
//    qDebug(QString("pasting frame data for %1 frame number %2 (%3)").arg(node->name()).arg(frame).arg(rotationCopyBuffer[pasteIndex].bodyPart));
    // increment paste buffer counter
    pasteIndex++;
  }

  // traverse down the child list, call this function recursively
  for(int i=0;i<node->numChildren();i++)
    bvhSetFrameDataHelper(node->child(i),frame);
}

void BVH::bvhSetFrameData(BVHNode* node,int frame)
{
  if(!node) return;

  // reset paste buffer counter
  pasteIndex=0;
  // paste all keyframes for all joints
  bvhSetFrameDataHelper(node,frame);
}

void BVH::bvhDelete(BVHNode *node) {
  if (node) {
    for (int i=0;i<node->numChildren();i++)
      bvhDelete(node->child(i));

    delete node;
  }
}

void BVH::bvhSetFrameTime(BVHNode *node, double frameTime) {
  node->frameTime = frameTime;

  for (int i=0;i<node->numChildren();i++)
      bvhSetFrameTime(node->child(i), frameTime);
}
