#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "ssl/SSLContext.h"
#include "ssl/SSLManager.h"
#include "net/EventLoop.h"
#include "net/Application.h"
#include "net/Connection.h"
#include "util/Logger.h"

void NewSSLConnection(const std::string& ctxName, int verifyMode, bool incoming, ananas::Connection* c) {
    ananas::ssl::OnNewSSLConnection(ctxName, verifyMode, incoming, c);

    auto open = c->GetUserData<ananas::ssl::OpenSSLContext>();
    open->SetLogicProcess([](ananas::Connection* c, const char* data, size_t len) {
        std::cout << "Process len " << len << std::endl;
        std::cout << "Process data " << data << std::endl;
        return len;
    });
}

int main(int ac, char* av[]) {
    ananas::LogManager::Instance().Start();

    using ananas::ssl::SSLManager;
    SSLManager::Instance().GlobalInit();

    const char* ctx = "serverctx";
    if (!SSLManager::Instance().AddCtx(ctx, "ca.pem", "server-cert.pem", "server-key.pem")) {
        std::cerr << "Load certs failed\n";
        return -1;
    }

    auto& app = ananas::Application::Instance();

    app.Listen("loopback", 8443,  std::bind(NewSSLConnection,
                                            ctx,
                                            SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                                            true,
                                            std::placeholders::_1));

    app.Run(ac, av);
    return 0;
}

