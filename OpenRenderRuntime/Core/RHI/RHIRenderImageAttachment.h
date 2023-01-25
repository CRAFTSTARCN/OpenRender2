#pragma once

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

/*
 * The render image attachment is another proxy of image
 * Your image cannot be create directly from RHITexImage because this will cause some problem
 */
class RHIRenderImageAttachment
{
    
public:
    
    RHITexImage* AttachmentImage = nullptr;
    RHITexImageView* DefaultView = nullptr;
    RenderImageAttachmentType AttachmentType = RenderImageAttachmentType_Color_Attachment;
};