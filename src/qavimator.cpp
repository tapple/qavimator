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

#include <qapplication.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qaction.h>
#include <qsettings.h>
#include <qgroupbox.h>
#include <qregexp.h>

#include "qavimator.h"
#include "animationview.h"
#include "rotation.h"
#include "prop.h"
#include "timeline.h"
#include "timelineview.h"

#define ANIM_FILTER "Animation Files (*.bvh *.avm)"
#define PROP_FILTER "Props (*.prp)"
#define PRECISION 100

qavimator::qavimator() : MainApplicationForm( 0, "qavimator", WDestructiveClose )
{
  playing=false;
  frameDataValid = false;

  readSettings();

  connect(animationView,SIGNAL(partClicked(const QString&,
                                           Rotation,
                                           RotationLimits,
                                           Position)),
                     this,SLOT(partClicked(const QString&,
                                           Rotation,
                                           RotationLimits,
                                           Position)));

  connect(animationView,SIGNAL(partDragged(const QString&,
                                           double,double,double)),
                     this,SLOT(partDragged(const QString&,
                                           double,double,double)));

  connect(animationView,SIGNAL(propClicked(Prop*)),this,SLOT(propClicked(Prop*)));

  connect(animationView,SIGNAL(propDragged(Prop*,double,double,double)),
                     this,SLOT(propDragged(Prop*,double,double,double)));

  connect(animationView,SIGNAL(propScaled(Prop*,double,double,double)),
                     this,SLOT(propScaled(Prop*,double,double,double)));

  connect(animationView,SIGNAL(propRotated(Prop*,double,double,double)),
                     this,SLOT(propRotated(Prop*,double,double,double)));

  connect(animationView,SIGNAL(backgroundClicked()),this,SLOT(backgroundClicked()));

  connect(xSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_RotRoller(int)));
  connect(ySlider,SIGNAL(valueChanged(int)),this,SLOT(cb_RotRoller(int)));
  connect(zSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_RotRoller(int)));

  connect(xRotationEdit,SIGNAL(returnPressed()),this,SLOT(cb_RotValue()));
  connect(yRotationEdit,SIGNAL(returnPressed()),this,SLOT(cb_RotValue()));
  connect(zRotationEdit,SIGNAL(returnPressed()),this,SLOT(cb_RotValue()));

  connect(xRotationEdit,SIGNAL(lostFocus()),this,SLOT(cb_RotValue()));
  connect(yRotationEdit,SIGNAL(lostFocus()),this,SLOT(cb_RotValue()));
  connect(zRotationEdit,SIGNAL(lostFocus()),this,SLOT(cb_RotValue()));

  connect(xPositionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_PosRoller(int)));
  connect(yPositionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_PosRoller(int)));
  connect(zPositionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_PosRoller(int)));

  connect(xPositionEdit,SIGNAL(returnPressed()),this,SLOT(cb_PosValue()));
  connect(yPositionEdit,SIGNAL(returnPressed()),this,SLOT(cb_PosValue()));
  connect(zPositionEdit,SIGNAL(returnPressed()),this,SLOT(cb_PosValue()));

  connect(xPositionEdit,SIGNAL(lostFocus()),this,SLOT(cb_PosValue()));
  connect(yPositionEdit,SIGNAL(lostFocus()),this,SLOT(cb_PosValue()));
  connect(zPositionEdit,SIGNAL(lostFocus()),this,SLOT(cb_PosValue()));

  connect(this,SIGNAL(enablePosition(bool)),positionGroupBox,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),rotationGroupBox,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableRotation(bool)),fpsLabel,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),fpsSpin,SLOT(setEnabled(bool)));

  connect(fpsSpin,SIGNAL(valueChanged(int)),this,SLOT(cb_fpsValue(int)));

  connect(positionSlider,SIGNAL(valueChanged(int)),this,SLOT(cb_FrameSlider(int)));
  connect(playButton,SIGNAL(clicked()),this,SLOT(cb_PlayBtn()));

  connect(this,SIGNAL(enableProps(bool)),propPositionGroup,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableProps(bool)),propScaleGroup,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableProps(bool)),propRotationGroup,SLOT(setEnabled(bool)));

  connect(&timer,SIGNAL(timeout()),this,SLOT(cb_timeout()));

  connect(this,SIGNAL(resetCamera()),animationView,SLOT(resetCamera()));
  connect(this,SIGNAL(protectFrame(bool)),animationView,SLOT(protectFrame(bool)));

  timeline=new Timeline(timelineView->viewport(),"timeline");
  timelineView->addChild(timeline);

  connect(timeline,SIGNAL(positionCenter(int)),timelineView,SLOT(scrollTo(int)));

  xSlider->setPageStep(10*PRECISION);
  ySlider->setPageStep(10*PRECISION);
  zSlider->setPageStep(10*PRECISION);
  xPositionSlider->setPageStep(10*PRECISION);
  yPositionSlider->setPageStep(10*PRECISION);
  zPositionSlider->setPageStep(10*PRECISION);

  positionSlider->setPageStep(1);

  if(qApp->argc()>1)
  {
    qDebug(qApp->argv()[1]);
    fileOpen(qApp->argv()[1]);
  }
  else fileNew();
}

