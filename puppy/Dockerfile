#check=skip=JSONArgsRecommended

FROM debian:13-slim AS builder
RUN apt-get update && DEBIAN_FRONTEND=noninteracive apt-get install -y \
    --no-install-recommends cmake g++-mingw-w64-i686-posix g++ make && \
    apt-get clean && rm -rf /var/lib/apt/lists/*;

WORKDIR /build
COPY app app
COPY include include
COPY src src
COPY CMakeLists.txt .

RUN g++ -o healthcheck -I./include ./app/healthcheck.cpp;

RUN --mount=type=cache,target=cmake-build cmake -B cmake-build \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ && cmake --build cmake-build -j$(nproc) && \
    cp cmake-build/promt-puppy.exe .;


FROM debian:13-slim AS runner
ENV WINEPREFIX=/wineprefix \
    XDG_RUNTIME_DIR=/tmp/ \
    DEBIAN_FRONTEND=noninteracive \
    WINEDEBUG=-all

RUN dpkg --add-architecture i386 && apt-get update &&  \
    apt-get install -y --no-install-recommends \
    wine wine64 wine32:i386 && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    mkdir -p /tmpfs $WINEPREFIX/drive_c && ln -sf /tmpfs $WINEPREFIX/drive_c/tmpfs;

COPY build/ $WINEPREFIX/drive_c

WORKDIR /app
COPY --from=builder /build/promt-puppy.exe /build/healthcheck .
HEALTHCHECK CMD ./healthcheck;

EXPOSE 80/tcp
VOLUME /cache

COPY docker-entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
