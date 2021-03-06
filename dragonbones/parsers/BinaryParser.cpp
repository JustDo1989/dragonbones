//
//  BinaryParser.cpp
//  xmlToBinary
//
//  Created by Relvin on 16/3/29.
//  Copyright © 2016年 Relvin. All rights reserved.
//

#include "BinaryParser.h"

#include "ConstValues.h"
// geoms
#include "geoms/ColorTransform.h"
#include "geoms/Matrix.h"
#include "geoms/Point.h"
#include "geoms/Rectangle.h"
#include "geoms/Transform.h"
// objects
#include "objects/Frame.h"
#include "objects/TransformFrame.h"
#include "objects/Timeline.h"
#include "objects/TransformTimeline.h"
#include "objects/FFDTimeline.h"
#include "objects/AnimationData.h"
#include "objects/DisplayData.h"
#include "objects/SlotData.h"
#include "objects/SkinData.h"
#include "objects/BoneData.h"
#include "objects/MeshData.h"
#include "objects/ArmatureData.h"
#include "objects/DragonBonesData.h"
#include "objects/SlotFrame.h"
#include "objects/FFDFrame.h"
#include "dragonbones_generated.h"


NAME_SPACE_DRAGON_BONES_BEGIN

BinaryParser::BinaryParser()
: _frameRate(30)
, _isGlobalData(false)
, _armatureScale(1.0f)
{
    
}

BinaryParser::~BinaryParser()
{
    
}

DragonBonesData* BinaryParser::parseDragonBonesData(const void *rawDragonBonesData,float scale) const
{
    _armatureScale = scale;
    
    DragonBonesData *dragonBonesData = new DragonBonesData();
    
    auto dbParseBinaryBuffer = GetDragonBonesParseBinary(rawDragonBonesData);
    parseFlatStringToString(dbParseBinaryBuffer->version(), dragonBonesData->version);
    parseFlatStringToString(dbParseBinaryBuffer->name(), dragonBonesData->name);
    dragonBonesData->isGlobalData = dbParseBinaryBuffer->isGlobalData();
    dragonBonesData->autoSearch = dbParseBinaryBuffer->autoSearch();
    if (dbParseBinaryBuffer->version())
    {
//        CCLOG();
    }
    
    auto armatureOptionList = dbParseBinaryBuffer->armatures();
    if (armatureOptionList)
    {
        for (int index = 0;index < armatureOptionList->size();index++)
        {
            ArmatureData *armatureData = parseArmatureDataFromBinary(armatureOptionList->Get(index));
            dragonBonesData->armatureDataList.push_back(armatureData);
        }
    }
    
    
    return dragonBonesData;
}

ArmatureData* BinaryParser::parseArmatureDataFromBinary(const ArmatureOption * armatureOption) const
{
    
    ArmatureData *armatureData = new ArmatureData();
    
    parseFlatStringToString(armatureOption->name(), armatureData->name);
    
    auto boneDataList = armatureOption->boneDataList();
    if (boneDataList)
    {
        for (int idx = 0;idx < boneDataList->size();idx++)
        {
            BoneData *boneData = parseBoneDataFromBinary(boneDataList->Get(idx));
            armatureData->boneDataList.push_back(boneData);
        }
    }
    
    auto slotDataList = armatureOption->slotDataList();
    if (slotDataList)
    {
        for (int idx = 0;idx < slotDataList->size();idx++)
        {
            SlotData *slotData = parseSlotDataFromBinary(slotDataList->Get(idx));
            armatureData->slotDataList.push_back(slotData);
        }
    }
    
    auto skinDataList = armatureOption->skinDataList();
    if (skinDataList)
    {
        for (int idx = 0;idx < skinDataList->size();idx ++)
        {
            SkinData *skinData = parseSkinDataFromBinary(skinDataList->Get(idx));
            armatureData->skinDataList.push_back(skinData);
        }
    }
    
    auto ikDataList = armatureOption->ikDataList();
    if (ikDataList)
    {
        for (int idx = 0;idx < ikDataList->size();idx ++)
        {
            IKData *ikData = parseIKDataFromBinary(ikDataList->Get(idx));
            armatureData->ikDataList.push_back(ikData);
        }
    }
    
    armatureData->sortBoneDataList();
    
    auto animationDataList = armatureOption->animationDataList();
    if (animationDataList)
    {
        for (int idx = 0;idx < animationDataList->size();idx++)
        {
            auto animationDataOpt = animationDataList->Get(idx);//
            AnimationData *animationData = parseAnimationDataFromBinary(animationDataOpt);
            if (animationData)
            {
                armatureData->animationDataList.push_back(animationData);
            }
        }
    }
    
    return armatureData;
}

