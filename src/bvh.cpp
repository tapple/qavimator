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

#include <QMessageBox>

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

  validNodes << "hip" << "abdomen" << "chest" << "neck" << "head"
             << "lCollar" << "lShldr" << "lForeArm" << "lHand"
             << "rCollar" << "rShldr" << "rForeArm" << "rHand"
             << "lThigh" << "lShin" << "lFoot"
             << "rThigh" << "rShin" << "rFoot"
             << "Site";
}

BVH::~ BVH()
{
}

QString BVH::token()
{
  if(tokenPos>=tokens.size())
  {
    qDebug("BVH::token(): no more tokens at index %d",tokenPos);
    return QString();
  }
  return tokens[tokenPos++];
}

bool BVH::expect_token(const QString& name)
{
  qDebug("BVH::expect_token('%s')",name.toLatin1().constData());

  if(name!=token())
  {
    qDebug("BVH::expect_token(): Bad or outdated animation file: %s missing\n", name.toLatin1().constData());
    return false;
  }
  return true;
}

BVHNode* BVH::bvhReadNode()
{
  qDebug("BVH::bvhReadNode()");

  QString type=token();
  if(type=="}") return NULL;

  // check for node type first
  BVHNodeType nodeType;
  if      (type=="ROOT")  nodeType=BVH_ROOT;
  else if (type=="JOINT") nodeType=BVH_JOINT;
  else if (type=="End")   nodeType=BVH_END;
  else
  {
    qDebug("BVH::bvhReadNode(): Bad animation file: unknown node type: '%s'\n",type.toLatin1().constData());
    return NULL;
  }

  // add node with name
  BVHNode* node=new BVHNode(token());
  if(!validNodes.contains(node->name()))
    node->type=BVH_NO_SL;
  else
    // set node type
    node->type=nodeType;

  expect_token("{");
  expect_token("OFFSET");
  node->offset[0]=token().toFloat();
  node->offset[1]=token().toFloat();
  node->offset[2]=token().toFloat();
  node->ikOn = false;
  node->ikWeight = 0.0;
  if(node->type!=BVH_END)
  {
    expect_token("CHANNELS");
    node->numChannels=token().toFloat();

    // rotation order for this node
    QString order(QString::null);
    for(int i=0;i<node->numChannels;i++)
    {
      node->channelMin[i] = -10000;
      node->channelMax[i] = 10000;
      type=token();
      if     (type=="Xposition") node->channelType[i]=BVH_XPOS;
      else if(type=="Yposition") node->channelType[i]=BVH_YPOS;
      else if(type=="Zposition") node->channelType[i]=BVH_ZPOS;
      else if(type=="Xrotation")
      {
        node->channelType[i]=BVH_XROT;
        order+='X';
      }
      else if(type=="Yrotation")
      {
        node->channelType[i]=BVH_YROT;
        order+='Y';
      }
      else if(type=="Zrotation")
      {
        node->channelType[i]=BVH_ZROT;
        order+='Z';
      }
    }

    if     (order=="XYZ") node->channelOrder=BVH_XYZ;
    else if(order=="ZYX") node->channelOrder=BVH_ZYX;
    else if(order=="YZX") node->channelOrder=BVH_YZX;
    else if(order=="XZY") node->channelOrder=BVH_XZY;
    else if(order=="YXZ") node->channelOrder=BVH_YXZ;
    else if(order=="ZXY") node->channelOrder=BVH_ZXY;
  }

  BVHNode* child=NULL;
  do
  {
    if((child=bvhReadNode()))
    {
      node->addChild(child);
    }
  } while (child!=NULL);

  return node;
}

