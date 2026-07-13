FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    python3 \
    python3-pip \
    irssi \
    netcat-openbsd \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN pip3 install pytest pytest-cov pexpect

RUN make

EXPOSE 6667

CMD ["python3", "tests/run_all.py", "--build", "--cov"]