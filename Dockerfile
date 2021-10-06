# Using builder pattern as instructed by https://www.youtube.com/watch?v=wGz_cbtCiEA
FROM alpine:3.13.6 AS build-env

RUN apk add cmake make g++ sqlite-dev linux-headers asio-dev
WORKDIR /app
ADD . /app
WORKDIR /app/build
# There is a phantom bug that causes the app to crash if build type is release
# Not worth debugging a 4 year old project for hours to figure out what's causing it
RUN cmake -DCMAKE_BUILD_TYPE=Debug ..
RUN make


FROM alpine:3.13.6
RUN apk add sqlite-libs libstdc++ libgcc
WORKDIR /app
COPY --from=build-env /app/build/webchat-server /app
COPY --from=build-env /app/index.html /app
RUN mkdir -p db/chatrooms resource

EXPOSE 8000
CMD ["./webchat-server", "-p", "8000"]