//
//  Copyright (c) 2011 Tweak Software.
//  All rights reserved.
//
//  SPDX-License-Identifier: Apache-2.0
//
//

#ifdef PLATFORM_WINDOWS
#include <GL/glew.h>
#include <GL/wglew.h>
#endif
#include <TwkGLF/GL.h>
#include <RvCommon/QTGLVideoDevice.h>
#include <RvCommon/GLView.h>
#include <RvCommon/DesktopVideoDevice.h>
#include <TwkGLF/GLFBO.h>
#include <IPCore/Session.h>
#include <TwkApp/Application.h>
#include <TwkApp/VideoModule.h>

#include <QScreen>

namespace Rv
{
    using namespace std;
    using namespace TwkGLF;
    using namespace TwkApp;

    QTGLVideoDevice::QTGLVideoDevice(VideoModule* m, const string& name,
                                     QOpenGLWidget* view)
        : GLVideoDevice(m, name, ImageOutput | ProvidesSync | SubWindow)
        , m_view(view)
        , m_translator(new QTTranslator(this, view))
        , m_x(0)
        , m_y(0)
        , m_refresh(-1.0)
    {
        assert(view);
    }

    QTGLVideoDevice::QTGLVideoDevice(VideoModule* m, const string& name)
        : GLVideoDevice(m, name, ImageOutput | ProvidesSync | SubWindow)
        , m_view(0)
        , m_translator(0)
        , m_x(0)
        , m_y(0)
    {
    }

    QTGLVideoDevice::QTGLVideoDevice(const string& name, QOpenGLWidget* view)
        : GLVideoDevice(NULL, name, NoCapabilities)
        , m_view(view)
        , m_translator(new QTTranslator(this, view))
        , m_x(0)
        , m_y(0)
        , m_refresh(-1.0)
    {
        assert(view);
    }

    QTGLVideoDevice::~QTGLVideoDevice() { delete m_translator; }

    void QTGLVideoDevice::setWidget(QOpenGLWidget* widget)
    {
        m_view = widget;
        m_translator = new QTTranslator(this, m_view);
    }

    GLVideoDevice* QTGLVideoDevice::newSharedContextWorkerDevice() const
    {
        // NOTE_QT: QOpenGLWidget does not take a share parameter anymore. Try
        // to share with setShareContext.
        QOpenGLWidget* openGLWidget = new QOpenGLWidget(m_view->parentWidget());
        openGLWidget->context()->setShareContext(m_view->context());
        return new QTGLVideoDevice(name() + "-workerContextDevice",
                                   openGLWidget);
    }

    void QTGLVideoDevice::makeCurrent() const
    {
        if (m_view->context() && m_view->context()->isValid())
        {
            m_view->makeCurrent();
            TWK_GLDEBUG;

            GLint widgetFBO = m_view->defaultFramebufferObject();
            if (widgetFBO != 0)
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, widgetFBO);
            TWK_GLDEBUG;
        }

