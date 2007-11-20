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

#include <qapplication.h>

#include <iostream>
#include <stdio.h>
#include <string.h>
// #include "main.h"

#include "animation.h"
#include "rotation.h"
#include "bvh.h"

Animation::Animation(BVH* newBVH,const QString& bvhFile) :
  frame(0),totalFrames(0),mirrored(false)
{
//  qDebug(QString("Animation::Animation(%1)").arg((long) this));

  bvh=newBVH;
  if(!bvh)
  {
    qDebug("Animation::Animation(): BVH initialisation failed.");
    return;
  }

  QString fileName;

  // optionally have the support files for qavimator in FSH location
#ifndef _LINUX_FSH
  execPath=qApp->applicationDirPath();
#else
  execPath="/usr/share/qavimator";
#endif

  // load BVH that defines motion
  if (bvhFile)
    fileName=bvhFile;
  else
    fileName=execPath+"/"+DEFAULT_POSE;

  loadBVH(fileName);
  calcPartMirrors();
  useRotationLimits(true);
  setNumberOfFrames(bvh->lastLoadedNumberOfFrames);
  setAvatarScale(bvh->lastLoadedAvatarScale);

  ikTree.set(frames);
  setIK(IK_LHAND, false);
  setIK(IK_RHAND, false);
  setIK(IK_LFOOT, false);
  setIK(IK_RFOOT, false);

  setDirty(false);
}

Animation::~Animation()
{
//  qDebug(QString("Animation::~Animation(%1)").arg((long) this));

  // remove keyframes
  if(frames)
  {
    bvh->bvhDelete(frames);
    frames=NULL;
  }
}

void Animation::loadBVH(const QString& bvhFile)
{
  QString limFile=execPath+"/"+LIMITS_FILE;
  frames = bvh->animRead(bvhFile, limFile);
  setFrame(0);
}

void Animation::saveBVH(const QString& bvhFile)
{
  bvh->animWrite(this,bvhFile);
  setDirty(false);
}

double Animation::frameTime()
{
  if (frames)
    return frames->frameTime;
  else
    return 0;
}

void Animation::setFrameTime(double frameTime) {
  if (frames)
    bvh->bvhSetFrameTime(frames, frameTime);
  setDirty(true);
}

int Animation::getNumberOfFrames()
{
// qDebug("getNumberOfFrames()=%d",totalFrames);
  return totalFrames;
}

void Animation::setNumberOfFrames(int num)
{
// qDebug("setNumberOfFrames(%d)",num);
  totalFrames=num;
  setDirty(true);
  emit numberOfFrames(num);
}

int Animation::getFrame()
{
  return frame;
}

void Animation::setFrame(int frameNumber)
{
  if (frameNumber >= 0 && frameNumber < totalFrames &&
      frame != frameNumber)
  {
//    for (int i=0; i<NUM_IK; i++) {
//      setIK((IKPartType)i, false);
//    }
    frame = frameNumber;
    for (int i=0; i<NUM_IK; i++) if (ikOn[i]) { solveIK(); break; }

    emit currentFrame(frame);
    emit frameChanged();
  }
}

void Animation::setMirrored(bool mirror)
{
  mirrored=mirror;
}

bool Animation::getMirrored()
{
  return mirrored;
}

unsigned int Animation::getPartMirror(int index)
{
  return partMirror[index];
}

int Animation::stepForward()
{
  if (frames)
  {
    int nextFrame=(frame + 1) % totalFrames;
    if(!nextFrame) nextFrame=loopingPoint;
    setFrame(nextFrame);
    return nextFrame;
  }
  else
    return 0;
}

void Animation::setEaseIn(const QString& name,bool state)
{
  BVHNode *node=bvh->bvhFindNode(frames,name);
  if(node->isKeyframe(frame))
  {
    setDirty(true);
    node->setEaseIn(frame,state);
    // tell main class that the keyframe has changed
    emit redrawTrack(getPartIndex(name));
  }
}

void Animation::setEaseOut(const QString& name,bool state)
{
  BVHNode *node=bvh->bvhFindNode(frames,name);
  if(node->isKeyframe(frame))
  {
    setDirty(true);
    node->setEaseOut(frame,state);
    // tell main class that the keyframe has changed
    emit redrawTrack(getPartIndex(name));
  }
}

bool Animation::easeIn(const QString& name)
{
  BVHNode *node=bvh->bvhFindNode(frames,name);
  if(node->isKeyframe(frame))
  {
    return node->easeIn(frame);
  }
  qDebug("Animation::easeIn("+name+"): requested easeIn for non-keyframe!");
  return false;
}