void BVH::removeNoSLNodes(BVHNode* root)
{
  qDebug("BVH::removeNoSLNodes()");
  // walk through list of all child nodes
  for(int i=0;i<root->numChildren();i++)
  {
    BVHNode* child=root->child(i);
    // if this is an unsupported node, remove it
    if(child->type==BVH_NO_SL)
    {
      qDebug("BVH::removeNoSLNodes(): removing node '%s'",child->name().toLatin1().constData());
      // find all child joints of the unsupported child
      for(int j=0;j<child->numChildren();j++)
      {
        // move child nodes to the current parent joint
        root->insertChild(child->child(j),i);
      }
      // remove unsupported child node
      root->removeChild(child);

      // start checking for nodes at this point over again
      removeNoSLNodes(root);
      return;
    }
    // check next parent node
    removeNoSLNodes(root->child(i));
  }
}

void BVH::assignChannels(BVHNode* node,int frame)
{
//  qDebug("BVH::assignChannels()");

  // create new rotation and position objects
  Rotation* rot=new Rotation();
  Position* pos=new Position();

  for(int i=0;i<node->numChannels;i++)
  {
    double value=token().toFloat();
    BVHChannelType type=node->channelType[i];
    if     (type==BVH_XPOS) pos->x=value;
    else if(type==BVH_YPOS) pos->y=value;
    else if(type==BVH_ZPOS) pos->z=value;
    else if(type==BVH_XROT) rot->x=value;
    else if(type==BVH_YROT) rot->y=value;
    else if(type==BVH_ZROT) rot->z=value;
    else qDebug("BVH::assignChannels(): unknown channel type %d",(int) type);
  }

  // put rotation and position into the node's cache for later keyframe referencing
  node->cacheRotation(rot);
  node->cachePosition(pos);

  for (int i=0; i<node->numChildren(); i++)
    assignChannels(node->child(i),frame);
}

