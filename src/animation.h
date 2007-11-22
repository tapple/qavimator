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

#include "iktree.h"
#include "rotation.h"

#define DEFAULT_POSE "data/TPose.avm"
// #define DEFAULT_POSE "data/Relaxed.bvh"
#define LIMITS_FILE "data/SL.lim"

class BVH;

class Animation : public QObject
{
  Q_OBJECT

  public:
    typedef enum { IK_LHAND=0, IK_RHAND, IK_LFOOT, IK_RFOOT, NUM_IK } IKPartType;
    typedef enum { FIGURE_MALE=0, FIGURE_FEMALE,NUM_FIGURES } FigureType;

    Animation(BVH* bvh,const QString& bvhFile=QString::null);
    ~Animation();

    void loadBVH(const QString& bvhFile);
    void saveBVH(const QString& bvhFile);
    double frameTime();
    int getNumberOfFrames();
    void setNumberOfFrames(int num);
    int getFrame();
    void setFrame(int frameNumber);
    int stepForward();
    void setLoopPoint(int frame);
    int loopPoint();
    void setIK(const QString& jointName, bool flag);
    bool getIK(const QString& jointName);
    const QString& getPartName(int index) const;
    int getPartIndex(const QString& part);
    void setMirrored(bool mirror);
    bool getMirrored();
    unsigned int getPartMirror(int index);
    const QString& getPartMirror(const QString& name) const;

    float getAvatarScale();
    void setAvatarScale(float newScale);

    FigureType getFigureType();
    void setFigureType(FigureType type);

    BVHNode* getMotion();
    BVHNode* getEndSite(const QString& siteParentName);
    BVHNode* getNode(int jointNumber);

    void cutFrame();
    void copyFrame();
    void pasteFrame();

    bool dirty() const;
    void setDirty(bool state);

    const FrameData keyframeDataByIndex(int jointNumber,int index);

    void setRotation(const QString& jointName,double x,double y,double z);
    Rotation getRotation(const QString& jointName);
    void useRotationLimits(bool flag);
    RotationLimits getRotationLimits(const QString& jointName);
    void setPosition(const QString& jointName,double x,double y,double z);
    Position getPosition(const QString& jointName);
    int getRotationOrder(const QString& jointName);
    void addKeyFrameAllJoints();
    void addKeyFrame(BVHNode *joint);
    bool isKeyFrame();
    bool isKeyFrame(const QString& jointName);
    bool isKeyFrame(int jointNumber,int frame);
    void delKeyFrameAllJoints();
    void delKeyFrame(BVHNode *joint,bool silent=false); // silent = only send signal to timeline
    bool toggleKeyFrameAllJoints();
    bool toggleKeyFrame(const QString& jointName);
    void setFrameTime(double frameTime);

    void setEaseIn(const QString& jointName,bool state);
    void setEaseOut(const QString& jointName,bool state);
    bool easeIn(const QString& jointName);
    bool easeOut(const QString& jointName);

    const int numKeyFrames(int jointNumber);
    void copyKeyFrame(int jointNumber,int from,int to);
    void moveKeyFrame(int jointNumber,int from,int to,bool copy=false);

    bool compareFrames(const QString& jointName,int key1,int key2);

    void optimize();

    enum { MAX_PARTS = 64 };

  public slots:
    void delKeyFrame(int jointNumber,int frame);
    void insertFrame(int jointNumber,int frame);
    void deleteFrame(int jointNumber,int frame);

  signals:
    void numberOfFrames(int num);
    void currentFrame(int frame);
    void frameChanged();
    void redrawTrack(int track);
    void animationDirty(bool state);

  protected:
    BVH* bvh;
    BVHNode *frames;

    FigureType figureType;

    // this flag shows if the animation has been worked on and not yet saved
    bool isDirty;

    // display the avatar at another scale (1.0 is default)
    float avatarScale;

    int frame;
    int totalFrames;
    int loopingPoint;

    bool mirrored;
    unsigned int partMirror[ MAX_PARTS + 1 ];
    bool limits;
    bool ikOn[NUM_IK];
    IKTree ikTree;

    void recursiveAddKeyFrame(BVHNode *joint);
    bool isKeyFrameHelper(BVHNode *joint);
    void recursiveDelKeyFrame(BVHNode *joint);
    void insertFrameHelper(BVHNode* joint,int frame);
    void deleteFrameHelper(BVHNode* joint,int frame);
    void optimizeHelper(BVHNode* joint);

    void calcPartMirrors();
    void setIK(IKPartType part, bool flag);
    bool getIK(IKPartType part);
    void applyIK(const QString& name);
    void solveIK();

    QString execPath;
};

#endif