bool Animation::easeOut(const QString& name)
{
  BVHNode *node=bvh->bvhFindNode(frames,name);
  if(node->isKeyframe(frame))
  {
    return node->easeOut(frame);
  }
  qDebug("Animation::easeOut("+name+"): requested easeOut for non-keyframe!");
  return false;
}

void Animation::setLoopPoint(int frame)
{
  loopingPoint=frame;
  setDirty(true);
}

int Animation::loopPoint()
{
  return loopingPoint;
}

void Animation::applyIK(const QString& name)
{
  BVHNode *node = bvh->bvhFindNode(frames, name);

  Rotation rot=node->frameData(frame).rotation();

  if (node) {
//    for (int i=0; i<3; i++) {
      rot.x+=node->ikRot.x;
      rot.y+=node->ikRot.y;
      rot.z+=node->ikRot.z;


      node->ikRot.x=0;
      node->ikRot.y=0;
      node->ikRot.z=0;
/*
      node->frame[frame][i] += node->ikRot[i];
      node->ikRot[i] = 0;
*/
//      node->ikOn = false;

      setDirty(true);
      addKeyFrame(node);
      node->setKeyframeRotation(frame,rot);
      emit redrawTrack(getPartIndex(name));
//    }
  }
}

void Animation::setIK(IKPartType part, bool flag)
{
  if (ikOn[part] == flag) return;
  ikOn[part] = flag;
  if (flag) {
    switch (part) {
      case IK_LHAND: ikTree.setGoal(frame, "lHand"); break;
      case IK_RHAND: ikTree.setGoal(frame, "rHand"); break;
      case IK_LFOOT: ikTree.setGoal(frame, "lFoot"); break;
      case IK_RFOOT: ikTree.setGoal(frame, "rFoot"); break;
      default: break;
    }
  }
  else {
    switch (part) {
      case IK_LHAND:
        applyIK("lHand");applyIK("lForeArm");
        applyIK("lShldr");applyIK("lCollar");
        if (!ikOn[IK_RHAND]) { applyIK("chest");applyIK("abdomen"); }
        break;
      case IK_RHAND:
        applyIK("rHand");applyIK("rForeArm");
        applyIK("rShldr");applyIK("rCollar");
        if (!ikOn[IK_LHAND]) { applyIK("chest");applyIK("abdomen"); }
        break;
      case IK_LFOOT: applyIK("lThigh");applyIK("lShin");applyIK("lFoot");break;
      case IK_RFOOT: applyIK("rThigh");applyIK("rShin");applyIK("rFoot");break;
      default: break;
    }
  }
}

void Animation::setIK(const QString& jointName,bool flag)
{
  if(jointName=="lHand" ||
     jointName=="lForeArm" ||
     jointName=="lShldr" ||
     jointName=="lCollar")
  {
    setIK(IK_LHAND, flag);
  }
  else if(jointName=="rHand" ||
          jointName=="rForeArm" ||
          jointName=="rShldr" ||
          jointName=="rCollar")
  {
    setIK(IK_RHAND, flag);
  }
  else if(jointName=="lThigh" ||
          jointName=="lShin" ||
          jointName=="lFoot")
  {
    setIK(IK_LFOOT, flag);
  }
  else if(jointName=="rThigh" ||
          jointName=="rShin" ||
          jointName=="rFoot")
  {
    setIK(IK_RFOOT, flag);
  }
}

bool Animation::getIK(const QString& jointName)
{
  if(jointName=="lHand" ||
     jointName=="lForeArm" ||
     jointName=="lShldr" ||
     jointName=="lCollar")
  {
    return getIK(IK_LHAND);
  }
  else if(jointName=="rHand" ||
          jointName=="rForeArm" ||
          jointName=="rShldr" ||
          jointName=="rCollar")
  {
    return getIK(IK_RHAND);
  }
  else if(jointName=="lThigh" ||
          jointName=="lShin" ||
          jointName=="lFoot")
  {
    return getIK(IK_LFOOT);
  }
  else if(jointName=="rThigh" ||
          jointName=="rShin" ||
          jointName=="rFoot")
  {
    return getIK(IK_RFOOT);
  }
  return false;
}

bool Animation::getIK(IKPartType part)
{
  return ikOn[part];
}

