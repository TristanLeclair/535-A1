FROM alpine:latest

# RUN apt update && apt install -y git gcc make
RUN apk add --no-cache gcc g++ make libc-dev

WORKDIR /app

COPY src/ src/
COPY include/ include/
COPY Makefile .

EXPOSE 8080/udp 5000/udp

RUN make all
