#pragma once

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

struct Server {
    std::unique_ptr<grpc::Server> server;
    Server(const std::string& uri, grpc::Service* service)
    {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(uri, grpc::InsecureServerCredentials());
        builder.RegisterService(service);
        server = builder.BuildAndStart();
    }
    ~Server()
    {
        server->Shutdown();
    }
};
