#include <server.grpc.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

class Service : public Greeter::Service {
 public:
  ::grpc::Status SayHello(::grpc::ServerContext* context, const ::HelloRequest* request, ::HelloReply* response) override {
    std::cout << "Greeter service received request to greet \"" << request->name() << "\"\n";
    response->set_message("Hello, " + request->name() + "!");
    return grpc::Status::OK;
  }
};

int main(int argc, char** argv) {
  (void) argc;
  (void) argv;

  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());

  Service service;
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}