BoneData* BinaryParser::parseBoneDataFromBinary(const BoneDataOption* boneDataOption) const
{
    BoneData *boneData = new BoneData();
    
    parseFlatStringToString(boneDataOption->name(), boneData->name);
    parseFlatStringToString(boneDataOption->parent(), boneData->parent);
    
    boneData->length = boneDataOption->length();
    boneData->inheritRotation = boneDataOption->inheritRotation();
    boneData->inheritScale = boneDataOption->inheritScale();
    parseTransformFromBinary(boneDataOption->global(),boneData->global);
    parseTransformFromBinary(boneDataOption->transform(),boneData->transform);

    return boneData;
}

SlotData* BinaryParser::parseSlotDataFromBinary(const dragonBones::SlotDataOption *slotDataOption) const
{
    SlotData *slotData = new SlotData();
    
    parseFlatStringToString(slotDataOption->name(), slotData->name);
    parseFlatStringToString(slotDataOption->parent(), slotData->parent);
    slotData->zOrder = slotDataOption->zOrder();
    slotData->displayIndex = slotDataOption->displayIndex();
    slotData->blendMode = (BlendMode)slotDataOption->blendMode();
    auto displayDataList = slotDataOption->displayDataList();
    if (displayDataList)
    {
        for (int idx = 0;idx < displayDataList->size();idx++)
        {
            DisplayData *displayData = parseMainDisplayDataFromBinary(displayDataList->Get(idx));
            slotData->displayDataList.push_back(displayData);
        }
    }
    auto meshDataList = slotDataOption->meshDataList();
    if (meshDataList)
    {
        for (int idx = 0;idx < meshDataList->size();idx++)
        {
            MeshData *meshData = parseMeshDataFromBinary(meshDataList->Get(idx));
            slotData->displayDataList.push_back(meshData);
        }
    }
    return slotData;
}

MeshData* BinaryParser::parseMeshDataFromBinary(const MeshDataOption* meshDataOption) const
{
    MeshData *meshData = new MeshData();
    this->parseDisplayDataFromBinary(meshData, meshDataOption->displayData());
    meshData->setWidth(meshDataOption->width() / _armatureScale);
    meshData->setHeight(meshDataOption->height() / _armatureScale);
    auto triangles = meshDataOption->triangles();
    if (triangles)
    {
        for (int idx = 0; idx < triangles->size();idx++)
        {
            meshData->addTriangle(triangles->Get(idx));
        }
    }
    
    auto vectices = meshDataOption->vectices();
    if (vectices)
    {
        for (int idx = 0; idx < vectices->size();idx++)
        {
            auto pointOpt = vectices->Get(idx);
            Point vectice(pointOpt->x() / _armatureScale,pointOpt->y() / _armatureScale);
            meshData->addVectex(vectice);
        }
    }
    
    auto uvs = meshDataOption->uvs();
    if (uvs)
    {
        for (int idx = 0; idx < uvs->size();idx++)
        {
            auto pointOpt = uvs->Get(idx);
            Point uv(pointOpt->x(),pointOpt->y());
            meshData->addUV(uv);
        }
    }
    meshData->resetVisitData();
    
    return meshData;
}

DisplayData* BinaryParser::parseMainDisplayDataFromBinary(const DisplayDataOption* displayDataOption) const
{
    DisplayData *displayData = new DisplayData();
    this->parseDisplayDataFromBinary(displayData, displayDataOption);
    return displayData;
}


