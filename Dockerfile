FROM docker.io/library/rockylinux:8.9

RUN dnf group -y install "Development tools"
RUN dnf install -y cmake

RUN useradd --create-home --no-log-init -u 4000 --shell /bin/bash gitlab-ci

WORKDIR /home/gitlab-ci

RUN mkdir -p .ssh && \
    chmod 0700 .ssh && \
    chown -R gitlab-ci:gitlab-ci /home/gitlab-ci && \
    ssh-keyscan gitlab.codasip.com >> .ssh/known_hosts

USER gitlab-ci
