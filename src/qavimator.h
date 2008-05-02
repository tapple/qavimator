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

#define UNTITLED_NAME "Untitled.avm"
#define PLAY_IMAGE "data/play.png"
#define PAUSE_IMAGE "data/pause.png"
#define KEY_IMAGE "data/key.png"
#define NOKEY_IMAGE "data/nokey.png"

#include "ui_mainapplicationform.h"
#include "rotation.h"

class Animation;
class Prop;
class Timeline;
class QCloseEvent;

class qavimator : public QMainWindow, Ui::MainWindow
{
  Q_OBJECT

  public:
    qavimator();
    ~qavimator();

    typedef enum { PLAYSTATE_STOPPED=0, PLAYSTATE_LOOPING, PLAYSTATE_PLAYING, NUM_PLAYSTATES } PlayState;

  signals:
    void enableRotation(bool state);
    void enablePosition(bool state);
    void enableProps(bool state);
    void enableEaseInOut(bool state);
    void resetCamera();
    void protectFrame(bool state);

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

    void frameTimeout();

    void setCurrentFrame(int frame);

    void selectAnimation(Animation* animation);
    void clearProps();

    // autoconnection from designer UI

    // ------- Menu Action Slots --------
    void on_fileNewAction_triggered();
    void on_fileOpenAction_triggered();
    void on_fileAddAction_triggered();
    void on_fileSaveAction_triggered();
    void on_fileSaveAsAction_triggered();
    void on_fileExportForSecondLifeAction_triggered();
    void on_fileLoadPropsAction_triggered();
    void on_fileSavePropsAction_triggered();
    void on_fileExitAction_triggered();

    void on_editCutAction_triggered();
    void on_editCopyAction_triggered();
    void on_editPasteAction_triggered();
    void on_editOptimizeBVHAction_triggered();

    void on_optionsSkeletonAction_toggled(bool on);
    void on_optionsJointLimitsAction_toggled(bool on);
    void on_optionsLoopAction_toggled(bool on);
    void on_optionsProtectFirstFrameAction_toggled(bool on);
    void on_optionsShowTimelineAction_toggled(bool on);
    void on_optionsConfigureQAvimatorAction_triggered();

    void on_helpAboutAction_triggered();

    // ------- Additional Toolbar Element Slots -------

    void on_resetCameraAction_triggered();

    // ------- UI Element Slots --------
    void on_selectAnimationCombo_activated(int);
    void on_figureCombo_activated(int);
    void on_scaleSpin_valueChanged(int newValue);
    void on_editPartCombo_activated(int);
    void on_xRotationEdit_returnPressed();
    void on_xRotationEdit_lostFocus();
    void on_xRotationSlider_valueChanged(int);
    void on_yRotationEdit_returnPressed();
    void on_yRotationEdit_lostFocus();
    void on_yRotationSlider_valueChanged(int);
    void on_zRotationEdit_returnPressed();
    void on_zRotationEdit_lostFocus();
    void on_zRotationSlider_valueChanged(int);
    void on_xPositionEdit_returnPressed();
    void on_xPositionEdit_lostFocus();
    void on_xPositionSlider_valueChanged(int);
    void on_yPositionEdit_returnPressed();
    void on_yPositionEdit_lostFocus();
    void on_yPositionSlider_valueChanged(int);
    void on_zPositionEdit_returnPressed();
    void on_zPositionEdit_lostFocus();
    void on_zPositionSlider_valueChanged(int);
    void on_easeInCheck_stateChanged(int newState);
    void on_easeOutCheck_stateChanged(int newState);

    void on_newBoxPropButton_clicked();
    void on_newSpherePropButton_clicked();
    void on_newConePropButton_clicked();
    void on_newTorusPropButton_clicked();
    void on_propNameCombo_activated(const QString& name);
    void on_deletePropButton_clicked();
    void on_attachToComboBox_activated(int attachmentPoint);
    void on_propXPosSpin_valueChanged(int);
    void on_propYPosSpin_valueChanged(int);
    void on_propZPosSpin_valueChanged(int);
    void on_propXScaleSpin_valueChanged(int);
    void on_propYScaleSpin_valueChanged(int);
    void on_propZScaleSpin_valueChanged(int);
    void on_propXRotSpin_valueChanged(int);
    void on_propYRotSpin_valueChanged(int);
    void on_propZRotSpin_valueChanged(int);

    void on_currentFrameSlider_valueChanged(int newValue);
    void on_playButton_clicked();
    void on_keyframeButton_toggled(bool on);
    void on_loopInSpinBox_valueChanged(int newValue);
    void on_loopOutSpinBox_valueChanged(int newValue);
    void on_framesSpin_valueChanged(int num);
    void on_fpsSpin_valueChanged(int num);
    // end autoconnection from designer UI

  protected:
    // prevent closing of main window if there are unsaved changes
    virtual void closeEvent(QCloseEvent* event);

    void fileNew();
    void fileOpen();
    void fileOpen(const QString& fileName);
    // "add" a new file without clearing the old one(s)
    void fileAdd();
    void fileAdd(const QString& fileName);
    void fileSave();
    void fileSaveAs();
    void fileExportForSecondLife();
    void fileLoadProps();
    void fileSaveProps();
    void fileExit();

    void editCut();
    void editCopy();
    void editPaste();
    void editOptimizeBVH();

    void showSkeleton(bool on);
    void setJointLimits(bool on);
    void setLoop(bool on);
    void setProtectFirstFrame(bool on);
    void showTimeline(bool state);
    void configure();

    void helpAbout();

    void animationChanged(int which);
    void setAvatarShape(int shape);
    void setAvatarScale(int percent);
    void partChoice();
    void rotationValue();
    void rotationSlider();
    void positionValue();
    void positionSlider();

    void easeInChanged(int change);
    void easeOutChanged(int change);

    void newProp(Prop::PropType);
    void selectProp(const QString& name);
    void deleteProp();
    void attachProp(int attachmentPoint);
    void propPositionChanged();
    void propScaleChanged();
    void propRotationChanged();

    void frameSlider(int position);
    void nextPlaystate();
    void setLoopInPoint(int inFrame);
    void setLoopOutPoint(int outFrame);
    void numFramesChanged(int num);
    void setFPS(int fps);

    void setSliderValue(QSlider* slider,QLineEdit* edit,float value);

    QString selectFileToOpen(const QString& caption);
    void addToOpenFiles(const QString& fileName);
    void removeFromOpenFiles(unsigned int which);
    bool clearOpenFiles();

    void setPlaystate(PlayState state);

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
    QList<Animation*> animationIds;

    QString currentPart;

    Timeline* timeline;
    // icons for play button
    QIcon playIcon;
    QIcon loopIcon;
    QIcon stopIcon;

    // holds the current playing status
    PlayState playstate;

    bool loop;
    bool jointLimits;
    bool frameDataValid;
    // if set the first frame of an animation is protected
    bool protectFirstFrame;
    // will be true if a frame is protected
    bool protect;
};

#endif