void BVH::setChannelLimits(BVHNode *node,BVHChannelType type,double min,double max) const
{
  qDebug("BVH::setChannelLimits()");

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
void BVH::parseLimFile(BVHNode* root,const QString& limFile) const
{
  qDebug("BVH::parseLimFile('%s')",limFile.toLatin1().constData());

  QFile limit(limFile);

  if(!limit.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(0,QObject::tr("Missing Limits File"),QObject::tr("<qt>Limits file not found at:<br>%1</qt>").arg(limFile));
    return;
  }

  while(!limit.atEnd())
  {
    QString line=limit.readLine(4096).trimmed();
    if(line.isEmpty())
    {
      QMessageBox::critical(0,QObject::tr("Error reading limits file"),QObject::tr("Error reading limits file."));
      return;
    }

    QStringList parameters=line.split(' ');
    QString name=parameters[0];
    double weight=parameters[1].toDouble();

    BVHNode* node=bvhFindNode(root,name);
    if(node)
    {
      node->ikWeight = weight;

      for(int i=0;i<3;i++)
      {
        QString channel=parameters[i*3+2];
        double min=parameters[i*3+3].toDouble();
        double max=parameters[i*3+4].toDouble();

        if     (channel.startsWith("X")) setChannelLimits(node,BVH_XROT,min,max);
        else if(channel.startsWith("Y")) setChannelLimits(node,BVH_YROT,min,max);
        else if(channel.startsWith("Z")) setChannelLimits(node,BVH_ZROT,min,max);
      } // for
    }
    else
      qDebug("BVH::parseLimFile(): Node '%s' not in animation. This will lead to problems!",name.toLatin1().constData());
  }
  limit.close();
}

// in BVH files, this is necessary so that
// all frames but the start and last aren't
// blown away by interpolation
void BVH::setAllKeyFramesHelper(BVHNode* node,int numberOfFrames) const
{
  qDebug("BVH::setAllKeyFramesHelper()");

  for(int i=0;i<numberOfFrames;i++)
  {
    const Rotation* rot=node->getCachedRotation(i);
    const Position* pos=node->getCachedPosition(i);

    if(node->type!=BVH_END)
      node->addKeyframe(i,Position(pos->x,pos->y,pos->z),Rotation(rot->x,rot->y,rot->z));
  }

  for (int i=0;i<node->numChildren();i++)
    setAllKeyFramesHelper(node->child(i),numberOfFrames);
}

void BVH::setAllKeyFrames(Animation* anim) const
{
  qDebug("BVH::setAllKeyFrames()");

  setAllKeyFramesHelper(anim->getMotion(),anim->getNumberOfFrames());
}

BVHNode* BVH::bvhRead(const QString& file)
{
  qDebug("BVH::bvhRead('%s')",file.toLatin1().constData());

  QFile animationFile(file);
  if(!animationFile.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(0,QObject::tr("File not found"),QObject::tr("BVH File not found: %1").arg(file.toLatin1().constData()));
    return NULL;
  }

  inputFile=animationFile.readAll();
  animationFile.close();

  inputFile=inputFile.simplified();
  tokens=inputFile.split(' ');

  expect_token("HIERARCHY");

  BVHNode* root=bvhReadNode();

  expect_token("MOTION");
  expect_token("Frames:");
  int totalFrames=token().toInt();
  lastLoadedNumberOfFrames=totalFrames;

  expect_token("Frame");
  expect_token("Time:");
  root->frameTime=token().toFloat();

  for(int i=0;i<totalFrames;i++)
    assignChannels(root,i);

  setAllKeyFramesHelper(root,totalFrames);

  return(root);
}

void BVH::avmReadKeyFrame(BVHNode* root)
{
  // NOTE: new system needs frame 0 as key frame
  // FIXME: find a better way without code duplication
  const Rotation* rot=root->getCachedRotation(0);
  const Position* pos=root->getCachedPosition(0);
  root->addKeyframe(0,Position(pos->x,pos->y,pos->z),Rotation(rot->x,rot->y,rot->z));

  int numKeyFrames=token().toInt();
  for(int i=0;i<numKeyFrames;i++)
  {
    int key=token().toInt();

    if(key<lastLoadedNumberOfFrames)
    {
      const Rotation* rot=root->getCachedRotation(key);
      const Position* pos=root->getCachedPosition(key);
      root->addKeyframe(key,Position(pos->x,pos->y,pos->z),Rotation(rot->x,rot->y,rot->z));
    }
  }

  // all keyframes are found, flush the node's cache to free up memory
  root->flushFrameCache();
//  root->dumpKeyframes();

  for (int i=0;i<root->numChildren();i++)
    avmReadKeyFrame(root->child(i));
}

// reads ease in / out data
void BVH::avmReadKeyFrameProperties(BVHNode* root)
{
  // NOTE: key frame properties save key 0, too, so numKeyFrames here will be one higher than before

  qDebug("BVH::avmReadKeyFrameProperties()");

  QString numKeys=token();
  if(numKeys.isEmpty()) return;

  int numKeyFrames=numKeys.toInt();
  qDebug("BVH::avmReadKeyFrameProperties(): reading properties for %d key frames",numKeyFrames);

  for(int i=0;i<numKeyFrames;i++)
  {
    int key=token().toInt();

    if(key & 1) root->setEaseIn(root->keyframeDataByIndex(i).frameNumber(),true);
    if(key & 2) root->setEaseOut(root->keyframeDataByIndex(i).frameNumber(),true);
  }

  // all keyframes are found, flush the node's cache to free up memory
  root->flushFrameCache();
//  root->dumpKeyframes();

  for (int i=0;i<root->numChildren();i++)
    avmReadKeyFrameProperties(root->child(i));
}

// for debugging only
void BVH::dumpNodes(BVHNode* node,QString indent)
{
  qDebug("%s %s (%d)",indent.toLatin1().constData(),node->name().toLatin1().constData(),node->numChildren());
  indent+="·--";
  for(int i=0;i<node->numChildren();i++)
  {
    dumpNodes(node->child(i),indent);
  }
}

/* .avm files look suspiciously like .bvh files, except
   with keyframe data tacked at the end -- Lex Neva */
BVHNode* BVH::avmRead(const QString& file)
{
  qDebug("BVH::avmRead(%s)",file.toLatin1().constData());

  QFile animationFile(file);
  animationFile.open(QIODevice::ReadOnly);
  inputFile=animationFile.readAll();
  animationFile.close();

  inputFile=inputFile.simplified();
  tokens=inputFile.split(' ');

  expect_token("HIERARCHY");

  BVHNode* root=bvhReadNode();

  expect_token("MOTION");
  expect_token("Frames:");
  int totalFrames=token().toInt();
  lastLoadedNumberOfFrames=totalFrames;
  lastLoadedLoopOut=totalFrames;
//  qDebug("BVH::avmRead(): set loop out to totalFrames");

  expect_token("Frame");
  expect_token("Time:");
  root->frameTime=token().toFloat();

  for(int i=0;i<totalFrames;i++)
  {
    assignChannels(root,i);
  }

  avmReadKeyFrame(root);

  if(expect_token("Properties"))
    avmReadKeyFrameProperties(root);

  // read remaining properties
  QString propertyName;
  while(!(propertyName=token()).isEmpty())
  {
      QString propertyValue=token();

      if(!propertyValue.isEmpty())
      {
        qDebug("BVH::avmRead(): Found extended property: '%s=%s'",propertyName.toLatin1().constData(),
                                                                  propertyValue.toLatin1().constData());
        if(propertyName=="Scale:")
        {
          lastLoadedAvatarScale=propertyValue.toFloat();
        }
        else if(propertyName=="Figure:")
        {
          lastLoadedFigureType=static_cast<Animation::FigureType>(propertyValue.toInt());
        }
        else if(propertyName=="LoopIn:")
        {
//          qDebug("BVH::avmRead(): set loop in to "+propertyValue);
          lastLoadedLoopIn=propertyValue.toInt();
        }
        else if(propertyName=="LoopOut:")
        {
//          qDebug("BVH::avmRead(): set loop out to "+propertyValue);
          lastLoadedLoopOut=propertyValue.toInt();
        }
        else
          qDebug("BVH::avmRead(): Unknown extended property '%s' (%s), ignoring.",propertyName.toLatin1().constData(),
                                                                                  propertyValue.toLatin1().constData());
    }
  } // while

  return(root);
}

BVHNode* BVH::animRead(const QString& file,const QString& limFile)
{
  BVHNode* root;

  // default avatar scale for BVH and AVM files
  lastLoadedAvatarScale=1.0;
  // default figure type
  lastLoadedFigureType=Animation::FIGURE_FEMALE;
  // indicates "no loop points set"
  lastLoadedLoopIn=-1;
//  qDebug("BVH::animRead(): set loop in to -1 to indicate missing loop points");
  // reset token position
  tokenPos=0;

  // rudimentary file type identification from filename
  if(file.endsWith(".bvh",Qt::CaseInsensitive))
    root = bvhRead(file);
  else if(file.endsWith(".avm",Qt::CaseInsensitive))
    root = avmRead(file);
  else
    return NULL;

  if(!limFile.isEmpty())
    parseLimFile(root, limFile);

  removeNoSLNodes(root);
//  dumpNodes(root,QString::null);

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
  fprintf(f,line.toLatin1().constData());
//  fprintf(f, "%s %s\n", bvhTypeName[node->type].toLatin1().constData(), node->name);
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
      fprintf(f, "%s ", bvhChannelName[node->channelType[i]].toLatin1().constData());
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

void BVH::bvhWrite(Animation* anim, const QString& file)
{
  int i;
  FILE *f = fopen(file.toLatin1().constData(), "wt");

  BVHNode* root=anim->getMotion();

  fprintf(f, "HIERARCHY\n");
  bvhWriteNode(root, f, 0);
  fprintf(f, "MOTION\n");
  fprintf(f, "Frames:\t%d\n", anim->getNumberOfFrames());
  fprintf(f, "Frame Time:\t%f\n", root->frameTime);
  for (i=0; i<anim->getNumberOfFrames(); i++) {
    bvhWriteFrame(root, i, f);
    fprintf(f, "\n");
  }
  fclose(f);
}

void BVH::avmWriteKeyFrame(BVHNode *root, FILE *f)
{
  const QList<int> keys=root->keyframeList();
  // no key frames (usually at joint ends), just write a 0\n line
  if(keys.count()==0)
  {
    fprintf(f, "0\n");
  }
  // write line of key files
  else
  {
    // write number of key files
    fprintf(f, "%u ", keys.count()-1);

    // skip frame 0 (always key frame) while saving and write all keys in a row
    for(unsigned int i=1;i< (unsigned int) keys.count();i++)
    {
      fprintf(f, "%d ", keys[i]);
    }
    fprintf(f, "\n");
  }

  for (int i=0;i<root->numChildren();i++)
  {
    avmWriteKeyFrame(root->child(i), f);
  }
}

// writes ease in / out data
void BVH::avmWriteKeyFrameProperties(BVHNode *root, FILE *f)
{
  const QList<int> keys=root->keyframeList();

  // NOTE: remember, ease in/out data always takes first frame into account
  fprintf(f,"%u ",keys.count());

  // NOTE: remember, ease in/out data always takes first frame into account
  for(unsigned int i=0;i< (unsigned int) keys.count();i++)
  {
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

void BVH::avmWrite(Animation* anim,const QString& file)
{
  FILE *f=fopen(file.toLatin1().constData(),"wt");

  BVHNode* root=anim->getMotion();

  fprintf(f, "HIERARCHY\n");
  bvhWriteNode(root, f, 0);
  fprintf(f, "MOTION\n");
  fprintf(f, "Frames:\t%d\n", anim->getNumberOfFrames());
//  qDebug("Frames:\t%d\n", anim->getNumberOfFrames());
  fprintf(f, "Frame Time:\t%f\n", root->frameTime);
  for (int i=0; i<anim->getNumberOfFrames(); i++) {
    bvhWriteFrame(root, i, f);
    fprintf(f, "\n");
  }

  avmWriteKeyFrame(root, f);
  fprintf(f, "Properties\n");
  avmWriteKeyFrameProperties(root, f);

  // write remaining properties
  fprintf(f, "Scale: %f\n",anim->getAvatarScale());
  fprintf(f, "Figure: %d\n",anim->getFigureType());
  fprintf(f, "LoopIn: %d\n",anim->getLoopInPoint());
  fprintf(f, "LoopOut: %d\n",anim->getLoopOutPoint());

  fclose(f);
}

void BVH::animWrite(Animation* anim,const QString& file)
{
  // rudimentary file type identification from filename
  if(file.endsWith(".bvh",Qt::CaseInsensitive))
    bvhWrite(anim,file);
  else if(file.endsWith(".avm",Qt::CaseInsensitive))
    avmWrite(anim,file);
}

void BVH::bvhPrintNode(BVHNode *n, int depth)
{
  int i;
  for (i=0; i<depth*4; i++) { printf(" "); }
  QString line=QString("%1 (%2 %3 %4)").arg(n->name()).arg(n->offset[0]).arg(n->offset[1]).arg(n->offset[2]);
  printf(line.toLatin1().constData());
//  printf("%s (%lf %lf %lf)\n", n->name, n->offset[0], n->offset[1], n->offset[2]);
  for (i=0; i<n->numChildren(); i++) {
    bvhPrintNode(n->child(i), depth+1);
  }
}

BVHNode* BVH::bvhFindNode(BVHNode* root,const QString& name) const
{
  BVHNode *node;
  if(!root) return NULL;
  if(root->name()==name) return root;

  for(int i=0;i<root->numChildren();i++)
  {
    if((node=bvhFindNode(root->child(i),name))) return node;
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

const QString BVH::bvhGetNameHelper(BVHNode* node,int index)
{
  nodeCount++;
  if(nodeCount==index) return node->name();

  for(int i=0;i<node->numChildren();i++)
  {
    const QString val=bvhGetNameHelper(node->child(i),index);
    if(!val.isEmpty()) return val;
  }
  return QString();
}

const QString BVH::bvhGetName(BVHNode* node,int index)
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