void Animation::solveIK()
{
  bvh->bvhResetIK(frames);
  if (ikOn[IK_LFOOT]) getEndSite("lFoot")->ikOn = true;
  if (ikOn[IK_RFOOT]) getEndSite("rFoot")->ikOn = true;
  if (ikOn[IK_LHAND]) getEndSite("lHand")->ikOn = true;
  if (ikOn[IK_RHAND]) getEndSite("rHand")->ikOn = true;

//  ikTree.setJointLimits(true);
  ikTree.solve(frame);
}

void Animation::setRotation(const QString& jointName, double x, double y, double z)
{
  BVHNode *node = bvh->bvhFindNode(frames, jointName);
  BVHNode *node2 = NULL;
  QString mirrorName;
  if (node) {
    //qDebug(QString("Animation::setRotation(")+jointName+")");

    for (int i=0; i<NUM_IK; i++) if (ikOn[i]) { solveIK(); break; }

    if(node->isKeyframe(frame))
      node->setKeyframeRotation(frame,Rotation(x,y,z));
    else
      node->addKeyframe(frame,node->frameData(frame).position(),Rotation(x,y,z));

    //      node->dumpKeyframes();
    if (mirrored && (mirrorName = getPartMirror(jointName)))
    {
      node2 = bvh->bvhFindNode(frames, mirrorName);

      // new keyframe system
      if(node2->isKeyframe(frame))
        node2->setKeyframeRotation(frame,Rotation(x,-y,-z));
      else
        node2->addKeyframe(frame,node->frameData(frame).position(),Rotation(x,y,z));
      // tell timeline that this keyframe has changed (added or changed is the same here)
      emit redrawTrack(getPartIndex(jointName));
    }
    setDirty(true);
    // tell timeline that this keyframe has changed (added or changed is the same here)
    emit redrawTrack(getPartIndex(jointName));
    emit frameChanged();
  }
}

Rotation Animation::getRotation(const QString& jointName)
{
  if(!jointName.isEmpty())
  {
    BVHNode* node=bvh->bvhFindNode(frames,jointName);
    if(node)
      return node->frameData(frame).rotation();
  }
  qDebug("Animation::getRotation(): jointName==0!");
  return Rotation();
}

void Animation::useRotationLimits(bool flag)
{
  limits = flag;
  ikTree.setJointLimits(flag);
}

RotationLimits Animation::getRotationLimits(const QString& jointName)
{
  if(!jointName.isEmpty())
  {
    double xMin,yMin,zMin,xMax,yMax,zMax;

    if (limits)
    {
      BVHNode* node=bvh->bvhFindNode(frames,jointName);
      if (node) {
        bvh->bvhGetChannelLimits(node,BVH_XROT,&xMin,&xMax);
        bvh->bvhGetChannelLimits(node,BVH_YROT,&yMin,&yMax);
        bvh->bvhGetChannelLimits(node,BVH_ZROT,&zMin,&zMax);
      }
    }
    else
    {
      xMin=yMin=zMin=-180;
      xMax=yMax=zMax=180;
    }

    RotationLimits rotLimit(jointName,xMin,xMax,yMin,yMax,zMin,zMax);
    return rotLimit;
  }
  qDebug("Animation::getRotationLimits(): jointName==0!");
  return RotationLimits(QString::null,0,0,0,0,0,0);
}

int Animation::getRotationOrder(const QString& jointName)
{
  BVHNode *node = bvh->bvhFindNode(frames, jointName);
  if (node) {
    return node->channelOrder;
  }
  return 0;
}

void Animation::setPosition(const QString& jointName,double x,double y,double z)
{
  BVHNode *node = bvh->bvhFindNode(frames, jointName);
  if (node) {
    for (int i=0; i<NUM_IK; i++) if (ikOn[i]) { solveIK(); break; }
    // new keyframe system
    if(node->isKeyframe(frame))
      node->setKeyframePosition(frame,Position(x,y,z));
    else
    {
      node->addKeyframe(frame,Position(x,y,z),node->frameData(frame).rotation());
    }
    setDirty(true);
    // tell timeline that this keyframe has changed (added or changed is the same here)
    emit redrawTrack(getPartIndex(jointName));
    emit frameChanged();
  }
}

Position Animation::getPosition(const QString& jointName)
{
  BVHNode *node = bvh->bvhFindNode(frames, jointName);

  if (node)
    return node->frameData(frame).position();

  return Position();
}

const QString& Animation::getPartName(int index) const
{
  return bvh->bvhGetName(frames, index);
}

int Animation::getPartIndex(const QString& part)
{
  return bvh->bvhGetIndex(frames, part);
}

BVHNode* Animation::getMotion()
{
  return frames;
}

