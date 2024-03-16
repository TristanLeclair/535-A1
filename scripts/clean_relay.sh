docker kill relay
docker rm relay
docker build -t zcs .
docker run -itd --name relay zcs
