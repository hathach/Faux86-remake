/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#pragma once

#include "HostSystemInterface.h"
#include "Renderer.h"
#include "SerialMouse.h"

#include <circle/input/mouse.h>

class CKernel;
class CUSBKeyboardDevice;
class CMouseDevice;
class CDeviceNameService;
class CInterruptSystem;
class C2DGraphics;
class CBcmFrameBuffer;
class CVCHIQDevice;
class CScreenDevice;

namespace Faux86
{
  class PWMSound;
  class VCHIQSound;

  class CircleFrameBufferInterface : public FrameBufferInterface
  {
  public:
    virtual void init(uint32_t desiredWidth, uint32_t desiredHeight) override;
    virtual void resize(uint32_t desiredWidth, uint32_t desiredHeight) override;
    virtual RenderSurface *getSurface() override;
    virtual void setPalette(Palette *palette) override;
// virtual void present() override;
    virtual void blit(uint16_t *pixels, int w, int h, int stride) override;

    CBcmFrameBuffer *frameBuffer = nullptr;

  private:
    // CBcmFrameBuffer* frameBuffer = nullptr;
    RenderSurface *surface;
  };

  class CircleAudioInterface : public AudioInterface
  {
  public:
    CircleAudioInterface(CInterruptSystem &inInterruptSystem, CVCHIQDevice &inVchiqDevice) : interruptSystem(inInterruptSystem), vchiqDevice(inVchiqDevice) {}
    virtual void init(VM &vm) override;
    virtual void shutdown() override;

  private:
    PWMSound *pwmSound = nullptr;
    VCHIQSound *vchiqSound = nullptr;
    CInterruptSystem &interruptSystem;
    CVCHIQDevice &vchiqDevice;
  };

  class CircleTimerInterface : public TimerInterface
  {
  public:
    CircleTimerInterface();
    virtual uint64_t getHostFreq() override;
    virtual uint64_t getTicks() override;

  private:
    uint64_t currentTick = 0;
    uint64_t lastTimerSample = 0;
  };

  class CircleHostInterface : public HostSystemInterface
  {
  public:
    // CircleHostInterface(CDeviceNameService& deviceNameService, CInterruptSystem& interruptSystem, CVCHIQDevice& inVchiqDevice, CScreenDevice& screenDevice, C2DGraphics 2DGraphics);
    // CircleHostInterface(CDeviceNameService& deviceNameService, CInterruptSystem& interruptSystem, CVCHIQDevice& inVchiqDevice, CScreenDevice& screenDevice);
    CircleHostInterface(CKernel &kernelInstance, CDeviceNameService &deviceNameService, CInterruptSystem &interruptSystem, CVCHIQDevice &inVchiqDevice, CScreenDevice &screenDevice);

    virtual void init(VM *inVM) override;
    virtual void resize(uint32_t desiredWidth, uint32_t desiredHeight) override;
    // void tick(VM& vm);
    void tick();

    virtual FrameBufferInterface &getFrameBuffer() override { return frameBuffer; }
    virtual TimerInterface &getTimer() override { return timer; }
    virtual AudioInterface &getAudio() override { return audio; }
    virtual DiskInterface *openFile(const char *filename) override;

  private:
    enum class EventType
    {
      KeyPress,
      KeyRelease,
      MousePress,
      MouseRelease,
      MouseMove
    };

    struct InputEvent
    {
      EventType eventType;

      union
      {
        u16 scancode;
        SerialMouse::ButtonType mouseButton;
        struct
        {
          // s8 mouseMotionX;
          // s8 mouseMotionY;
          int mouseMotionX;
          int mouseMotionY;
        };
      };
    };

    void queueEvent(InputEvent &inEvent);
    void queueEvent(EventType eventType, u16 scancode);

    static void mouseEventHandler(TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY, int nWheelMove);
    static void mouseStatusHandler(unsigned nButtons, int nDisplacementX, int nDisplacementY, int nWheelMove);
    static void keyStatusHandlerRaw(unsigned char ucModifiers, const unsigned char RawKeys[6]);
    static void keyRemovedHandler(CDevice *pDevice, void *pContext);
    static void mouseRemovedHandler(CDevice *pDevice, void *pContext);
    static void shutdownHandler(void);

    CircleFrameBufferInterface frameBuffer;
    CircleAudioInterface audio;
    CircleTimerInterface timer;

    static CircleHostInterface *instance;

    CKernel *kernel;
    CScreenDevice *screen;
    C2DGraphics *p2DGraphics;
    CUSBKeyboardDevice *keyboard;
    CMouseDevice *mouse;

    static constexpr int MaxInputBufferSize = 16;
    // static constexpr int MaxInputBufferSize = 8;

    uint8_t lastModifiers;
    uint8_t lastRawKeys[6];
    InputEvent inputBuffer[MaxInputBufferSize];
    int inputBufferPos = 0;
    int inputBufferSize = 0;
    unsigned lastMouseButtons = 0;
    unsigned m_MouseX = 0;
    unsigned m_MouseY = 0;
  };
}
