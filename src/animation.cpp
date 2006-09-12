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

#include <stdio.h>
#include <string.h>
// #include "main.h"
#include "animation.h"
#include "bvh.h"
#include "rotation.h"

Animation::Animation(const char *bvhFile) :
  frame(0), mirrored(false)
{
  QString fileName;

  execPath=qApp->applicationDirPath();
  // load BVH that defines motion
  if (bvhFile)
    fileName=bvhFile;
  else
    fileName=execPath+"/"+DEFAULT_POSE;

  loadBVH(fileName);
  calcPartMirrors();
  useRotationLimits(true);

  ikTree.set(frames);
  setIK(IK_LHAND, false);
  setIK(IK_RHAND, false);
  setIK(IK_LFOOT, false);
  setIK(IK_RFOOT, false);
}

Animation::~Animation() {
  if (frames) {
    bvhDelete(frames);
    frames = NULL;
  }
}

void Animation::loadBVH(const char *bvhFile)
{
  QString limFile=execPath+"/"+LIMITS_FILE;
  frames = animRead(bvhFile, limFile);
  setFrame(0);
}

void Animation::saveBVH(const char *bvhFile)
{
  animWrite(frames, bvhFile);
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
    bvhSetFrameTime(frames, frameTime);
}

void Animation::setNumberOfFramesHelper(BVHNode *joint, int num)
{
  int currNum = joint->numFrames;

  if (num > currNum) {
    for (int i=currNum; i < num; i++) {
      for (int c=0; c < joint->numChannels; c++) {
        joint->frame[i][c] = joint->frame[currNum-1][c];
      }
    }
  }
  joint->numFrames = num;
  for (int i=0; i < joint->numChildren; i++)
    setNumberOfFramesHelper(joint->child[i], num);
}

void Animation::setNumberOfFrames(int num)
{
  setNumberOfFramesHelper(frames, num);
  emit numberOfFrames(num);
}

void Animation::setFrame(int frameNumber)
{
  if (frameNumber >= 0 && frameNumber < frames->numFrames &&
      frame != frameNumber) {
    for (int i=0; i<NUM_IK; i++) {
      setIK((IKPartType)i, false);
    }
    frame = frameNumber;

    emit currentFrame(frame);
    emit frameChanged();
  }
}

int Animation::stepForward()
{
  if (frames)
  {
    int nextFrame=(frame + 1) % frames->numFrames;
    if(!nextFrame) nextFrame=loopingPoint;
//    if(protectFirst && nextFrame==0) nextFrame=1;
    setFrame(nextFrame);
    return nextFrame;
  }
  else
    return 0;
}

void Animation::setLoopPoint(int frame)
{
  loopingPoint=frame;
}

int Animation::loopPoint()
{
  return loopingPoint;
}