        if (!isWorkerDevice())
            GLVideoDevice::makeCurrent();
    }

    GLuint QTGLVideoDevice::fboID() const
    {
        if (m_view)
            return m_view->defaultFramebufferObject();

        return 0;
    }

    void QTGLVideoDevice::setPhysicalDevice(VideoDevice* d)
    {
        TwkGLF::GLVideoDevice::setPhysicalDevice(d);

        // Update the DevicePixelRatio associated with the physical device.
        // This is important for high DPI displays, where the device pixel ratio
        // may differ from the logical pixel ratio on a per screen basis.
        m_devicePixelRatio = 1.0f;

        static bool noQtHighDPISupport =
            getenv("RV_NO_QT_HDPI_SUPPORT") != nullptr;
        if (noQtHighDPISupport)
        {
            return;
        }

        if (const DesktopVideoDevice* desktopVideoDevice =
                dynamic_cast<const DesktopVideoDevice*>(d))
        {
            const QList<QScreen*> screens = QGuiApplication::screens();
            if (desktopVideoDevice->qtScreen() < screens.size())
            {
                m_devicePixelRatio =
                    screens[desktopVideoDevice->qtScreen()]->devicePixelRatio();
            }
        }
    }

    void QTGLVideoDevice::redraw() const
    {
        if (!isWorkerDevice())
        {
            QSize s = m_view->size();
            m_view->update();
        }
    }

    void QTGLVideoDevice::redrawImmediately() const
    {
        if (!isWorkerDevice())
        {
            if (m_view->isVisible())
            {
#ifdef PLATFORM_DARWIN
                // Make sure that the QGLWidget gets redrawn by updateGL() even
                // when completely overlapped by another window.
                // Note that on macOS, Qt correctly detects when the QGLWidget
                // is completely overlapped by another window and in which case
                // resets the Qt::WA_Mapped attribute. This will prevent the
                // GLView::paintGL() operation from being called by
                // m_view->updateGL(), which will result in automatically
                // interrupting any video playback that might be in progress
                // while the RV window is completely overlapped. This is an
                // undesirable behaviour during a review session, especially if
                // an external video output device is used.
                m_view->setAttribute(Qt::WA_Mapped);
#endif

                m_view->update();
            }
            else
            {
                redraw();
            }
        }
    }

    VideoDevice::Resolution QTGLVideoDevice::resolution() const
    {
        return Resolution(m_view->width() * devicePixelRatio(),
                          m_view->height() * devicePixelRatio(), 1.0f, 1.0f);
    }

    VideoDevice::Resolution QTGLVideoDevice::internalResolution() const
    {
        return Resolution(m_view->width(), m_view->height(), 1.0f, 1.0f);
    }

    VideoDevice::Offset QTGLVideoDevice::offset() const
    {
        return Offset(m_x, m_y);
    }

    VideoDevice::Timing QTGLVideoDevice::timing() const
    {
        return Timing((m_refresh != -1.0) ? m_refresh : 0.0);
    }

    VideoDevice::VideoFormat QTGLVideoDevice::format() const
    {
        return VideoFormat(m_view->width() * devicePixelRatio(),
                           m_view->height() * devicePixelRatio(), 1.0, 1.0,
                           (m_refresh != -1.0) ? m_refresh : 0.0,
                           hardwareIdentification());
    }

    void QTGLVideoDevice::open(const StringVector& args)
    {
        if (!isWorkerDevice())
            m_view->show();
    }

    void QTGLVideoDevice::close()
    {
        if (!isWorkerDevice())
            m_view->hide();
    }

    bool QTGLVideoDevice::isOpen() const
    {
        if (isWorkerDevice())
        {
            return false;
        }
        else
        {
            return m_view->isVisible();
        }
    }

    size_t QTGLVideoDevice::width() const
    {
        return m_view->width() * devicePixelRatio();
    }

    size_t QTGLVideoDevice::height() const
    {
        return m_view->height() * devicePixelRatio();
    }

    void QTGLVideoDevice::syncBuffers() const
    {
        if (!isWorkerDevice())
        {
            makeCurrent();
            m_view->context()->swapBuffers(m_view->context()->surface());
        }
    }

    void QTGLVideoDevice::setAbsolutePosition(int x, int y)
    {
        if (isWorkerDevice())
            return;

        if (x != m_x || y != m_y || m_refresh == -1.0)
        {
            float refresh = -1.0;

            VideoDevice::Resolution res = internalResolution();
            int tx = x + res.width / 2;
            int ty = y + res.height / 2;

            if (const TwkApp::VideoModule* module =
                    TwkApp::App()->primaryVideoModule())
            {
                if (TwkApp::VideoDevice* d = module->deviceFromPosition(tx, ty))
                {
                    setPhysicalDevice(d);
                    refresh = d->timing().hz;

                    VideoDeviceContextChangeEvent event("video-device-changed",
                                                        this, this, d);
                    sendEvent(event);
                }
            }

            if (refresh != m_refresh)
            {
                // Check to see refresh has a legal value before assigning it to
                // m_refresh.
                if (refresh > 0)
                {
                    m_refresh = refresh;
                }
                else
                {
                    if (IPCore::debugPlayback)
                        cout << "WARNING: ignoring intended desktop refresh "
                                "rate = "
                             << refresh << endl;
                }

                if (IPCore::debugPlayback)
                    cout << "INFO: new desktop refresh rate " << m_refresh
                         << endl;
            }
        }
        m_x = x;
        m_y = y;
    }

} // namespace Rv
