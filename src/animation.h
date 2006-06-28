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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <qobject.h>

#include "bvh.h"
#include "iktree.h"
#include "rotation.h"

//#define DEFAULT_POSE "data/TPose.avm"
#define DEFAULT_POSE "data/Relaxed.avm"
#define LIMITS_FILE "data/SL.lim"

class Animation : public QObject
{
  Q_OBJECT

  public:
    typedef enum { IK_LHAND=0, IK_RHAND, IK_LFOOT, IK_RFOOT, NUM_IK } IKPartType;

  Animation(const char *bvhFile = NULL);
  ~Animation();
  void loadBVH(const char *bvhFile);
  void saveBVH(const char *bvhFile);
  double frameTime();
  int getNumberOfFrames() { return (frames ? frames->numFrames : 0); }
  void setNumberOfFrames(int num);
  int getFrame() { return frame; }
  void setFrame(int frameNumber);
  int stepForward();
  void setIK(const char *jointName, bool flag);
  bool getIK(const char *jointName);
  const char *getPartName(int index);
  int getPartIndex(const char *part);
  void setMirrored(bool mirror) { mirrored = mirror; }
  bool getMirrored() { return mirrored; }
  int getPartMirror(int index) { return partMirror[index]; }
  const char *getPartMirror(const char *name);
  BVHNode *getMotion() { return frames; }
  BVHNode *getEndSite(const char *siteParentName);
  void getFrameData(double *data);
  void setFrameData(double *data);
  void setRotation(const char *jointName, double x, double y, double z);
  Rotation getRotation(const char *jointName);
  void useRotationLimits(bool flag);
  bool useRotationLimits() { return limits; }
  RotationLimits getRotationLimits(const char *jointName);
  void setPosition(const char *jointName, double x, double y, double z);
  Position getPosition(const char *jointName);
  int getRotationOrder(const char *jointName);
  void addKeyFrame();
  void addKeyFrame(BVHNode *joint);
  bool isKeyFrame();
  bool isKeyFrame(BVHNode *joint);
  bool isKeyFrame(const char *jointName);
  void delKeyFrame();
  void delKeyFrame(BVHNode *joint);
  bool toggleKeyFrame();
  bool toggleKeyFrame(const char *jointName);
  void setFrameTime(double frameTime);

  static bool isSecondLifeJoint(BVHNode *joint);

  enum { MAX_PARTS = 64 };

  signals:
    void currentFrame(int frame);

 private:
  BVHNode *frames;
  int frame;
  bool mirrored;
  int partMirror[ MAX_PARTS + 1 ];
  bool limits;
  bool ikOn[NUM_IK];
  IKTree ikTree;

  void setNumberOfFramesHelper(BVHNode *joint, int num);
  void addKeyFrameHelper(BVHNode *joint);
  bool isKeyFrameHelper(BVHNode *joint);
  void delKeyFrameHelper(BVHNode *joint);
  void interpolateFrames(BVHNode *joint);
  void calcPartMirrors();
  void setIK(IKPartType part, bool flag);
  bool getIK(IKPartType part) { return ikOn[part]; }
  void applyIK(const char *name);
  void solveIK();

  QString execPath;
};

#endif

