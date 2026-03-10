FROM gcc:14

WORKDIR /app
COPY . .

RUN apt-get update \
    && apt-get install -y cmake \
    && rm -rf /var/lib/apt/lists/*

RUN cmake -S . -B build && cmake --build build

ENTRYPOINT ["./build/pathfinding_visualizer"]
