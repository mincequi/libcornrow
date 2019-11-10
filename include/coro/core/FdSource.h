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

    void init(uint32_t sampleRate, int fd, uint32_t blockSize, uint8_t allocFactor = 1);

    GstBuffer* readFd();
    void pushConf(uint32_t sampleRate);

    volatile gint m_isFlushing;

    // FD related members
    int         m_fd;
    GstPoll*    m_poll;
    GstPollFD   m_pfd;
    uint32_t    m_blockSize;
    uint8_t     m_allocFactor;

    int         m_currentPacketSize;

    std::queue<GstBuffer*> m_pendingBuffers;
};

GType cr_fd_source_get_type (void);

G_END_DECLS
