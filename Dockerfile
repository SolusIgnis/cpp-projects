# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /opt

# ------------------------------------------------------------
# 1. Base dependencies
# ------------------------------------------------------------
RUN apt-get update && apt-get install -y \
    wget \
    lsb-release \
    software-properties-common \
    git \
    ninja-build \
    python3 \
    build-essential \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------
# 2. Install Clang 21 + tools
# ------------------------------------------------------------
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 21 && \
    apt-get update && apt-get install -y \
        clang-21 \
        clang++-21 \
        clang-format-21 \
        clang-tidy-21 \
        lldb-21 \
        lld-21 && \
    rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------
# 3. Set Clang 21 as default
# ------------------------------------------------------------
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-21 65536 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-21 65536 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-21 65536 && \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-21 65536 && \
    update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-21 65536 && \
    update-alternatives --set clang /usr/bin/clang-21 && \
    update-alternatives --set clang++ /usr/bin/clang++-21 && \
    update-alternatives --set clang-format /usr/bin/clang-format-21 && \
    update-alternatives --set clang-tidy /usr/bin/clang-tidy-21

# ------------------------------------------------------------
# 4. Install CMake 4.1.2
# ------------------------------------------------------------
RUN wget https://github.com/Kitware/CMake/releases/download/v4.1.2/cmake-4.1.2-linux-x86_64.sh && \
    chmod +x cmake-4.1.2-linux-x86_64.sh && \
    ./cmake-4.1.2-linux-x86_64.sh --skip-license --prefix=/usr/local && \
    rm cmake-4.1.2-linux-x86_64.sh

# ------------------------------------------------------------
# 5. Build libc++
# ------------------------------------------------------------
RUN git clone --depth=1 --branch llvmorg-21.1.8 https://github.com/llvm/llvm-project.git && \
    mkdir llvm-project/build

WORKDIR /opt/llvm-project/build

RUN cmake -G Ninja -S ../runtimes -B . \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_INSTALL_PREFIX=/opt/libcxx-21

RUN cmake --build . --parallel --target cxx cxxabi unwind

RUN cmake --build . --parallel --target install-cxx install-cxxabi install-unwind

# ------------------------------------------------------------
# 6. Install libc++ into LLVM tree
# ------------------------------------------------------------
RUN cp -rf /opt/libcxx-21/* /usr/lib/llvm-21

# ------------------------------------------------------------
# 7. Install Asio
# ------------------------------------------------------------
WORKDIR /opt
RUN git clone --depth=1 --branch asio-1-36-0 https://github.com/chriskohlhoff/asio.git && \
    cp -r asio/asio/include/* /usr/local/include/

# ------------------------------------------------------------
# 8. Environment variables
# ------------------------------------------------------------
ENV LIBCXX_ROOT=/opt/libcxx-21
ENV CPLUS_INCLUDE_PATH=/opt/libcxx-21/include/c++/v1:$CPLUS_INCLUDE_PATH
ENV LIBRARY_PATH=/opt/libcxx-21/lib:$LIBRARY_PATH
ENV LD_LIBRARY_PATH=/opt/libcxx-21/lib:$LD_LIBRARY_PATH

# ------------------------------------------------------------
# 9. Default working directory for CI
# ------------------------------------------------------------
WORKDIR /workspace
