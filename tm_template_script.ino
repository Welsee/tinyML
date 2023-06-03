
//적외선 리모컨 case
//FF30CF FF18E7 FF7A85 : 1, 2, 3
//FF10EF FF38C7 FF5AA5 : 4, 5, 6
//FF42BD FF4AB5 FF52AD : 7, 8, 9

#include <IRremote.h>
#include <TensorFlowLite.h>

#include "main_functions.h"
#include "image_provider.h"
#include "model_settings.h"
#include "person_detect_model_data.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#define REMOTEPIN 8               //적외선 센서가 연결된 디지털 핀 번호 매핑

IRrecv irrecv(REMOTEPIN);
decode_results results;           //수신된 적외선 신호를 저장할 변수


// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;



// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 136 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {

  irrecv.enableIRIn();    //적외선 센서 활성화
                          //2진수로 LED표현할거면 OUTPUT으로 pinMode 설정해야됨

  Serial.begin(9600);

  
  static tflite::MicroErrorReporter micro_error_reporter;                       //tensor 모드 설정임 리모콘은 위쪽 예정
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_person_detect_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }


  // tflite::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<6> micro_op_resolver;
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();
  micro_op_resolver.AddFullyConnected();

  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);
}

void loop() {

  if(irrecv.decode(%results)){          //적외선통신 설정,
    Serial.println(results.value, HEX);

    delay(30);
    irrecv.resume();
  }

  
  // Get image from provider.
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels,
                            input->data.int8)) {
    TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
  }

  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter->Invoke()) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
  }

  TfLiteTensor* output = interpreter->output(0);                       //여기까지가 카메라, tensor 세팅임
                                                                       //카메라 연산이 들어갈 자리. 그럼 위쪽에 리모콘 뭔가가 들어가야 됨                              
  // Process the inference results.
  int8_t person_score = output->data.uint8[kPersonIndex];            //data.uint8[] 이게 점수를 알려줌 얘가 높으면 사람일 확률이 높음, 낮은면 아닐 확률이 높음
  int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
  for (int i = 0; i < kCategoryCount; i++) {                        //model setting 파일에 선언 여기서는 kCategoryCount = 3
    int8_t curr_category_score = output->data.uint8[i];             //output->data.uint8[0] = 숫자 0, output->data.uint8[1] = 숫자 1,output->data.uint8[2] = 숫자 2,
    const char* currCategory = kCategoryLabels[i];                  //클래스를 포함하는 배열, model settings.cpp파일에 선언됨!!!!
    TF_LITE_REPORT_ERROR(error_reporter, "%s : %d", currCategory, curr_category_score);
  }
//  Serial.write(input->data.int8, bytesPerFrame);
}
