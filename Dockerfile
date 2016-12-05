FROM ubuntu:15.10

RUN apt-get update && \
apt-get install -y libssl-dev

ADD build/cpp/osmosis.bin /osmosis.bin
EXPOSE 1010
STOPSIGNAL SIGINT
ENTRYPOINT ["/osmosis.bin", "server"]
CMD ["--objectStoreRootPath", "/data"]