BVHNode* Animation::getEndSite(const QString& rootName)
{
  BVHNode *node = bvh->bvhFindNode(frames, rootName);
  while (node && node->numChildren() > 0) {
    node = node->child(0);
  }
  return node;
}

void Animation::recursiveAddKeyFrame(BVHNode *joint)
{
  if(joint->type!=BVH_END)
    addKeyFrame(joint);

  for (int i=0; i < joint->numChildren(); i++)
    recursiveAddKeyFrame(joint->child(i));

  setDirty(true);
}

void Animation::addKeyFrameAllJoints()
{
  recursiveAddKeyFrame(frames);
}

void Animation::addKeyFrame(BVHNode *joint)
{
  joint->addKeyframe(frame,getPosition(joint->name()),getRotation(joint->name()));

  setDirty(true);

  emit redrawTrack(getPartIndex(joint->name()));
  emit frameChanged();
}

bool Animation::isKeyFrameHelper(BVHNode *joint) {
  if (joint->isKeyframe(frame))
    return true;

  for (int i=0;i<joint->numChildren();i++) {
    if (isKeyFrameHelper(joint->child(i))) {
      return true;
    }
  }

  return false;
}

bool Animation::isKeyFrame(const QString& jointName)
{
  if(jointName.isEmpty())
     return isKeyFrame();
  else
  {
    BVHNode *node = bvh->bvhFindNode(frames, jointName);
    return node->isKeyframe(frame);
  }
}

bool Animation::isKeyFrame()
{
  return isKeyFrameHelper(frames);
}

bool Animation::isKeyFrame(int jointNumber,int frame)
{
  const BVHNode* joint=getNode(jointNumber);
  return joint->isKeyframe(frame);
}

void Animation::delKeyFrame(BVHNode *joint,bool silent)
{
  // never delete first keyframe
  if(frame)
  {
    joint->deleteKeyframe(frame);
    setDirty(true);
  }

  // if silent is true then only send a signal to the timeline but not to the animation view
  if(!silent) emit frameChanged();
  emit redrawTrack(getPartIndex(joint->name()));
}

void Animation::delKeyFrame(int jointNumber,int frame)
{
  // frames should always be current frame, but better play safe for future enhancements
  setFrame(frame);
  if(jointNumber)
  {
     BVHNode* joint=getNode(jointNumber);
     if(joint->isKeyframe(frame)) delKeyFrame(joint);
  }
  else if(isKeyFrame()) delKeyFrameAllJoints();

  // tell main class that the keyframe has changed
  emit currentFrame(frame);
}

void Animation::recursiveDelKeyFrame(BVHNode *joint) {
  delKeyFrame(joint);

  for (int i=0;i<joint->numChildren();i++)
    recursiveDelKeyFrame(joint->child(i));
}

void Animation::delKeyFrameAllJoints() {
  // never delete the first keyframe
  if(frame==0) return;
  recursiveDelKeyFrame(frames);
}

bool Animation::toggleKeyFrame(const QString& jointName)
{
  if(jointName.isEmpty())
    return toggleKeyFrameAllJoints();
  else
  {
    BVHNode *node = bvh->bvhFindNode(frames, jointName);

    if (node->isKeyframe(frame)) {
      delKeyFrame(node);
      return false;
    } else {
      addKeyFrame(node);
      return true;
    }
  }
}

// returns TRUE if frame is now a keyframe for entire animation, FALSE if not
bool Animation::toggleKeyFrameAllJoints() {
  if(frame==0)
    return true;  // first frame will always stay keyframe

  if (isKeyFrame()) {
    delKeyFrameAllJoints();
    return false;
  } else {
    addKeyFrameAllJoints();
    return true;
  }
}

void Animation::cutFrame()
{
  // copy frame data into copy buffer
  copyFrame();
  // always delete frame from all tracks
  deleteFrame(0,frame);
}

void Animation::copyFrame()
{
  bvh->bvhGetFrameData(frames,frame);
}

void Animation::pasteFrame()
{
  bvh->bvhSetFrameData(frames,frame);
  addKeyFrameAllJoints();
}

void Animation::calcPartMirrors()
{
  int i = 1;
  char name[256];
  const char *n;
  partMirror[0] = 0;  // part indices start at 1
  while ((n = bvh->bvhGetName(frames, i))) {
    strcpy(name, n);
    if (n[0] == 'r') name[0] = 'l';
    else name[0] = 'r';  // if n doesn't start with l, there will be no match,
                         // which is what we want since partMirror will be 0.
    partMirror[i++] = bvh->bvhGetIndex(frames, name);
  }
}

