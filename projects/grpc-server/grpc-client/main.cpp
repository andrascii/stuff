#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable:5054)
#pragma warning(disable:4996)
#pragma warning(disable:4127)
#pragma warning(disable:5054)
#pragma warning(disable:4251)
#endif

#include <server.grpc.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#if defined(_WIN32)
#pragma warning(pop)
#endif

class Service : public Greeter::Service {
 public:
  ::grpc::Status SayHello(::grpc::ServerContext*, const ::HelloRequest* request, ::HelloReply* response) override {
    std::cout << "Greeter service received request to greet \"" << request->name() << "\"\n";
    response->set_message("Hello, " + request->name() + "!");
    return grpc::Status::OK;
  }
};

int main() {
  HelloRequest request;
  request.set_name("John");

  HelloReply response;

  auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
  std::unique_ptr<Greeter::Stub> stub = Greeter::NewStub(channel);

  grpc::ClientContext context;
  grpc::Status status = stub->SayHello(&context, request, &response);

  // Output result
  std::cout << "I got: " << response.message() << std::endl;
}