#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 5054)
#pragma warning(disable : 4996)
#pragma warning(disable : 4127)
#pragma warning(disable : 5054)
#endif

#include <clickhouse.grpc.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <server.grpc.pb.h>

#if defined(_WIN32)
#pragma warning(pop)
#endif

class Service : public Greeter::Service {
 public:
  ::grpc::Status SayHello(::grpc::ServerContext*,
                          const ::HelloRequest* request,
                          ::HelloReply* response) override {
    std::cout << "Greeter service received request to greet \""
              << request->name() << "\"\n";
    response->set_message("Hello, " + request->name() + "!");
    return grpc::Status::OK;
  }
};

int main() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051",
                           grpc::InsecureServerCredentials());

  Service service;
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}