qavimator::~qavimator()
{
  fileExit();
  if(timeline) delete timeline;
}

void qavimator::readSettings()
{
  QSettings settings;
  settings.setPath("DeZiRee","QAvimator",QSettings::User);
  settings.beginGroup("/qavimator");

  // if no settings found, start up with defaults
  int width=850;
  int height=600;
  int figureType=0;
  bool skeleton=false;
  bool jointLimits=true;
  bool showTimeline=false;
  loop=true;
  protectFirstFrame=true;
  lastPath=QString::null;

  bool settingsFound=settings.readBoolEntry("/settings");
  if(settingsFound)
  {
    loop=settings.readBoolEntry("/loop");
    skeleton=settings.readBoolEntry("/skeleton");
    jointLimits=settings.readBoolEntry("/joint_limits");
    protectFirstFrame=settings.readBoolEntry("/protect_first_frame");
    showTimeline=settings.readBoolEntry("/show_timeline");

    int width=settings.readNumEntry("/mainwindow_width");
    int height=settings.readNumEntry("/mainwindow_height");

    lastPath=settings.readEntry("/last_path");

    // sanity
    if(width<50) width=50;
    if(height<50) height=50;

    figureType=settings.readNumEntry("/figure");

    settings.endGroup();
  }

  resize(width,height);

  optionsLoopAction->setOn(loop);
  optionsSkeletonAction->setOn(skeleton);
  optionsJointLimitsAction->setOn(jointLimits);
  optionsShowTimelineAction->setOn(showTimeline);
  if(!showTimeline) timelineView->hide();
  // prevent a signal to be sent to yet uninitialized animation view
  optionsProtectFirstFrameAction->blockSignals(true);
  optionsProtectFirstFrameAction->setOn(protectFirstFrame);
  optionsProtectFirstFrameAction->blockSignals(false);
  figureCombo->setCurrentItem(figureType);
  figureChanged(figureType);
}

