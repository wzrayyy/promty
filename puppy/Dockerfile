#check=skip=JSONArgsRecommended
FROM debian:12-slim AS builder
RUN apt-get update && DEBIAN_FRONTEND=noninteracive apt-get install -y \
    --no-install-recommends cmake g++-mingw-w64-i686-posix make && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .
RUN --mount=type=cache,target=cmake-build cmake -B cmake-build \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ && cmake --build cmake-build -j$(nproc) && \
    cp cmake-build/promt-puppy.exe .

FROM debian:12-slim AS runner
ENV WINEPREFIX=/wineprefix \
    DEBIAN_FRONTEND=noninteracive
RUN sed -i 's/main/main contrib/' /etc/apt/sources.list.d/debian.sources && \
    dpkg --add-architecture i386 && apt-get update &&  \
    apt-get install -y --no-install-recommends \
    winetricks wine64 wine32:i386 xvfb xauth ca-certificates cabextract && \
    wineboot -i && echo "Sleeping for 15 seconds..." && sleep 15

COPY build/ $WINEPREFIX/drive_c
RUN xvfb-run -a winetricks -q mfc42 && \
    wine regedit $WINEPREFIX/drive_c/registry.reg && \
    apt remove -y --autoremove --purge xvfb winetricks && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    ln -sf /cache $WINEPREFIX/drive_c/cache && \
    mkdir /tmpfs && ln -sf /tmpfs $WINEPREFIX/drive_c/tmpfs && \
    echo "Sleeping for 15 seconds" && sleep 15

WORKDIR /app

COPY --from=builder /build/promt-puppy.exe .

EXPOSE 80/tcp
VOLUME /cache

CMD exec wine promt-puppy.exe
