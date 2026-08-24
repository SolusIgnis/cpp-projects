# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

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
    gnupg \
    git \
    ninja-build \
    python3 \
    build-essential \
    ca-certificates \
    locales \
    curl \
    libssl-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Ensure consistent locale (avoids weird sorting / tool output issues)
RUN locale-gen en_US.UTF-8 && \
    update-locale LANG=en_US.UTF-8

ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# ------------------------------------------------------------
# 2. Install Clang 23 + tools
# ------------------------------------------------------------
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 23 && \
    apt-get update && apt-get install -y \
        clang-23 \
        clang-format-23 \
        clang-tidy-23 \
        lldb-23 \
        lld-23 \
        libc++-23-dev \
        libc++abi-23-dev && \
    rm -rf /var/lib/apt/lists/* && \
    rm llvm.sh

# ------------------------------------------------------------
# 3. Set Clang 23 as default
# ------------------------------------------------------------
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-23 65536 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-23 65536 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-23 65536 && \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-23 65536 && \
    update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-23 65536 && \
    update-alternatives --set clang /usr/bin/clang-23 && \
    update-alternatives --set clang++ /usr/bin/clang++-23 && \
    update-alternatives --set clang-format /usr/bin/clang-format-23 && \
    update-alternatives --set clang-tidy /usr/bin/clang-tidy-23

# ------------------------------------------------------------
# 4. Install CMake 4.4.2
# ------------------------------------------------------------
RUN wget https://github.com/Kitware/CMake/releases/download/v4.4.2/cmake-4.4.2-linux-x86_64.sh && \
    echo "d238e3da6b160838bd0d4cb5b2bb085e0099d31512e15c56b1dd412ad82779cb  cmake-4.4.2-linux-x86_64.sh" | sha256sum -c - && \
    chmod +x cmake-4.4.2-linux-x86_64.sh && \
    ./cmake-4.4.2-linux-x86_64.sh --skip-license --prefix=/usr/local && \
    rm cmake-4.4.2-linux-x86_64.sh

# ------------------------------------------------------------
# 5. Install Asio
# ------------------------------------------------------------
WORKDIR /opt
RUN git clone --depth=1 --branch asio-1-36-0 https://github.com/chriskohlhoff/asio.git && \
    cp -r asio/asio/include/* /usr/local/include/ && \
    rm -rf asio

# ------------------------------------------------------------
# 6. Default working directory for CI
# ------------------------------------------------------------
WORKDIR /workspace