const QString& Animation::getPartMirror(const QString& name) const
{
  int index = bvh->bvhGetIndex(frames, name);
  if (index) index = partMirror[index];
  return bvh->bvhGetName(frames, index);
}

const int Animation::numKeyFrames(int jointNumber)
{
  BVHNode* node=bvh->bvhFindNode(frames,getPartName(jointNumber));
//  qDebug(QString("joint number %1 has %2 keyframes").arg(jointNumber).arg(node->numKeyFrames));
  return node->numKeyframes();
}

// copies the position and rotation data of one body part to another key frame position
void Animation::copyKeyFrame(int jointNumber,int from,int to)
{
  // move keyframe in copy mode
  moveKeyFrame(jointNumber,from,to,true);
}

// moves the position and rotation data of one body part to another key frame position
void Animation::moveKeyFrame(int jointNumber,int from,int to,bool copy)
{
  // set frame pointer to source frame position
  setFrame(from);

  // get the joint structure
  BVHNode* joint=getNode(jointNumber);
  const FrameData& frameData=joint->frameData(frame);

  // get rotation and position of the body part
  Rotation rot=frameData.rotation();
  Position pos=frameData.position();

  // silently (true) delete key frame if not copy mode
  // we do copy mode here to avoid code duplication
  if(!copy) delKeyFrame(joint,true);

  // block all further signals to avoid flickering
  blockSignals(true);
  setFrame(to);
  setRotation(joint->name(),rot.x,rot.y,rot.z);
  joint->setEaseIn(frame,frameData.easeIn());
  joint->setEaseOut(frame,frameData.easeOut());
  blockSignals(false);
  // now re-enable signals so we get updates on screen
  setPosition(joint->name(),pos.x,pos.y,pos.z);
  // tell timeline where we are now
  emit currentFrame(frame);
}

bool Animation::compareFrames(const QString& jointName,int key1,int key2)
{
  BVHNode *node=bvh->bvhFindNode(frames,jointName);
  if(node) return node->compareFrames(key1,key2);
  return false;
}

const FrameData Animation::keyframeDataByIndex(int jointNumber,int index)
{
  BVHNode* joint=getNode(jointNumber);
  return joint->keyframeDataByIndex(index);
}

BVHNode* Animation::getNode(int jointNumber)
{
  // get the joint structure
  return bvh->bvhFindNode(frames,getPartName(jointNumber));
}

void Animation::insertFrameHelper(BVHNode* joint,int frame)
{
  joint->insertFrame(frame);
  for(int i=0;i<joint->numChildren();i++)
    insertFrameHelper(joint->child(i),frame);
}

void Animation::insertFrame(int track,int pos)
{
  if(track==0)
    insertFrameHelper(frames,pos);
  else
  {
    BVHNode* joint=getNode(track);
    if(joint) joint->insertFrame(frame);
  }
  setDirty(true);
  emit frameChanged();
}

// recursively remove frames from joint and all its children
void Animation::deleteFrameHelper(BVHNode* joint,int frame)
{
//  qDebug("Animation::deleteFrameHelper(joint %s,frame %d)",joint->name().latin1(),frame);
  joint->deleteFrame(frame);
  for(int i=0;i<joint->numChildren();i++)
    deleteFrameHelper(joint->child(i),frame);
  emit redrawTrack(getPartIndex(joint->name()));
}

// delete frame from a joint, if track==0 rdelete from all joints
void Animation::deleteFrame(int track,int pos)
{
//  qDebug("Animation::deleteFrame(joint %d,frame %d)",track,frame);

  if(track==0)
    deleteFrameHelper(frames,pos);
  else
  {
    BVHNode* joint=getNode(track);
    if(joint) joint->deleteFrame(pos);
  }
  setDirty(true);
  emit frameChanged();
}

void Animation::optimizeHelper(BVHNode* joint)
{
  if(joint->type!=BVH_END)
  {
    joint->optimize();
    emit redrawTrack(getPartIndex(joint->name()));
  }

  for(int i=0;i<joint->numChildren();i++)
    optimizeHelper(joint->child(i));
}

void Animation::optimize()
{
  optimizeHelper(frames);
  setDirty(true);
}

bool Animation::dirty() const
{
  return isDirty;
}

void Animation::setDirty(bool state)
{
  isDirty=state;
  emit animationDirty(state);
}

float Animation::getAvatarScale()
{
  return avatarScale;
}

void Animation::setAvatarScale(float newScale)
{
  avatarScale=newScale;
}
