FROM ibct/ledgis:2.0.7
LABEL maintiner="Chiyoung Jeong <jcy@ibct.or.kr>"

RUN mkdir -p /etc/nodeos
RUN mkdir /data

ADD ./config.ini /etc/nodeos/config.ini
ADD ./genesis.json /etc/nodeos/genesis.json

EXPOSE 8888
EXPOSE 9876
EXPOSE 8080
