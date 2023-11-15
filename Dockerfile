FROM centos:7
RUN yum makecache
COPY rpm-requirements.txt /root/rpm-requirements.txt
RUN yum install --assumeyes $(cat /root/rpm-requirements.txt)
RUN rm -f /root/rpm-requirements.txt