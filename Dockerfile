FROM alpine:3.19.1

# RUN apt update && apt install -y git gcc make
RUN apk add --no-cache gcc g++ make libc-dev

WORKDIR /app

COPY include/ include/
COPY Makefile .
COPY src/ src/

RUN make all