void BinaryParser::parseDisplayDataFromBinary(DisplayData *displayData,const DisplayDataOption* displayDataOption) const
{
    parseFlatStringToString(displayDataOption->name(), displayData->name);
    displayData->type = (DisplayType)displayDataOption->type();
    parseFlatStringToString(displayDataOption->slotName(), displayData->slotName);
    parseTransformFromBinary(displayDataOption->transform(), displayData->transform);
    parsePointFromBinary(displayDataOption->pivot(), displayData->pivot);
  
    

}

SkinData* BinaryParser::parseSkinDataFromBinary(const SkinDataOption* skinDataOption) const
{
    SkinData *skinData = new SkinData();
    parseFlatStringToString(skinDataOption->name(), skinData->name);
    auto slotDataList = skinDataOption->slotDataList();
    if (slotDataList)
    {
        for (int idx = 0;idx < slotDataList->size();idx++)
        {
            SlotData *slotData = parseSlotDataFromBinary(slotDataList->Get(idx));
            skinData->slotDataList.push_back(slotData);
        }
    }
    
    return skinData;
}

IKData*  BinaryParser::parseIKDataFromBinary(const IKDataOption* ikDataOption) const
{
    IKData* ikData = new IKData();
    parseFlatStringToString(ikDataOption->name(), ikData->name);
    parseFlatStringToString(ikDataOption->target(), ikData->target);
    parseFlatStringToString(ikDataOption->bone(), ikData->bone);
    ikData->chain = ikDataOption->chain();
    ikData->weight = ikDataOption->weight();
    ikData->bendPositive = ikDataOption->bendPositive();
    return ikData;
}

AnimationData* BinaryParser::parseAnimationDataFromBinary(const AnimationDataOption *animationDataOption) const
{
    std::string animationName = "";
    parseFlatStringToString(animationDataOption->name(), animationName);
    
    AnimationData *animationData = new AnimationData();
    
    auto frameList = animationDataOption->timeline()->frameList();
    if (frameList)
    {
        for (int idx = 0;idx < frameList->size();idx++)
        {
            Frame *frame = parseMainFrameFromBinary(frameList->Get(idx));
            animationData->frameList.push_back(frame);
        }
    }
    
    auto timelineList = animationDataOption->timelineList();
    if (timelineList)
    {
        for (int idx = 0;idx < timelineList->size();idx++)
        {
            TransformTimeline *timeline = parseTransformTimelineFromBinary(timelineList->Get(idx));
            animationData->timelineList.push_back(timeline);
        }
    }
    
    auto slotTimelineList = animationDataOption->slotTimelineList();
    if (slotTimelineList)
    {
        for (int idx = 0; idx < slotTimelineList->size();idx++)
        {
            SlotTimeline *timeline = parseSlotTimelineFromBinary(slotTimelineList->Get(idx));
            animationData->slotTimelineList.push_back(timeline);
        }
    }
    
    auto ffdTimelineList = animationDataOption->ffdTimelineList();
    if (ffdTimelineList)
    {
        for (int idx = 0; idx < ffdTimelineList->size();idx++)
        {
            FFDTimeline *timeline = parseFFDTimelineFromBinary(ffdTimelineList->Get(idx));
            animationData->ffdTimelineList.push_back(timeline);
        }
    }
    
    auto hideTimelineList = animationDataOption->hideTimelineList();
    if (hideTimelineList)
    {
        for (int idx = 0;idx < hideTimelineList->size();idx++)
        {
            animationData->hideTimelineList.push_back(hideTimelineList->Get(idx)->c_str());
        }
    }
    
    parseFlatStringToString(animationDataOption->name(), animationData->name);
    
    animationData->autoTween = animationDataOption->autoTween();
    animationData->frameRate = animationDataOption->frameRate();
    animationData->playTimes = animationDataOption->playTimes();
    animationData->fadeTime = animationDataOption->fadeTime();
    animationData->tweenEasing = animationDataOption->tweenEasing();
    animationData->lastFrameDuration = animationDataOption->lastFrameDuration();
    
    animationData->duration = animationDataOption->timeline()->duration();
    animationData->scale = animationDataOption->timeline()->scale();
    
    
    return animationData;
}

