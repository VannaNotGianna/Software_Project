FROM ubuntu:noble

ARG uid
ARG gid
ARG user
ARG group

RUN groupadd -g ${gid} ${group} || true
RUN useradd -l -u ${uid} -g ${group} -m ${user} || true

RUN apt-get update 
RUN apt-get install -y python3 python3-fastapi python3-requests python3-sqlalchemy python3-uvicorn pipx

USER ${uid}
ENV PATH /home/ubuntu/.local/bin:$PATH
RUN pipx install fastapi-cli --system-site-packages
    