FROM ubuntu:20.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
      cmake \
      git \
      build-essential \
      automake \
      pkg-config \
      libsctp-dev \
      libsctp1 \
      libboost-program-options1.67-dev \
      libboost-program-options1.67.0

RUN git clone https://github.com/dvidelabs/flatcc.git && \
    cd flatcc && \
    ./scripts/initbuild.sh make-concurrent && \
    ./scripts/build.sh

RUN apt-get install -y software-properties-common && \
    add-apt-repository -y ppa:pistache+team/unstable && \
    apt update && \
    apt install -y libpistache-dev

COPY . flexric
WORKDIR flexric
#RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DREST_NORTHBOUND=ON -DE2AP_ENCODING=FLATBUFFERS -DFlatCC_HINT_INCLUDE_DIR=/flatcc/include -DFlatCC_LIBRARY_DIR=/flatcc/lib/ && make -j8
RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DREST_NORTHBOUND=ON -DE2AP_ENCODING=ASN -DFlatCC_HINT_INCLUDE_DIR=/flatcc/include -DFlatCC_LIBRARY_DIR=/flatcc/lib/ && make -j8

RUN find / -name 'libboost_program_options.so.*'

FROM ubuntu:20.04 AS flexran_rtc
COPY --from=builder \
  /usr/lib/x86_64-linux-gnu/libboost_program_options.so* \
  /usr/lib/x86_64-linux-gnu/libsctp.so* \
	/usr/lib/x86_64-linux-gnu/libpistache.so* \
	/usr/lib/x86_64-linux-gnu/libpthread.so* \
	/usr/lib/x86_64-linux-gnu/libstdc++.so* \
	/usr/lib/x86_64-linux-gnu/libgcc_s.so* \
	/usr/lib/x86_64-linux-gnu/libc.so* \
	/usr/lib/x86_64-linux-gnu/libssl.so* \
	/usr/lib/x86_64-linux-gnu/libcrypto.so* \
	/usr/lib/x86_64-linux-gnu/libm.so* \
	/usr/lib/x86_64-linux-gnu/libdl.so* \
  /usr/lib/x86_64-linux-gnu/
COPY --from=builder flexric/build/src/controller/flexran/flexran_rtc /bin

EXPOSE 9999/sctp
EXPOSE 36412/sctp
#CMD flexran_rtc --saddress 0.0.0.0 --naddress 0.0.0.0
ENTRYPOINT ["flexran_rtc"]