TransformTimeline* BinaryParser::parseTransformTimelineFromBinary(const TransformTimelineOption *timelineOption) const
{
    TransformTimeline *timeline = new TransformTimeline();
    
    auto timelist = timelineOption->timeline()->transformFrameList();
    if (timelist)
    {
        for (int idx = 0;idx < timelist->size();idx++)
        {
            TransformFrame *frame = parseTransformFrameFromBinary(timelist->Get(idx));
            timeline->frameList.push_back(frame);
        }
    }
    
    parseFlatStringToString(timelineOption->name(), timeline->name);
    
    timeline->transformed = timelineOption->transformed();
    timeline->offset = timelineOption->offset();
    timeline->duration = timelineOption->timeline()->duration();
    timeline->scale = timelineOption->timeline()->scale();
    
    
    parseTransformFromBinary(timelineOption->originTransform(), timeline->originTransform);
    parsePointFromBinary(timelineOption->originPivot(), timeline->originPivot);
    
    return timeline;
}

SlotTimeline* BinaryParser::parseSlotTimelineFromBinary(const SlotTimelineOption *timelineOption) const
{
    SlotTimeline* slotTimeline = new SlotTimeline();
    auto timelist = timelineOption->timeline()->slotFrameList();
    if (timelist)
    {
        for (int idx = 0;idx < timelist->size();idx++)
        {
            SlotFrame *frame = parseSlotFrameFromBinary(timelist->Get(idx));
            slotTimeline->frameList.push_back(frame);
        }
    }
    
    parseFlatStringToString(timelineOption->name(), slotTimeline->name);
    slotTimeline->offset = timelineOption->offset();
    slotTimeline->duration = timelineOption->timeline()->duration();
    slotTimeline->scale = timelineOption->timeline()->scale();
    slotTimeline->transformed = timelineOption->transformed();
    
    return slotTimeline;
}

FFDTimeline* BinaryParser::parseFFDTimelineFromBinary(const dragonBones::FFDTimelineOption *timelineOption) const
{
    FFDTimeline* ffdTimeline = new FFDTimeline();
    auto timelist = timelineOption->timeline()->ffdFrameList();
    if (timelist)
    {
        for (int idx = 0;idx < timelist->size();idx++)
        {
            FFDFrame *frame = parseFFDFrameFromBinary(timelist->Get(idx));
            ffdTimeline->frameList.push_back(frame);
        }
    }
    
    parseFlatStringToString(timelineOption->name(), ffdTimeline->name);
    parseFlatStringToString(timelineOption->skinName(), ffdTimeline->skinName);
    parseFlatStringToString(timelineOption->slotName(), ffdTimeline->slotName);
    ffdTimeline->duration = timelineOption->timeline()->duration();
    ffdTimeline->scale = timelineOption->timeline()->scale();
    ffdTimeline->offset = timelineOption->offset();
    return ffdTimeline;
}

FFDFrame* BinaryParser::parseFFDFrameFromBinary(const dragonBones::FFDFrameOption *ffdFrameOption) const
{
    FFDFrame* ffdFrame = new FFDFrame();
    parseFrameFromBinary(ffdFrameOption->frame(), ffdFrame);

    ffdFrame->tweenEasing = ffdFrameOption->tweenEasing();
    ffdFrame->offset = ffdFrameOption->offset();
    
    auto vectices = ffdFrameOption->vertices();
    if (vectices)
    {
        for (int idx = 0;idx < vectices->size();idx++)
        {
            ffdFrame->vertices.push_back(vectices->Get(idx) / _armatureScale);
        }
    }
    
    return ffdFrame;
}

Frame* BinaryParser::parseMainFrameFromBinary(const FrameOption *frameOption) const
{
    Frame *frame = new Frame();
    parseFrameFromBinary(frameOption, frame);
    return frame;
}


