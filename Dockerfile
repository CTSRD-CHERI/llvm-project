FROM rockylinux/rockylinux:9.4

RUN dnf install -y epel-release
RUN dnf install -y 'dnf-command(config-manager)' && \
    dnf config-manager --set-enabled crb && \
    dnf groupinstall -y "Development Tools" && \
    dnf install -y cmake ninja-build lld clang perl

RUN useradd --create-home --no-log-init -u 4000 --shell /bin/bash gitlab-ci

WORKDIR /home/gitlab-ci

RUN mkdir -p .ssh && \
    chmod 0700 .ssh && \
    chown -R gitlab-ci:gitlab-ci /home/gitlab-ci && \
    ssh-keyscan gitlab.codasip.com >> .ssh/known_hosts

USER gitlab-ci
