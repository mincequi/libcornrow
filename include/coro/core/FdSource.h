#pragma once

#include <gst/base/gstpushsrc.h>

#include <stdint.h>

#include <queue>

G_BEGIN_DECLS

#define CR_TYPE_FD_SOURCE    (cr_fd_source_get_type())
#define CR_FD_SOURCE(obj)    (G_TYPE_CHECK_INSTANCE_CAST((obj),CR_TYPE_FD_SOURCE,CrFdSource))
#define CR_FD_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),CR_TYPE_FD_SOURCE,CrFdSourceClass))

typedef struct _CrFdSource      CrFdSource;
typedef struct _CrFdSourceClass CrFdSourceClass;

struct _CrFdSourceClass
{
    GstPushSrcClass parentClass;
};

class _CrFdSource
{
public:
    GstPushSrc pushSource;

    void init(int fd, uint32_t blockSize, uint8_t allocFactor = 1);
    GstBuffer* readFd();

    volatile gint m_isFlushing;

    // FD related members
    int         m_fd = -1;
    GstPoll*    m_poll = nullptr;
    GstPollFD   m_pfd;
    uint32_t    m_blockSize = 4096;
    uint8_t     m_allocFactor = 1;

    int         m_currentPacketSize = -1;

    std::queue<GstBuffer*> m_pendingBuffers;
};

GType cr_fd_source_get_type (void);

G_END_DECLS