void BinaryParser::parseFrameFromBinary(const FrameOption *frameOption, Frame *frame) const
{
    frame->position = frameOption->position();
    frame->duration = frameOption->duration();
    frame->frameType = (Frame::FrameType)frameOption->frameType();
    
    parseFlatStringToString(frameOption->action(),frame->action);
    parseFlatStringToString(frameOption->event(),frame->event);
    parseFlatStringToString(frameOption->sound(),frame->sound);

    auto curveOption = frameOption->curve();
    if(curveOption)
    {
        frame->curve = new CurveData();
        
        for (int idx = 0; idx < curveOption->pointList()->size();idx++)
        {
            auto potOption = curveOption->pointList()->Get(idx);
            Point point (potOption->x(),potOption->y());
            frame->curve->_pointList.push_back(point);
        }
    }
}

TransformFrame* BinaryParser::parseTransformFrameFromBinary(const TransformFrameOption *transformFrameOption) const
{
    TransformFrame *frame = new TransformFrame();
    
    parseFrameFromBinary(transformFrameOption->frame(), frame);
    
    frame->visible = transformFrameOption->visible();
    frame->tweenEasing = transformFrameOption->tweenEasing();
    frame->tweenRotate = transformFrameOption->tweenRotate();
    frame->tweenScale = transformFrameOption->tweenScale();
    
    parseTransformFromBinary(transformFrameOption->transform(), frame->transform);
    parseTransformFromBinary(transformFrameOption->global(), frame->global);
    parsePointFromBinary(transformFrameOption->scaleOffset(), frame->scaleOffset);
    parsePointFromBinary(transformFrameOption->pivot(), frame->pivot);
    
    return frame;
}

SlotFrame* BinaryParser::parseSlotFrameFromBinary(const dragonBones::SlotFrameOption *slotFrameOption) const
{
    SlotFrame* slotFrame = new SlotFrame();
    parseFrameFromBinary(slotFrameOption->frame(), slotFrame);
    slotFrame->visible = slotFrameOption->visible();
    slotFrame->tweenEasing = slotFrameOption->tweenEasing();
    slotFrame->displayIndex = slotFrameOption->displayIndex();
    slotFrame->zOrder = slotFrameOption->zOrder();
    
    if (slotFrameOption->color())
    {
        auto color = parseColorTransformFromBinary(slotFrameOption->color());
        slotFrame->color = color;
    }
    
    return slotFrame;
}

ColorTransform* BinaryParser::parseColorTransformFromBinary(const ColorTransformOption *colorTransformOption) const
{
    ColorTransform* colorTransform = new ColorTransform();
    colorTransform->alphaOffset = colorTransformOption->alphaOffset();
    colorTransform->redOffset = colorTransformOption->redMultiplier();
    colorTransform->greenOffset = colorTransformOption->greenOffset();
    colorTransform->blueOffset = colorTransformOption->blueOffset();
    colorTransform->alphaMultiplier = colorTransformOption->alphaMultiplier();
    colorTransform->redMultiplier = colorTransformOption->redMultiplier();
    colorTransform->greenMultiplier = colorTransformOption->greenMultiplier();
    colorTransform->blueMultiplier = colorTransformOption->blueMultiplier();
    return colorTransform;
}


void BinaryParser::parseFlatStringToString(const flatbuffers::String *flatString,std::string &stdString) const
{
    if (flatString)
    {
        stdString = flatString->c_str();
    }
}

void BinaryParser::parseTransformFromBinary(const TransformOption *transformOption, Transform &transform) const
{
    transform.x = transformOption->x() / _armatureScale;
    transform.y = transformOption->y() / _armatureScale;
    transform.skewX = transformOption->skewX();
    transform.skewY = transformOption->skewY();
    transform.scaleX = transformOption->scaleX();
    transform.scaleY = transformOption->scaleY();
}

void BinaryParser::parsePointFromBinary(const PointOption *pointOption, Point &point) const
{
    point.x = pointOption->x() / _armatureScale;
    point.y = pointOption->y() / _armatureScale;
}


NAME_SPACE_DRAGON_BONES_END