// slot gets called by AnimationView::mousePressEvent()
void qavimator::partClicked(const QString& partName,Rotation rot,RotationLimits limits,Position pos)
{
  if(partName)
  {
    for(int index=0;index<editPartCombo->count();index++)
      if(editPartCombo->text(index)==partName) editPartCombo->setCurrentItem(index);

    setX(rot.x);
    setY(rot.y);
    setZ(rot.z);

    if(partName=="hip")
    {
      enablePosition(!protect);

      setXPos(pos.x);
      setYPos(pos.y);
      setZPos(pos.z);

      xSlider->setRange(-359*PRECISION, 359*PRECISION);
      ySlider->setRange(-359*PRECISION, 359*PRECISION);
      zSlider->setRange(-359*PRECISION, 359*PRECISION);
    }
    else
    {
      enablePosition(false);

      xSlider->setRange((int)(limits.xMin*PRECISION),
			(int)(limits.xMax*PRECISION));
      ySlider->setRange((int)(limits.yMin*PRECISION),
			(int)(limits.yMax*PRECISION));
      zSlider->setRange((int)(limits.zMin*PRECISION),
			(int)(limits.zMax*PRECISION));
    }

    // show the user if this part has a key frame here
    updateKeyBtn();
  }
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::partDragged(const QString& partName,double x,double y,double z)
{
  if(partName)
  {
    // check if this frame is protected
    if(!protect)
    {
      // get animation object
      Animation *anim = animationView->getAnimation();
      // get rotation values for selected part
      Rotation rot=anim->getRotation(partName);
      // get rotation limits for part
      RotationLimits rotLimits=anim->getRotationLimits(partName);

      // calculate new rotation (x, y, z are the modifiers)
      double newX=rot.x+x;
      double newY=rot.y+y;
      double newZ=rot.z+z;

      double xMin=rotLimits.xMin;
      double yMin=rotLimits.yMin;
      double zMin=rotLimits.zMin;
      double xMax=rotLimits.xMax;
      double yMax=rotLimits.yMax;
      double zMax=rotLimits.zMax;

      if (newX < xMin) newX = xMin;
      if (newX > xMax) newX = xMax;
      if (newY < yMin) newY = yMin;
      if (newY > yMax) newY = yMax;
      if (newZ < zMin) newZ = zMin;
      if (newZ > zMax) newZ = zMax;

      setX(newX);
      setY(newY);
      setZ(newZ);

      animationView->getAnimation()->setRotation(partName,newX,newY,newZ);
      animationView->repaint();

      updateKeyBtn();
    }
  }
  else qDebug("partDragged(): partName==\"\"!");
}

// slot gets called by AnimationView::propClicked()
void qavimator::propClicked(Prop* prop)
{
  // update prop name combo box
  for(int index=0;index<propNameCombo->count();index++)
    if(propNameCombo->text(index)==prop->name()) propNameCombo->setCurrentItem(index);

  // update prop value spinboxes
  selectProp(prop->name());
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::propDragged(Prop* prop,double x,double y,double z)
{
  prop->x+=x;
  prop->y+=y;
  prop->z+=z;
  updatePropSpins(prop);
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::propScaled(Prop* prop,double x,double y,double z)
{
  prop->xs+=x;
  prop->ys+=y;
  prop->zs+=z;
  updatePropSpins(prop);
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::propRotated(Prop* prop,double x,double y,double z)
{
  prop->xr+=x;
  prop->yr+=y;
  prop->zr+=z;
  updatePropSpins(prop);
}

// slot gets called by AnimationView::mouseButtonClicked()
void qavimator::backgroundClicked()
{
  updateKeyBtn();
}

// slot gets called by body part dropdown list
void qavimator::cb_PartChoice()
{
  // selectPart will fire partClicked event, so we don't bother
  // about updating controls ourselves here
  animationView->selectPart(editPartCombo->currentText());
  animationView->setFocus();
}

void qavimator::cb_RotRoller(int)
{
  double x = getX();
  double y = getY();
  double z = getZ();

  setX(x);
  setY(y);
  setZ(z);

  Animation *anim = animationView->getAnimation();
  if (animationView->getSelectedPart())
  {
      anim->setRotation(animationView->getSelectedPart(), x, y, z);
      animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::cb_RotValue()
{
  double x = xRotationEdit->text().toDouble();
  double y = yRotationEdit->text().toDouble();
  double z = zRotationEdit->text().toDouble();

  double min_x = xSlider->minValue()/PRECISION;
  double min_y = ySlider->minValue()/PRECISION;
  double min_z = zSlider->minValue()/PRECISION;

  double max_x = xSlider->maxValue()/PRECISION;
  double max_y = ySlider->maxValue()/PRECISION;
  double max_z = zSlider->maxValue()/PRECISION;

  if (x<min_x) x = min_x;  if (y<min_y) y = min_y;  if (z<min_z) z = min_z;
  if (x>max_x) x = max_x;  if (y>max_y) y = max_y;  if (z>max_z) z = max_z;

  setX(x);
  setY(y);
  setZ(z);

  Animation *anim = animationView->getAnimation();
  if (animationView->getSelectedPart())
  {
      anim->setRotation(animationView->getSelectedPart(), x, y, z);
      animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::cb_PosRoller(int)
{
  double x = getXPos();
  double y = getYPos();
  double z = getZPos();

  setXPos(x);
  setYPos(y);
  setZPos(z);

  if (editPartCombo->currentText()) {
    animationView->getAnimation()->setPosition(editPartCombo->currentText(), x, y, z);
    animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::cb_PosValue()
{
  double x = xPositionEdit->text().toDouble();
  double y = yPositionEdit->text().toDouble();
  double z = zPositionEdit->text().toDouble();

  double min_x = xPositionSlider->minValue()/PRECISION;
  double min_y = yPositionSlider->minValue()/PRECISION;
  double min_z = zPositionSlider->minValue()/PRECISION;

  double max_x = xPositionSlider->maxValue()/PRECISION;
  double max_y = yPositionSlider->maxValue()/PRECISION;
  double max_z = zPositionSlider->maxValue()/PRECISION;

  if (x<min_x) x = min_x;  if (y<min_y) y = min_y;  if (z<min_z) z = min_z;
  if (x>max_x) x = max_x;  if (y>max_y) y = max_y;  if (z>max_z) z = max_z;

  setXPos(x);
  setYPos(y);
  setZPos(z);

  if (editPartCombo->currentText()) {
    animationView->getAnimation()->setPosition(editPartCombo->currentText(), x, y, z);
    animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::updateInputs()
{
  double x=0, y=0, z=0;
  Animation *anim = animationView->getAnimation();

  if (anim) {
    double xMin, xMax, yMin, yMax, zMin, zMax;

    Rotation rot=anim->getRotation(editPartCombo->currentText());
    x=rot.x;
    y=rot.y;
    z=rot.z;

    RotationLimits rotLimits=anim->getRotationLimits(editPartCombo->currentText());
    xMin=rotLimits.xMin;
    yMin=rotLimits.yMin;
    zMin=rotLimits.zMin;
    xMax=rotLimits.xMax;
    yMax=rotLimits.yMax;
    zMax=rotLimits.zMax;

    if (editPartCombo->currentText()=="hip") {
      xSlider->setRange(-359*PRECISION, 359*PRECISION);
      ySlider->setRange(-359*PRECISION, 359*PRECISION);
      zSlider->setRange(-359*PRECISION, 359*PRECISION);
    }
    else
    {
      xSlider->setRange((int)(xMin*PRECISION), (int)(xMax*PRECISION));
      ySlider->setRange((int)(yMin*PRECISION), (int)(yMax*PRECISION));
      zSlider->setRange((int)(zMin*PRECISION), (int)(zMax*PRECISION));
    }

    setX(x);
    setY(y);
    setZ(z);

    positionSlider->setMaxValue(anim->getNumberOfFrames()-1);

    framesSpin->setValue(anim->getNumberOfFrames());
  }
  if (editPartCombo->currentText()=="hip") {
    emit enablePosition(!protect);
    Position pos=anim->getPosition("hip");

    setXPos(pos.x);
    setYPos(pos.y);
    setZPos(pos.z);
  }
  else {
    emit enablePosition(false);
  }
  playButton->setText(playing ? "||" : ">");

  updateKeyBtn();

  if (playing)
    enableInputs(false);
  else
    enableInputs(true);

  if (frameDataValid)
    editPasteAction->setEnabled(true);
  else
    editPasteAction->setEnabled(false);

  if(propNameCombo->count())
    enableProps(true);
  else
    enableProps(false);
}

void qavimator::updateKeyBtn()
{
  // make sure we don't send a toggle signal on display update
  keyframeButton->blockSignals(true);
  keyframeButton->setOn(animationView->getAnimation()->isKeyFrame(animationView->getSelectedPart()));
  // re-enable toggle signal
  keyframeButton->blockSignals(false);

//  timeline->repaint();
}

void qavimator::enableInputs(bool state)
{
  if(protect) state=false;

  emit enableRotation(state);
  if (editPartCombo->currentText()=="hip") emit enablePosition(state);

  keyframeButton->setEnabled(state);
}

void qavimator::cb_timeout()
{
  if (playing) {
    Animation *anim = animationView->getAnimation();
    if (anim) {

      // don't show protected frames color on playback to avoid flicker
      protectFrame(false);
      // cycle through frames, do not show first frame, if protected
      animationView->stepForward(protectFirstFrame);

      if (anim->getFrame() == (anim->getNumberOfFrames() - 1) && !loop)
      {
        timer.stop();
        playing=false;
      }

      updateInputs();
      return;
    }
  }
}

void qavimator::cb_PlayBtn()
{
  playing = !playing;
  if (playing)
    timer.start((int)(animationView->getAnimation()->frameTime()*1000));
  else
  {
    // take care of locks, key frames ...
    cb_FrameSlider(positionSlider->value());
  }

  updateInputs();
}

void qavimator::cb_fpsValue(int fps) {
  animationView->setFrameTime(1.0/(double) fps);
}

void qavimator::cb_FrameSlider(int position)
{
  // check if we are at the first frame and if it's protected
  if(position==0 && protectFirstFrame) protect=true;
  else protect=false;

  playing=false;
  animationView->setFrame(position);
  emit protectFrame(protect);

  updateInputs();
}

void qavimator::animationChanged(int which)
{
    if (which >= openFiles.count()) return;
    setCurrentFile(*openFiles.at(which));
    animationView->selectAnimation(which);
}

void qavimator::figureChanged(int shape)
{
  if(shape==0)
    animationView->setFigure(AnimationView::FEMALE);
  else
    animationView->setFigure(AnimationView::MALE);
  animationView->repaint();
}

void qavimator::numFramesChanged(int num)
{
  animationView->getAnimation()->setNumberOfFrames(num);
  updateInputs();
}

void qavimator::keyframeButtonToggled(bool)
{
  animationView->getAnimation()->toggleKeyFrame(animationView->getSelectedPart());
  animationView->repaint();
}

// ------ Menu Action Slots (Callbacks) -----------

// Menu action: File / New
void qavimator::fileNew()
{
  setCurrentFile(UNTITLED_NAME);

  Animation* anim=new Animation();
  animationView->setAnimation(anim);
  timeline->setAnimation(anim);
  addToOpenFiles(UNTITLED_NAME);

  // FIXME: code duplication
  connect(animationView->getAnimation(),SIGNAL(currentFrame(int)),this,SLOT(setCurrentFrame(int)));

  editPartCombo->setCurrentItem(1);

  playing=false;

  if(protectFirstFrame)
    // skip first frame, since it's protected anyway
      animationView->setFrame(1);
  else
    animationView->setFrame(0);

  // show frame as unprotected
  emit protectFrame(false);
  protect=false;

  updateInputs();
  updateFps();
}

// Menu action: File / Open ...
void qavimator::fileOpen()
{
  clearOpenFiles();
  fileAdd();
}

void qavimator::fileOpen(const QString& name)
{
    clearOpenFiles();
    fileAdd(name);
}

// Menu action: File / Add New Animation ...
void qavimator::fileAdd()
{
  fileAdd(QString::null);
}

void qavimator::fileAdd(const QString& name)
{
  QString file=name;
  if(!file)
  {
    file=QFileDialog::getOpenFileName(lastPath,
                                      ANIM_FILTER,
                                      this,
                                      "file_open_dialog",
                                      tr("Select Animation File"),
                                      0,
                                      false);
  }
  if (file) {
    QFileInfo fileInfo(file);
    if(fileInfo.exists())
    {
      addToOpenFiles(file);
      Animation* anim=new Animation(file);

      setCurrentFile(file);
      lastPath=fileInfo.dirPath(false);
      animationView->addAnimation(anim);
      timeline->setAnimation(anim);

      // set the frame
      if (animationView->getAnimation(1))
	  animationView->setFrame(animationView->getAnimation(0)->getFrame());
      else if (protectFirstFrame)
	  animationView->setFrame(1);
      else
	  animationView->setFrame(0);

      // FIXME: code duplication
      connect(animationView->getAnimation(),SIGNAL(currentFrame(int)),this,SLOT(setCurrentFrame(int)));
      editPartCombo->setCurrentItem(1);
      updateInputs();
      updateFps();
    }
  }
  // if no files are open create a new animation
  if(!openFiles.count()) fileNew();
}

// Menu Action: File / Save
void qavimator::fileSave()
{
  if(currentFile==UNTITLED_NAME)
    fileSaveAs();
  else
    animationView->getAnimation()->saveBVH(currentFile);
}

// Menu Action: File / Save As...
void qavimator::fileSaveAs()
{
  QString file=QFileDialog::getSaveFileName(currentFile,
                                            ANIM_FILTER,
                                            this,
                                            "file_save_as_dialog",
                                            tr("Save Animation File"),
                                            0,
                                            false);
  if (file) {
    setCurrentFile(file);
    QFileInfo fileInfo(file);
    lastPath=fileInfo.dirPath(false);
    animationView->getAnimation()->saveBVH(file);
  }
}

// Menu Action: File / Load Props...
void qavimator::fileLoadProps()
{
  QString fileName=QFileDialog::getOpenFileName(lastPath,
                                                PROP_FILTER,
                                                this,
                                                "load_props_dialog",
                                                tr("Select Props File"),
                                                0,
                                                false);
  if (fileName) {
    QFileInfo fileInfo(fileName);
    if(fileInfo.exists())
    {
      clearProps();
      QFile file(fileName);
      if(file.open(IO_ReadOnly))
      {
        QString line;
        while(!file.atEnd())
        {
          file.readLine(line,2048);
          QStringList properties=QStringList::split(' ',line);
          const Prop* prop=animationView->addProp((Prop::PropType) properties[0].toInt(),
                                            properties[1].toDouble(),
                                            properties[2].toDouble(),
                                            properties[3].toDouble(),
                                            properties[4].toDouble(),
                                            properties[5].toDouble(),
                                            properties[6].toDouble(),
                                            properties[7].toDouble(),
                                            properties[8].toDouble(),
                                            properties[9].toDouble()
                                           );
          if(prop)
          {
            propNameCombo->insertItem(prop->name());
            propNameCombo->setCurrentItem(propNameCombo->count()-1);
            selectProp(prop->name());
          }
        } // while
      }
    }
  }
}

// Menu Action: File / Save Props...
void qavimator::fileSaveProps()
{
  QString fileName=QFileDialog::getSaveFileName(currentFile,
                                                PROP_FILTER,
                                                this,
                                                "save_props_dialog",
                                                tr("Save Props"),
                                                0,
                                                false);
  if (fileName) {
    QFile file(fileName);
    if(file.open(IO_WriteOnly))
    {
      for(int index=0;index<propNameCombo->count();index++)
      {
        Prop* prop=animationView->getPropByName(propNameCombo->text(index));
        QStringList properties;
        properties.append(QString::number(prop->type));
        properties.append(QString::number(prop->x));
        properties.append(QString::number(prop->y));
        properties.append(QString::number(prop->z));
        properties.append(QString::number(prop->xs));
        properties.append(QString::number(prop->ys));
        properties.append(QString::number(prop->zs));
        properties.append(QString::number(prop->xr));
        properties.append(QString::number(prop->yr));
        properties.append(QString::number(prop->zr));
        QString line=properties.join(" ")+"\n";
        file.writeBlock(line,line.length());
      } // for
    }
  }
}


// Menu Action: File / Exit
void qavimator::fileExit()
{
  QSettings settings;
  settings.setPath("DeZiRee","QAvimator",QSettings::User);

  settings.beginGroup("/qavimator");

  // make sure we know next time, that there actually was a settings file
  settings.writeEntry("/settings",true);

  settings.writeEntry("/loop",loop);
  settings.writeEntry("/skeleton",optionsSkeletonAction->isOn());
  settings.writeEntry("/joint_limits",optionsJointLimitsAction->isOn());
  settings.writeEntry("/protect_first_frame",optionsProtectFirstFrameAction->isOn());
  settings.writeEntry("/show_timeline",optionsShowTimelineAction->isOn());

  settings.writeEntry("/figure",figureCombo->currentItem());
  settings.writeEntry("/mainwindow_width",size().width());
  settings.writeEntry("/mainwindow_height",size().height());

  settings.writeEntry("/last_path",lastPath);

  settings.endGroup();
  qApp->quit();
}

// Menu Action: Edit / Copy
void qavimator::editCopy()
{
  animationView->getAnimation()->getFrameData(frameData);
  frameDataValid = true;
  updateInputs();
}

// Menu Action: Edit / Paste
void qavimator::editPaste()
{
  if (frameDataValid)
  {
    animationView->getAnimation()->setFrameData(frameData);
    animationView->repaint();
    updateInputs();
  }
}

// Menu Action: Options / Skeleton
void qavimator::optionsSkeleton(bool on)
{
  if(on)
    animationView->showSkeleton();
  else
    animationView->hideSkeleton();
}

// Menu Action: Options / Loop
void qavimator::optionsLoop(bool on)
{
  loop=on;
}

// Menu Action: Options / Joint Limits
void qavimator::optionsJointLimits(bool on)
{
  animationView->getAnimation()->useRotationLimits(on);
  animationView->repaint();
  updateInputs();
}

// Menu Action: Oprions / Protect First Frame
void qavimator::optionsProtectFirstFrame(bool on)
{
  protectFirstFrame=on;
  if(on && positionSlider->value()==0) protect=true;
  else protect=false;

  emit protectFrame(protect);
  updateInputs();
}

// Menu Action: Oprions / Show Timeline
void qavimator::optionsShowTimeline(bool on)
{
  if(on)
    timelineView->show();
  else
    timelineView->hide();

  // hack to get 3D view back in shape
  qApp->processEvents();
  QSize oldSize=animationView->size();
  animationView->resize(oldSize.width(),oldSize.height()-1);
  qApp->processEvents();
  animationView->resize(oldSize);
}

void qavimator::setX(float x)
{
  setSliderValue(xSlider,xRotationEdit,x);
}

void qavimator::setY(float y)
{
  setSliderValue(ySlider,yRotationEdit,y);
}

void qavimator::setZ(float z)
{
  setSliderValue(zSlider,zRotationEdit,z);
}

float qavimator::getX()
{
  return xSlider->value()/PRECISION;
}

float qavimator::getY()
{
  return ySlider->value()/PRECISION;
}

float qavimator::getZ()
{
  return zSlider->value()/PRECISION;
}

void qavimator::setXPos(float x)
{
  setSliderValue(xPositionSlider,xPositionEdit,x);
}

void qavimator::setYPos(float y)
{
  setSliderValue(yPositionSlider,yPositionEdit,y);
}

void qavimator::setZPos(float z)
{
  setSliderValue(zPositionSlider,zPositionEdit,z);
}

float qavimator::getXPos()
{
  return xPositionSlider->value()/PRECISION;
}

float qavimator::getYPos()
{
  return yPositionSlider->value()/PRECISION;
}

float qavimator::getZPos()
{
  return zPositionSlider->value()/PRECISION;
}

// helper function to prevent feedback between the two widgets
void qavimator::setSliderValue(QSlider* slider,QLineEdit* edit,float value)
{
  slider->blockSignals(true);
  edit->blockSignals(true);
  slider->setValue((int)(value*PRECISION));
  edit->setText(QString::number(value));
  edit->blockSignals(false);
  slider->blockSignals(false);
}

void qavimator::updateFps()
{
  double frameTime=animationView->getAnimation()->frameTime();

  // guard against division by zero
  if(frameTime!=0.0)
  {
    // don't send FPS change back to Animation object
    framesSpin->blockSignals(true);
    fpsSpin->setValue((int)(1/frameTime));
    // re-enable FPS signal
    framesSpin->blockSignals(false);
  }
}

// Adds a file to the open files list, and adds the entry in the combo box
void qavimator::addToOpenFiles(const QString& fileName)
{
    openFiles.append(fileName);

    QString fixedName = fileName;
    QRegExp pattern(".*/");
    fixedName.remove(pattern);
    pattern.setPattern("(\\.bvh|\\.avm)");
    fixedName.remove(pattern);

    selectAnimationCombo->insertItem(fixedName);
}

void qavimator::removeFromOpenFiles(int which)
{
    if (which >= openFiles.count()) return;
    openFiles.remove(openFiles.at(which));
    selectAnimationCombo->removeItem(which);
}


// empty out the open files list
void qavimator::clearOpenFiles()
{
    timeline->setAnimation(0);
    animationView->clear();
    openFiles.clear();
    selectAnimationCombo->clear();
}

// convenience function to set window title in a defined way
void qavimator::setCurrentFile(const QString& fileName)
{
  currentFile=fileName;
  setCaption("qavimator ["+currentFile+"]");
}

// this slot gets called from Animation::setFrame(int)
void qavimator::setCurrentFrame(int frame)
{
  positionSlider->blockSignals(true);
  positionSlider->setValue(frame);
  currentFrameLabel->setText(QString::number(frame+1));
  positionSlider->blockSignals(false);
  timeline->setCurrentFrame(frame);
  animationView->setFrame(frame);
  updateInputs();
  updateKeyBtn();
  // check if we are at the first frame and if it's protected
  if(frame==0 && protectFirstFrame) protect=true;
  else protect=false;
  emit protectFrame(protect);
}

// this slot gets called when someone clicks the "New Prop" button
void qavimator::newPropButtonClicked()
{
  const Prop* prop=animationView->addProp(Prop::Box,10,40,10, 10,10,10, 0,0,0);
  if(prop)
  {
    propNameCombo->insertItem(prop->name());
    propNameCombo->setCurrentItem(propNameCombo->count()-1);
    selectProp(prop->name());
  }
}

void qavimator::selectProp(const QString& propName)
{
  const Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    emit enableProps(true);
    propNameCombo->setEnabled(true);
    deletePropButton->setEnabled(true);

    updatePropSpins(prop);
    animationView->selectProp(prop->name());
  }
  else
  {
    emit enableProps(false);
    propNameCombo->setEnabled(false);
    deletePropButton->setEnabled(false);
  }
}

void qavimator::updatePropSpins(const Prop* prop)
{
  propXPosSpin->blockSignals(true);
  propYPosSpin->blockSignals(true);
  propZPosSpin->blockSignals(true);

  propXPosSpin->setValue((int)(prop->x));
  propYPosSpin->setValue((int)(prop->y));
  propZPosSpin->setValue((int)(prop->z));

  propXPosSpin->blockSignals(false);
  propYPosSpin->blockSignals(false);
  propZPosSpin->blockSignals(false);

  propXRotSpin->blockSignals(true);
  propYRotSpin->blockSignals(true);
  propZRotSpin->blockSignals(true);

  propXRotSpin->setValue((int)(prop->xr));
  propYRotSpin->setValue((int)(prop->yr));
  propZRotSpin->setValue((int)(prop->zr));

  propXRotSpin->blockSignals(false);
  propYRotSpin->blockSignals(false);
  propZRotSpin->blockSignals(false);

  propXScaleSpin->blockSignals(true);
  propYScaleSpin->blockSignals(true);
  propZScaleSpin->blockSignals(true);

  propXScaleSpin->setValue((int)(prop->xs));
  propYScaleSpin->setValue((int)(prop->ys));
  propZScaleSpin->setValue((int)(prop->zs));

  propXScaleSpin->blockSignals(false);
  propYScaleSpin->blockSignals(false);
  propZScaleSpin->blockSignals(false);
}

void qavimator::propPosChanged(int dummy)
{
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    prop->setPosition(propXPosSpin->value(),propYPosSpin->value(),propZPosSpin->value());
    animationView->repaint();
  }
}

void qavimator::propScaleChanged(int dummy)
{
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    prop->setScale(propXScaleSpin->value(),propYScaleSpin->value(),propZScaleSpin->value());
    animationView->repaint();
  }
}

void qavimator::propRotChanged(int dummy)
{
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    prop->setRotation(propXRotSpin->value(),propYRotSpin->value(),propZRotSpin->value());
    animationView->repaint();
  }
}

void qavimator::clearProps()
{
}
