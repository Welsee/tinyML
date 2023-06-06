
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

int trigPin =9;
int echoPin = 10;

int randNumber = random(3);         //랜덤으로 변수를 만들어서 암호키를 만듬

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

// In order to use optimized tensorflow lite kernels, a signed int8_t quantized
// model is preferred over the legacy unsigned model format. This means that
// throughout this project, input images must be converted from unisgned to
// signed format. The easiest and quickest way to convert from unsigned to
// signed 8-bit integers is to subtract 128 from the unsigned value to get a
// signed value.

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 136 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

void setup() {
 
  Serial.begin(9600);               //pin 설정
  
   pinMode(trigPin, OUTPUT);        //초음파 pin 설정
  pinMode(echoPin, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);               //거리 설정 핀
  pinMode(8, OUTPUT);               //암호키 핀, 비교해서 맞으면 켜짐

  randomSeed(analogRead(0));        //랜덤 변수 설정
  static tflite::MicroErrorReporter micro_error_reporter;
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
  // Get image from provider.
  long duration, distance;
  int now_number;     //현재 가장 높은 가능성의 값을 저장해서 실제 암호키랑 비교

  // 초음파 센서로 거리 측정
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  
  // 거리 계산
  distance = duration * 0.034 / 2;

  
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels,
                            input->data.int8)) {
    TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
  }

  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter->Invoke()) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
  }

  TfLiteTensor* output = interpreter->output(0);

  // Process the inference results.
  int8_t person_score = output->data.uint8[kPersonIndex];            //data.uint8[] 이게 점수를 알려줌 얘가 높으면 사람일 확률이 높음, 낮은면 아닐 확률이 높음
  int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
  for (int i = 0; i < kCategoryCount; i++) {                        //model setting 파일에 선언 여기서는 kCategoryCount = 3
    int8_t curr_category_score = output->data.uint8[i];             //output->data.uint8[0] = 숫자 0, output->data.uint8[1] = 숫자 1,output->data.uint8[2] = 숫자 2, i가 숫자
    const char* currCategory = kCategoryLabels[i];                  //클래스를 포함하는 배열, model settings.cpp파일에 선언됨
    
    TF_LITE_REPORT_ERROR(error_reporter, "%s : %d", currCategory, curr_category_score);
     // 거리 출력
  Serial.print("\t\tDistance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if(distance >= 27 && distance <= 35) {        //초음파 센서 측정 27~35cm이 적정거리, 딱 이정도 거리에서 잘 인식
    
    digitalWrite(6, HIGH);                      //파란불 켜짐
    digitalWrite(5, LOW);
    if(curr_category_score >= 70){            //쉽게 person_score가 70점 넘는다고 생각하면 됨
      now_number = i;                        //i는 지금 점수가 가장 높은 값을 나타냄
      if(randNumber == now_number){               //랜덤 생성 암호키가 지금의 값과 같다면 8번핀 led가 켜지고, 맞다고 나옴
        Serial.print("\t\t\tcorrect. key is ");
        Serial.println(randNumber);
        digitalWrite(8, HIGH);
      }
      else {
        Serial.print("\t\t\twrong. key is ");     //틀리면 8번 핀 불이 켜지지 않음
        Serial.println(randNumber);
        digitalWrite(8, LOW);
      }
    }
   digitalWrite(6, HIGH)
    digitalWrite(5, LOW)
  }
  
  else{
    digitalWrite(6, LOW);                       //적정거리가 안되면 led가 거리를 유지하지 못하는 중이라고 알려줌
    digitalWrite(5, HIGH);
  }
  

  }
//  Serial.write(input->data.int8, bytesPerFrame);
}
