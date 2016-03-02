/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Request.h"
#include "Response.h"

#include "SkCommandLineFlags.h"

#include "microhttpd.h"

#include "urlhandlers/UrlHandler.h"
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace Response;

// To get image decoders linked in we have to do the below magic
#include "SkForceLinking.h"
#include "SkImageDecoder.h"
__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_int32(port, 8888, "The port to listen on.");
DEFINE_string(address, "localhost", "The address to bind to.");

class UrlManager {
public:
    UrlManager() {
        // Register handlers
        fHandlers.push_back(new RootHandler);
        fHandlers.push_back(new PostHandler);
        fHandlers.push_back(new ImgHandler);
        fHandlers.push_back(new ClipAlphaHandler);
        fHandlers.push_back(new EnableGPUHandler);
        fHandlers.push_back(new CmdHandler);
        fHandlers.push_back(new InfoHandler);
        fHandlers.push_back(new DownloadHandler);
        fHandlers.push_back(new DataHandler);
        fHandlers.push_back(new BreakHandler);
        fHandlers.push_back(new BatchesHandler);
        fHandlers.push_back(new BatchBoundsHandler);
    }

    ~UrlManager() {
        for (int i = 0; i < fHandlers.count(); i++) { delete fHandlers[i]; }
    }

    // This is clearly not efficient for a large number of urls and handlers
    int invoke(Request* request, MHD_Connection* connection, const char* url, const char* method,
               const char* upload_data, size_t* upload_data_size) const {
        for (int i = 0; i < fHandlers.count(); i++) {
            if (fHandlers[i]->canHandle(method, url)) {
                return fHandlers[i]->handle(request, connection, url, method, upload_data,
                                            upload_data_size);
            }
        }
        return MHD_NO;
    }

private:
    SkTArray<UrlHandler*> fHandlers;
};

const UrlManager kUrlManager;

int answer_to_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method, const char* version,
                         const char* upload_data, size_t* upload_data_size,
                         void** con_cls) {
    SkDebugf("New %s request for %s using version %s\n", method, url, version);

    Request* request = reinterpret_cast<Request*>(cls);
    int result = kUrlManager.invoke(request, connection, url, method, upload_data,
                                    upload_data_size);
    if (MHD_NO == result) {
        fprintf(stderr, "Invalid method and / or url: %s %s\n", method, url);
    }
    return result;
}

int skiaserve_main() {
    Request request(SkString("/data")); // This simple server has one request

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(FLAGS_port);
    inet_pton(AF_INET, FLAGS_address[0], &address.sin_addr);

    printf("Visit http://%s:%d in your browser.\n", FLAGS_address[0], FLAGS_port);

    struct MHD_Daemon* daemon;
    // TODO Add option to bind this strictly to an address, e.g. localhost, for security.
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, FLAGS_port, nullptr, nullptr,
                              &answer_to_connection, &request,
                              MHD_OPTION_SOCK_ADDR, &address,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        return 1;
    }

    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return skiaserve_main();
}
#endif
