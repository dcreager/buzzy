FROM base/archlinux

RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm --needed \
    base-devel \
    cmake \
    git \
    python \
    sudo \
 && paccache -r -k 0

COPY makepkg.conf /etc/makepkg.conf
COPY sudoers /etc/sudoers
RUN chmod 0660 /etc/sudoers
RUN useradd -d /home/buzzy -m -G wheel buzzy
USER buzzy
ENV HOME /home/buzzy
CMD ["/bin/bash"]
