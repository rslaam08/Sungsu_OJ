# SOJ - SUNGSOO Offline Judge

Windows 11 기준 C 언어 오프라인 알고리즘 저지 시스템입니다.

## 실행 환경

- Windows 11
- GCC 필요
  - 추천: MSYS2 MinGW-w64 또는 MinGW-w64
  - `gcc` 명령이 CMD/PowerShell의 PATH에서 실행되어야 합니다.
- 이 프로젝트는 Windows API `CreateProcessA`, `WaitForSingleObject`를 사용해 제출 프로그램을 실행하고 시간 제한을 검사합니다.

## 빌드/실행

### 가장 쉬운 방법

1. `build.bat` 실행
2. `run.bat` 실행

### CMD에서 직접 실행

```bat
build.bat
run.bat
```

### 직접 GCC 명령으로 빌드

```bat
gcc -Iinclude -std=c11 -Wall -Wextra -O2 src\*.c -o build\soj.exe
build\soj.exe
```

일부 CMD 환경에서 `src\*.c` 와일드카드가 제대로 처리되지 않으면 `build.bat`를 사용하세요.

## 기본 관리자 계정

- ID: `admin`
- PW: `admin123`

## 기본 예시 문제

프로그램 최초 실행 시 `1001 - A+B` 문제가 자동 등록됩니다.

문제 상세 화면에는 다음 세 영역이 표시됩니다.

- 문제 설명: 문제 자체에 대한 설명
- 입력: 입력값의 형식과 의미
- 출력: 출력값의 형식과 의미

테스트케이스 위치:

```text
data/testcases/1001/input1.txt
data/testcases/1001/output1.txt
data/testcases/1001/input2.txt
data/testcases/1001/output2.txt
data/testcases/1001/input3.txt
data/testcases/1001/output3.txt
```

정답 예시 소스:

```text
examples/answer_1001.c
```

실행 후 문제 `1001`을 선택하고 제출 파일 경로에 아래를 입력하면 됩니다.

```text
examples/answer_1001.c
```

## 채점 방식

1. 사용자가 `.c` 소스 파일 경로를 입력합니다.
2. SOJ가 제출 파일을 `workspace/sources/user_{id}/sub_{id}.c`로 복사합니다.
3. `gcc`로 컴파일해 `workspace/executables/sub_{id}.exe`를 만듭니다.
4. 각 테스트케이스 입력 파일을 실행 파일의 표준 입력으로 넣습니다.
5. 실행 결과를 `workspace/outputs/`에 저장합니다.
6. 문제의 정답 출력 파일과 비교합니다.
7. 결과를 `AC`, `WA`, `TLE`, `RE`, `CE`로 판정합니다.

## 새 문제 추가 방법

문제 정보는 `data/problems.dat`에 저장되고, 테스트케이스 파일은 `data/testcases/{problem_id}/`에 저장됩니다.

1. 관리자 계정으로 로그인합니다.
2. 관리자 메뉴에서 문제 등록을 선택합니다.
3. 제목, 문제 설명, 입력 설명, 출력 설명, 카테고리, 난이도, 시간 제한, 테스트케이스 수를 입력합니다.
   - `문제 설명`, `입력`, `출력`은 여러 줄 입력이 가능합니다.
   - 각 항목 입력을 끝내려면 한 줄에 `.`만 입력합니다.
4. 출력된 문제 ID를 확인합니다.
5. `data/testcases/{problem_id}/` 폴더에 다음 형식으로 파일을 넣습니다.

```text
input1.txt
output1.txt
input2.txt
output2.txt
...
```

테스트케이스 수가 3이면 반드시 `input1.txt`~`input3.txt`, `output1.txt`~`output3.txt`가 존재해야 합니다.

문제 등록 예시는 아래와 같습니다.

```text
제목: A+B
문제 설명:
두 int형 정수 a와 b를 입력받아 a+b를 출력하는 프로그램을 작성하세요.
.
입력:
첫째 줄에 두 int형 정수 a와 b가 공백으로 구분되어 주어집니다.
.
출력:
첫째 줄에 a+b 값을 출력합니다.
.
카테고리: 입출력
난이도(1~5): 1
시간 제한(초): 2
테스트케이스 수: 3
```

## Windows 11 구현상 주의점

- 채점 대상 C 코드는 실제 `.exe`로 컴파일되어 실행됩니다.
- 학교 프로젝트용 구현이므로 악성 코드 방지용 샌드박스는 포함되어 있지 않습니다.
- 한글 깨짐을 줄이기 위해 `build.bat`, `run.bat`, 프로그램 시작부에서 `chcp 65001`을 적용합니다.
- 제출 코드가 무한 루프에 빠지면 Windows API로 제한 시간 이후 프로세스를 종료하고 `TLE`로 판정합니다.

## 저장 위치 주의

이 프로젝트는 `data/users.dat`, `data/problems.dat`, `data/submissions.dat`, `data/promotions.dat`에 실행 데이터를 저장합니다.
Windows에서 `build\soj.exe`를 직접 더블클릭하거나 VSCode 작업 디렉터리가 달라지면 저장 위치가 달라질 수 있으므로, 이 버전은 실행 시 작업 디렉터리를 프로젝트 루트로 자동 보정합니다.
`reset_data.bat`을 실행하면 저장 데이터가 삭제됩니다.

## 이번 버전의 변경점

- `Problem` 구조체에 `input_desc`, `output_desc` 필드를 추가했습니다.
- 문제 상세 화면에서 `[문제 설명]`, `[입력]`, `[출력]` 영역을 따로 출력합니다.
- 관리자 문제 등록 화면에서도 `문제 설명`, `입력`, `출력`을 각각 입력할 수 있습니다.
- 기존 `problems.dat`가 이전 구조체 형식이면 가능한 경우 새 구조체로 자동 변환하고, 입력/출력 설명은 기본 문구로 채웁니다.


## 기본 탑재 문제 세트

이 버전에는 1001번부터 1012번까지 총 12문제가 기본 탑재되어 있습니다.

- `data/problems.dat`: 문제 제목, 설명, 입력 설명, 출력 설명, 난이도, 카테고리, 시간 제한, 테스트케이스 수
- `data/testcases/{문제번호}/inputN.txt`: 테스트케이스 입력
- `data/testcases/{문제번호}/outputN.txt`: 테스트케이스 정답 출력
- `examples/answer_문제번호.c`: 예시 정답 코드
- `examples/wrong_문제번호.c`: 예시 오답 코드

문제 세트를 완전히 초기화하려면 `data/users.dat`, `data/submissions.dat`, `data/promotions.dat`를 삭제하고, 문제를 바꾸려면 `data/problems.dat`와 `data/testcases/`를 함께 교체하세요.