void Animation::applyIK(const char *name)
{
  BVHNode *node = bvhFindNode(frames, name);
  if (node) {
    for (int i=0; i<3; i++) {
      node->frame[frame][i] += node->ikRot[i];
      node->ikRot[i] = 0;
      node->ikOn = false;
      addKeyFrame(node);
    }
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

void Animation::setIK(const char *jointName, bool flag)
{
  if (!strcmp(jointName, "lHand")||
      !strcmp(jointName, "lForeArm")||
      !strcmp(jointName, "lShldr")||
      !strcmp(jointName, "lCollar")) {
    setIK(IK_LHAND, flag);
  }
  else if (!strcmp(jointName, "rHand")||
	   !strcmp(jointName, "rForeArm")||
	   !strcmp(jointName, "rShldr")||
	   !strcmp(jointName, "rCollar")) {
    setIK(IK_RHAND, flag);
  }
  else if (!strcmp(jointName, "lThigh")||
	   !strcmp(jointName, "lShin")||
	   !strcmp(jointName, "lFoot")) {
    setIK(IK_LFOOT, flag);
  }
  else if (!strcmp(jointName, "rThigh")||
	   !strcmp(jointName, "rShin")||
	   !strcmp(jointName, "rFoot")) {
    setIK(IK_RFOOT, flag);
  }
}

bool Animation::getIK(const char *jointName)
{
  if (!strcmp(jointName, "lHand")||
      !strcmp(jointName, "lForeArm")||
      !strcmp(jointName, "lShldr")||
      !strcmp(jointName, "lCollar")) {
    return getIK(IK_LHAND);
  }
  else if (!strcmp(jointName, "rHand")||
	   !strcmp(jointName, "rForeArm")||
	   !strcmp(jointName, "rShldr")||
	   !strcmp(jointName, "rCollar")) {
    return getIK(IK_RHAND);
  }
  else if (!strcmp(jointName, "lThigh")||
	   !strcmp(jointName, "lShin")||
	   !strcmp(jointName, "lFoot")) {
    return getIK(IK_LFOOT);
  }
  else if (!strcmp(jointName, "rThigh")||
	   !strcmp(jointName, "rShin")||
	   !strcmp(jointName, "rFoot")) {
    return getIK(IK_RFOOT);
  }
  return false;
}

void Animation::solveIK()
{
  bvhResetIK(frames);
  if (ikOn[IK_LFOOT]) getEndSite("lFoot")->ikOn = true;
  if (ikOn[IK_RFOOT]) getEndSite("rFoot")->ikOn = true;
  if (ikOn[IK_LHAND]) getEndSite("lHand")->ikOn = true;
  if (ikOn[IK_RHAND]) getEndSite("rHand")->ikOn = true;

  ikTree.setJointLimits(true);
  ikTree.solve(frame);
}

void Animation::setRotation(const char *jointName, double x, double y, double z)
{
  BVHNode *node = bvhFindNode(frames, jointName);
  BVHNode *node2 = NULL;
  const char *mirrorName;
  if (node) {
    bvhSetChannel(node, frame, BVH_XROT, x);
    bvhSetChannel(node, frame, BVH_YROT, y);
    bvhSetChannel(node, frame, BVH_ZROT, z);
    if (mirrored && (mirrorName = getPartMirror(jointName))) {
      node2 = bvhFindNode(frames, mirrorName);
      bvhSetChannel(node2, frame, BVH_XROT, x);
      bvhSetChannel(node2, frame, BVH_YROT, -y);
      bvhSetChannel(node2, frame, BVH_ZROT, -z);
    }
    for (int i=0; i<NUM_IK; i++) if (ikOn[i]) { solveIK(); break; }
    addKeyFrame(node);
    if (node2) addKeyFrame(node2);
  }
}

Rotation Animation::getRotation(const char* jointName)
{
  double x,y,z;

  BVHNode *node = bvhFindNode(frames, jointName);
  if (node) {
    x = bvhGetChannel(node, frame, BVH_XROT);
    y = bvhGetChannel(node, frame, BVH_YROT);
    z = bvhGetChannel(node, frame, BVH_ZROT);
  }
  else {
    x = y = z = 0;
  }

  Rotation rot(jointName,x,y,z);

  return rot;
}

void Animation::useRotationLimits(bool flag)
{
  limits = flag;
  ikTree.setJointLimits(flag);
}

RotationLimits Animation::getRotationLimits(const char *jointName)
{
  double xMin,yMin,zMin,xMax,yMax,zMax;

  if (limits) {
    BVHNode *node = bvhFindNode(frames, jointName);
    if (node) {
      bvhGetChannelLimits(node, BVH_XROT, &xMin, &xMax);
      bvhGetChannelLimits(node, BVH_YROT, &yMin, &yMax);
      bvhGetChannelLimits(node, BVH_ZROT, &zMin, &zMax);
    }
  }
  else {
    xMin = yMin = zMin = -180;
    xMax = yMax = zMax = 180;
  }

  RotationLimits rotLimit(jointName,xMin,xMax,yMin,yMax,zMin,zMax);
  return rotLimit;
}

int Animation::getRotationOrder(const char *jointName)
{
  BVHNode *node = bvhFindNode(frames, jointName);
  if (node) {
    return node->channelOrder;
  }
  return 0;
}

void Animation::setPosition(const char *jointName, double x, double y, double z)
{
  BVHNode *node = bvhFindNode(frames, jointName);
  if (node) {
    bvhSetChannel(node, frame, BVH_XPOS, x);
    bvhSetChannel(node, frame, BVH_YPOS, y);
    bvhSetChannel(node, frame, BVH_ZPOS, z);
    for (int i=0; i<NUM_IK; i++) if (ikOn[i]) { solveIK(); break; }
    addKeyFrame(node);
  }
}

Position Animation::getPosition(const char *jointName)
{
  double x,y,z;

  BVHNode *node = bvhFindNode(frames, jointName);
  if (node) {
    x = bvhGetChannel(node, frame, BVH_XPOS);
    y = bvhGetChannel(node, frame, BVH_YPOS);
    z = bvhGetChannel(node, frame, BVH_ZPOS);
  }
  else {
    x = y = z = 0;
  }

  Position pos(jointName,x,y,z);
  return pos;
}

const char *Animation::getPartName(int index)
{
  return bvhGetName(frames, index);
}

int Animation::getPartIndex(const char *part)
{
  return bvhGetIndex(frames, part);
}

BVHNode *Animation::getEndSite(const char *rootName)
{
  BVHNode *node = bvhFindNode(frames, rootName);
  while (node && node->numChildren > 0) {
    node = node->child[0];
  }
  return node;
}

bool Animation::isSecondLifeJoint(BVHNode *joint)
{
  return !(!strcmp(joint->name, "neckDummy") ||
	   !strcmp(joint->name, "figureHair"));
}

void Animation::addKeyFrameHelper(BVHNode *joint)
{
  addKeyFrame(joint);
  for (int i=0; i < joint->numChildren; i++)
    addKeyFrameHelper(joint->child[i]);
}

void Animation::addKeyFrame()
{
  addKeyFrameHelper(frames);
}

void Animation::addKeyFrame(BVHNode *joint)
{
  bool newKey = true;
  int numKeyFrames = joint->numKeyFrames;
  int i, *kf = joint->keyFrames;

  if (frame == 0) {
    // first and last frames are automatic key frames, don't add again
    newKey = false;
  }
  else {
    for (i=0; i<numKeyFrames; i++) {
      if (*kf == frame) { newKey = false; break; }
      if (*kf > frame) break;
      kf++;
    }
  }
  if (newKey) {
    memmove(kf+1, kf, sizeof(int)*(numKeyFrames - i));
    *kf = frame;
    joint->numKeyFrames++;
    //numKeyFrames++;
  }

  interpolateFrames(joint);
  emit keyframeAdded(getPartIndex(joint->name),frame);
  emit frameChanged();
}

// Re-calculate intermediate frames based on the values in keyframes
void Animation::interpolateFrames(BVHNode *joint) {
  int *kf;
  double start, end, step;
  int i, len, last;

  for (int c = 0; c < joint->numChannels; c++) {
    start = joint->frame[0][c];
    last = 0;
    kf = joint->keyFrames;
    for (i=0; i<=joint->numKeyFrames; i++) {
      if (i == joint->numKeyFrames) {  // last frame
	if (i > 0 && *(kf-1) == joint->numFrames-1)
	  break;  // last keyframe is last frame, we're done
	end = joint->frame[joint->numFrames-1][c] = start;
	len = joint->numFrames - last - 2;
      }
      else {
	end = joint->frame[*kf][c];
	len = *kf - last - 1;
      }
      if (len > 0) {
	step = (end - start) / (len + 1);
	for (int j = last + 1; j <= last + len; j++) {
	  start += step;
	  joint->frame[j][c] = start;
	}
      }
      start = joint->frame[*kf][c];
      last = *kf;
      kf++;
    }
  }
}

bool Animation::isKeyFrameHelper(BVHNode *joint) {
  if (isKeyFrame(joint))
    return true;

  for (int i=0;i<joint->numChildren;i++) {
    if (isKeyFrameHelper(joint->child[i])) {
      return true;
    }
  }

  return false;
}

bool Animation::isKeyFrame(BVHNode *joint) {
  for (int i=0;i<joint->numKeyFrames;i++) {
    if (joint->keyFrames[i] == frame) {
      return true;
    } else if (joint->keyFrames[i] > frame) {
      break;
    }
  }

  return false;
}

bool Animation::isKeyFrame(const char *jointName) {
  if (frame == 0 || frame == (getNumberOfFrames() - 1))
    return true;

  if (jointName == NULL) {
     return isKeyFrame();
  } else {
    BVHNode *node = bvhFindNode(frames, jointName);

    return isKeyFrame(node);
  }
}

bool Animation::isKeyFrame() {
  // first and last frames are automatic keyframes
  if (frame == 0 || frame == (getNumberOfFrames() - 1))
    return true;

  return isKeyFrameHelper(frames);
}

// if silent is true then only send a signal to the timeline but not to the animation view
void Animation::delKeyFrame(BVHNode *joint,bool silent) {
  bool found = false;
  int numKeyFrames = joint->numKeyFrames;
  int i, *kf = joint->keyFrames;

  if (frame == 0) {
    // first and last frames are automatic key frames, don't delete
    found = false;
  } else {
    for (i=0; i<numKeyFrames; i++) {
      if (*kf == frame) { found = true; break; }
      if (*kf > frame) break;
      kf++;
    }
  }
  if (found) {
    memmove(kf, kf+1, sizeof(int)*(numKeyFrames - i - 1));
    joint->numKeyFrames--;
    //numKeyFrames++;
  }

  interpolateFrames(joint);
  emit keyframeRemoved(getPartIndex(joint->name),frame);
  if(!silent) emit frameChanged();
}

void Animation::delKeyFrameHelper(BVHNode *joint) {
  delKeyFrame(joint);

  for (int i=0;i<joint->numChildren;i++)
    delKeyFrameHelper(joint->child[i]);
}

void Animation::delKeyFrame() {
  // first and last frames are ALWAYS automatic keyframes
  if (frame == 0 || frame == (getNumberOfFrames() - 1))
    return;

  delKeyFrameHelper(frames);
}

bool Animation::toggleKeyFrame(const char *jointName) {
  if (jointName == NULL) {
    return toggleKeyFrame();
  } else {
    BVHNode *node = bvhFindNode(frames, jointName);

    if (isKeyFrame(node)) {
      delKeyFrame(node);
      return false;
    } else {
      addKeyFrame(node);
      return true;
    }
  }
}

// returns TRUE if frame is now a keyframe for entire animation, FALSE if not
bool Animation::toggleKeyFrame() {
  if (frame == 0 || frame == (getNumberOfFrames() - 1))
    return true;  // first and last frames will always stay keyframes

  if (isKeyFrame()) {
    delKeyFrame();
    return false;
  } else {
    addKeyFrame();
    return true;
  }
}

void Animation::getFrameData(double *data)
{
  bvhGetFrameData(frames, frame, data);
}

void Animation::setFrameData(double *data)
{
  bvhSetFrameData(frames, frame, data);
  addKeyFrame();
}

void Animation::calcPartMirrors()
{
  int i = 1;
  char name[256];
  const char *n;
  partMirror[0] = 0;  // part indices start at 1
  while ((n = bvhGetName(frames, i))) {
    strcpy(name, n);
    if (n[0] == 'r') name[0] = 'l';
    else name[0] = 'r';  // if n doesn't start with l, there will be no match,
                         // which is what we want since partMirror will be 0.
    partMirror[i++] = bvhGetIndex(frames, name);
  }
}

const char *Animation::getPartMirror(const char *name)
{
  int index = bvhGetIndex(frames, name);
  if (index) index = partMirror[index];
  return bvhGetName(frames, index);
}

const int* Animation::keyFrames(int jointNumber)
{
  const char* jointName=getPartName(jointNumber);
  BVHNode* node=bvhFindNode(frames,jointName);
  return node->keyFrames;
}

const int Animation::numKeyFrames(int jointNumber)
{
  const char* jointName=getPartName(jointNumber);
  BVHNode* node=bvhFindNode(frames,jointName);
//  qDebug(QString("joint number %1 has %2 keyframes").arg(jointNumber).arg(node->numKeyFrames));
  return node->numKeyFrames;
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
  const char* jointName=getPartName(jointNumber);
  BVHNode* joint=bvhFindNode(frames,jointName);

  // get rotation and position of the body part
  Rotation rot=getRotation(jointName);
  Position pos=getPosition(jointName);

  // silently (true) delete key frame if not copy mode
  // we do copy mode here to avoid code duplication
  if(!copy) delKeyFrame(joint,true);

  // block all further signals to avoid flickering
  blockSignals(true);
  setFrame(to);
  setRotation(jointName,rot.x,rot.y,rot.z);
  blockSignals(false);
  // now re-enable signals so we get updates on screen
  setPosition(jointName,pos.x,pos.y,pos.z);
  // tell timeline where we are now
  emit currentFrame(frame);
}

bool Animation::compareFrames(const char* jointName,int key1,int key2)
{
  double x1,y1,z1;
  double x2,y2,z2;

  BVHNode *node=bvhFindNode(frames,jointName);
  if(node)
  {
    x1=bvhGetChannel(node,key1,BVH_XROT);
    y1=bvhGetChannel(node,key1,BVH_YROT);
    z1=bvhGetChannel(node,key1,BVH_ZROT);
    x2=bvhGetChannel(node,key2,BVH_XROT);
    y2=bvhGetChannel(node,key2,BVH_YROT);
    z2=bvhGetChannel(node,key2,BVH_ZROT);

    if(x1!=x2) return false;
    if(y1!=y2) return false;
    if(z1!=z2) return false;

    x1=bvhGetChannel(node,key1,BVH_XPOS);
    y1=bvhGetChannel(node,key1,BVH_YPOS);
    z1=bvhGetChannel(node,key1,BVH_ZPOS);
    x2=bvhGetChannel(node,key2,BVH_XPOS);
    y2=bvhGetChannel(node,key2,BVH_YPOS);
    z2=bvhGetChannel(node,key2,BVH_ZPOS);

    if(x1!=x2) return false;
    if(y1!=y2) return false;
    if(z1!=z2) return false;
  }

  return true;
}
