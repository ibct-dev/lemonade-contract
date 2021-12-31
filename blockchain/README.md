# 로컬에 싱글 블록체인 노드 실행을 위한 스크립트

> **WARNING**: docker hub 로그인

> **WARNING**: 컨트랙트가 실행되지 않을 경우 docker의 ram과 cpu를 늘려야함
## 필수 라이브러리
 - Docker ^20.10.0

## 폴더 및 파일 설명
- contracts - 초기 시스템 컨트랙트들이 들어있는 폴더
- scripts - 노드 실행을 위한 스크립트 폴더
- config.ini - 블록체인 설정을 위한 설정 파일
- genesis.json - 초기 블록체인의 설정을 위한 설정 파일
- Dockerfile - 도커 실행을 위한 파일

## 사용법
1. config.ini 수정
2. scripts/constants.sh 수정
3. 아래 커맨드라인 실행

```bash
cd scripts
./build.sh
./run.sh
./boot.sh
```