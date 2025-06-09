사용 방법



사용자 코드에서 다음과 같이 헤더를 조건부로 포함시킵니다:

#ifdef USE_FPGA_VS
#include "fpga_io_vs.h"
#else
#include "fpga_io.h"
#endif



컴파일 시 USE_FPGA_VS 매크로를 정의하여 VS 모드를 활성화합니다:

gcc -DUSE_FPGA_VS -o fpga_test fpga_test.c



실행 후, 숫자 키패드(1~9) 입력으로 푸시 스위치를 시뮬레이션할 수 있습니다:

키보드 입력 → 푸시스위치 위치
  7 8 9     →   0 1 2
  4 5 6     →   3 4 5
  1 2 3     →   6 7 8



종료는 Ctrl+C로 하며, 터미널 입력 상태는 자동 복원됩니다. 문제 발생 시 stty sane 명령어로 복구 가능합니다.







