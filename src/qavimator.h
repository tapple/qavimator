/***************************************************************************
 *   Copyright (C) 2006 by Zi Ree   *
 *   Zi Ree @ SecondLife   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*
 * features added by Darkside Eldrich
 */

#ifndef QAVIMATOR_H
#define QAVIMATOR_H

#include <qtimer.h>
#include <qfileinfo.h>

#define UNTITLED_NAME "Untitled.avm"
#define PLAY_IMAGE "data/play.png"
#define PAUSE_IMAGE "data/pause.png"
#define KEY_IMAGE "data/key.png"
#define NOKEY_IMAGE "data/nokey.png"

#include "mainapplicationform.h"
#include "rotation.h"

class Animation;
class Prop;
class Timeline;

class qavimator : public MainApplicationForm
{
  Q_OBJECT

  public:
    qavimator();
    ~qavimator();

  signals:
    void enableRotation(bool state);
    void enablePosition(bool state);
    void enableProps(bool state);
    void enableEaseInOut(bool state);
    void resetCamera();
    void protectFrame(bool state);

  public slots:
    // prevent closing of main window if there are unsaved changes
    bool close(bool deDelete);

  protected slots:
    void readSettings();
    void configChanged();

    void partClicked(const QString& partName,Rotation rot,RotationLimits rotLimits,Position pos);
    void partDragged(const QString& partName,double changeX,double changeY,double changeZ);
    void propClicked(Prop* prop);
    void propDragged(Prop* prop,double x,double y,double z);
    void propScaled(Prop* prop,double x,double y,double z);
    void propRotated(Prop* prop,double x,double y,double z);
    void backgroundClicked();

    void helpAbout();

    void cb_PartChoice();
    void cb_RotRoller(int dummy);
    void cb_RotValue();
    void cb_PosRoller(int dummy);
    void cb_PosValue();

    void propPosChanged(int dummy);
    void propScaleChanged(int dummy);
    void propRotChanged(int dummy);

    void cb_timeout();
    void cb_PlayBtn();
    void cb_fpsValue(int fps);
    void cb_FrameSlider(int position);

    void easeInChanged(int change);
    void easeOutChanged(int change);

    void animationChanged(int which);

    void figureChanged(int shape);
    void scaleChanged(int percent);
    void numFramesChanged(int num);
    void keyframeButtonToggled(bool on);

    void setCurrentFrame(int frame);
    void setLoopInPoint(int inFrame);
    void setLoopOutPoint(int outFrame);

    // ------- Menu Action Slots (Callbacks) --------
    void fileNew();
    void fileOpen();
    void fileOpen(const QString& fileName);
    // "add" a new file without clearing the old one(s)
    void fileAdd();
    void fileAdd(const QString& fileName);
    void fileSave();
    void fileSaveAs();
    void fileLoadProps();
    void fileSaveProps();
    void fileExit();

    void editCut();
    void editCopy();
    void editPaste();
    void editOptimize();

    void optionsSkeleton(bool on);
    void optionsJointLimits(bool on);
    void optionsLoop(bool on);
    void optionsProtectFirstFrame(bool on);
    void optionsShowTimeline(bool state);
    void optionsConfigure();

    void newPropButtonClicked();
    void attachToComboBoxChanged(int attachPoint);
    void deletePropButtonClicked();
    void selectProp(const QString& name);
    void selectAnimation(Animation* animation);
    void clearProps();

  protected:
    void setSliderValue(QSlider* slider,QLineEdit* edit,float value);

    QString selectFileToOpen(const QString& caption);
    void addToOpenFiles(const QString& fileName);
    void removeFromOpenFiles(unsigned int which);
    bool clearOpenFiles();

    bool checkFileOverwrite(const QFileInfo& fileInfo);
    void setCurrentFile(const QString& fileName);
    void enableInputs(bool state);

    void updateFps();
    void updateKeyBtn();
    void updateInputs();
    void updatePropSpins(const Prop* prop);

    void setX(float x);
    void setY(float y);
    void setZ(float z);

    float getX();
    float getY();
    float getZ();

    void setXPos(float x);
    void setYPos(float y);
    void setZPos(float z);

    float getXPos();
    float getYPos();
    float getZPos();

    QString currentFile;
    QStringList openFiles;
    // last path used for open or save
    QString lastPath;
    QTimer timer;
    // list of animation ids mapped to combo box indexes
    QPtrList<Animation> animationIds;

    QString currentPart;

    Timeline* timeline;

    bool playing;
    bool loop;
    bool jointLimits;
    bool frameDataValid;
    // if set the first frame of an animation is protected
    bool protectFirstFrame;
    // will be true if a frame is protected
    bool protect;
};

#endif
