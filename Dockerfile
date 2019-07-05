FROM ubuntu:xenial as build

RUN apt-get -qq update && apt-get install pkg-config zip g++ zlib1g-dev unzip python wget git python-numpy -yq

RUN wget https://github.com/bazelbuild/bazel/releases/download/0.24.1/bazel-0.24.1-installer-linux-x86_64.sh -O bazel-install
RUN bash bazel-install

WORKDIR "/app/"

COPY . .

RUN bazel build -c opt //cc_grpc_server:remote_agent

FROM gcr.io/distroless/cc:latest

COPY --from=build /app/bazel-bin/cc_grpc_server/remote_agent /

ENTRYPOINT ["/remote_agent"]
