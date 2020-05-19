#include "mbed.h"
#include <cmath>
#include "DA7212.h"

#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "uLCD_4DGL.h"

DA7212 audio;
int16_t waveform[kAudioTxBufferSize];
Serial pc(USBTX, USBRX);

#define bufferLength (32)
#define signalLength (49)

uLCD_4DGL uLCD(D1, D0, D2);
InterruptIn menu(SW2);
InterruptIn check(SW3);
DigitalOut greenLED(LED2);

void forward();
void backward();
void mode_sel();
void song_sel();
int PredictGesture(float* output);
void playNote(int freq);
void loadSignal();
void playing();
void playing1();
int taiko();

int mode = 1;
int song = 0;
int start = 0;
bool menu_selection = false;
bool mode_selection = false;

char serialInBuffer[bufferLength];

int serialCount = 0;
// frequency of song note
int song_note[3][49];
char song_name[3][30] = {"Twinkle Twinkle", "The bee song", "Happy Birthday song"};;
int noteLength[3][49];
int beat[49];
int score = 0;
int game = false;
int t = false;
Timer t1;
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
static tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
static tflite::MicroOpResolver<6> micro_op_resolver;
// Build an interpreter to run the model with
static tflite::MicroInterpreter static_interpreter(
    model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
tflite::MicroInterpreter *interpreter = &static_interpreter;
TfLiteTensor* model_input;
int input_length;

// Whether we should clear the buffer next time we fetch data
bool should_clear_buffer = false;
bool got_data = false;

// The gesture index of the prediction
int gesture_index;
void playNote(int freq)
{
  for(int i = 0; i < kAudioTxBufferSize; i++) {
    waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
  }
  // the loop below will play the note for the duration of 1s
  for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
    audio.spk.play(waveform, kAudioTxBufferSize);
  }
}
void playing1(){
 // while(true) {
     int n;
    uLCD.cls();
    uLCD.printf("\n Now playing\n");
    uLCD.printf("\n Twinkle Twinkle\n");
    for(n = 0; n < 49; n++) {
      uLCD.printf("\n Frequency #%d\n", song_note[0][n]);
      int length = noteLength[0][n];
      while (length--){
        for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
          playNote(song_note[0][n]);
        }
      }
      if (mode_selection) break;
    }
    //wait_us(1000000);
    if (!mode_selection){
        uLCD.cls();
        uLCD.printf("\n Now playing\n");
        uLCD.printf("\n The bee song\n");
        for(n = 0; n < 49; n++) {
          uLCD.printf("\n Frequency #%d\n", song_note[1][n]);
          int length = noteLength[1][n];
          while (length--){
            for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
              playNote(song_note[1][n]);
            }
          }
          if (mode_selection) break;
        }
    }
    if (!mode_selection){
        wait_us(1000000);
        uLCD.cls();
        uLCD.printf("\n Now playing\n");
        uLCD.printf("\n Happy Birthday song\n");
        for(n = 0; n < 49; n++) {
          uLCD.printf("\n Frequency #%d\n", song_note[2][n]);
          int length = noteLength[2][n];
          while (length--){
            for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
              playNote(song_note[2][n]);
            }
          }
            if (mode_selection) break;
        }
    }
}
void playing(){
  int n;
  uLCD.cls();
  if(!game){
    uLCD.printf("\n Now playing\n");
    uLCD.printf("\n %s\n", song_name[song]);
  }
  for(n = 0; n < 49; n++) {
      int length = noteLength[song][n];
      while (length--){
        for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
          playNote(song_note[song][n]);
        }
      }   
      
      if (mode_selection){
          game = false;
          break;
      }
    

  } 
}

int main(int argc, char* argv[]) {
  greenLED = 1;
  uLCD.printf("\n Mode 0 : \n");  
  menu.rise(&mode_sel);
  check.rise(&song_sel);
  uLCD.printf("\n Now Playing Music\n");
  loadSignal();
  //greenLED = 0;
  
  
  //uLCD.printf("song1 : Twinkle Twinkle\n");
  //uLCD.printf("song2 : The bee song\n");
  
  
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return -1;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
static tflite::MicroOpResolver<6> micro_op_resolver;
  micro_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                               tflite::ops::micro::Register_MAX_POOL_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                               tflite::ops::micro::Register_RESHAPE(),1);                             

  // Build an interpreter to run the model with
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  tflite::MicroInterpreter* interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  interpreter->AllocateTensors();

  // Obtain pointer to the model's input tensor
  TfLiteTensor* model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != config.seq_length) ||
      (model_input->dims->data[2] != kChannelNumber) ||
      (model_input->type != kTfLiteFloat32)) {
    error_reporter->Report("Bad input tensor parameters in model");
    return -1;
  }

  int input_length = model_input->bytes / sizeof(float);

  TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
  if (setup_status != kTfLiteOk) {
    error_reporter->Report("Set up failed\n");
    return -1;
  }

  error_reporter->Report("Set up successful...\n");

  

  while (true) {
    //if (!start) playing();
    if (mode_selection){
        playNote(0);
        if (mode == -1){
          uLCD.cls();
          uLCD.printf("\n Select mode...\n");
          uLCD.printf("\n 1.Forward \n");
          uLCD.printf("\n 2.Backward \n");
          uLCD.printf("\n 3.Change songs\n");
          uLCD.printf("\n 4.Taiko");
          mode = 0;
        }
    
        // Attempt to read new data from the accelerometer
        got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                    input_length, should_clear_buffer);

        // If there was no new data,
        // don't try to clear the buffer again and wait until next time
        if (!got_data) {
          should_clear_buffer = false;
          continue;
        }

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
          error_reporter->Report("Invoke failed on index: %d\n", begin_index);
          continue;
        }

        // Analyze the results to obtain a prediction
        gesture_index = PredictGesture(interpreter->output(0)->data.f);

        // Clear the buffer next time we read data
        should_clear_buffer = gesture_index < label_num;

        if (gesture_index == 1) {
          // error_reporter->Report(config.output_message[gesture_index]);
          if (mode < 4) mode ++;
          else mode = 1;

          if (mode == 1) {
            uLCD.cls();
            uLCD.printf("\n 1.Forward\n");
          } 
          else if (mode == 2) {
            uLCD.cls();
            uLCD.printf("\n 2.Backward\n");
          }
          else if (mode == 3){
            uLCD.cls();
            uLCD.printf("\n 3.Change song\n");
          } 
          else{
            uLCD.cls();
            uLCD.printf("\n 4.Taiko\n");
          }
        } 
    }
    else if (mode == 33){
      playNote(0);
      uLCD.cls();
      uLCD.printf("\nSELECT Twinkle Twinkle\n");
      uLCD.printf("\nSELECT The bee song\n");
      uLCD.printf("\nSELECT Happy Birthday song\n");
      while (mode == 33){
          
          got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                    input_length, should_clear_buffer);

          // If there was no new data,
          // don't try to clear the buffer again and wait until next time
          if (!got_data) {
            should_clear_buffer = false;
            continue;
          }

          // Run inference, and report any error
          TfLiteStatus invoke_status = interpreter->Invoke();
          if (invoke_status != kTfLiteOk) {
            error_reporter->Report("Invoke failed on index: %d\n", begin_index);
            continue;
          }

          // Analyze the results to obtain a prediction
          gesture_index = PredictGesture(interpreter->output(0)->data.f);

          // Clear the buffer next time we read data
          should_clear_buffer = gesture_index < label_num;
          //uLCD.printf("\n value : %d\n",gesture_index);
          if (gesture_index == 1){
            if (song < 2) song ++;
            else song = 0;

            uLCD.cls();
            if (song == 0){
                uLCD.printf("\n No.1\n");
            }
              
            else if (song == 1)
              uLCD.printf("\n No.2\n");
            else if (song == 2)
              uLCD.printf("\n No.3\n");
          }
          //uLCD.printf("\n gesture_index : %d\n",gesture_index);
          //wait_us(5000000);
          
      }
    }
    else if (mode == 4){
      if (t){
        int score = 0;
        game = true;
        for (int i = 0; i < 49; i ++){
            beat[i]= 0;
        }
        t = false;
        
        for (int n = 0 ; n <49; n++){
                        
                //int length = noteLength[song][n];

                //while (length--){
                  //for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
                
          playNote(song_note[song][n]);
          uLCD.cls();
          uLCD.printf("\n BEAT : %d\n", noteLength[song][n]);      
          t1.start();
          wait_us(500000);
          t1.reset();
          t1.start();
                
          while(t1.read()<0.73){
            
                got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                                input_length, should_clear_buffer);                     
                if (!got_data) {
                  should_clear_buffer = false;
                  continue;
                }

                // Run inference, and report any error
                TfLiteStatus invoke_status = interpreter->Invoke();
                if (invoke_status != kTfLiteOk) {
                  error_reporter->Report("Invoke failed on index: %d\n", begin_index);
                  continue;
                }

                // Analyze the results to obtain a prediction
                gesture_index = PredictGesture(interpreter->output(0)->data.f);

                // Clear the buffer next time we read data
                should_clear_buffer = gesture_index < label_num;
                //uLCD.printf("\n value: %d\n", gesture_index);
                if (gesture_index < label_num){
                    beat[n] = 1;
                } 
                  
                if (beat[n] == noteLength[song][n])
                  score ++ ;
                
              //}
          }
          uLCD.printf("my beat : %d\n", beat[n]);
          t1.reset(); 
          if (mode_selection)
              break;      
        }
        wait_us(500000);
        uLCD.printf("\n Total score: %d\n",score/15);
       }
    }
    else{
      if (start){
          if (mode != -1){
            if (mode == 1){
              uLCD.cls();
              uLCD.printf("\n Forward~~\n");
              forward();
              wait_us(3000000);
            }else if (mode == 2){
                uLCD.cls();
                uLCD.printf("\n Backward~~\n");
                backward();
                wait_us(3000000);
            }
          }
      }
      if (!start) playing1();
      else if (!t)playing();
      mode = -1;
    }
  }
}

void mode_sel(){
  t = false;
  start = 1;
  mode_selection = true;
  mode = -1;
}
void forward(){
  if (song == 0) song = 1;
  else if (song == 1) song = 2;
  else if (song == 2) song = 0;
}
void backward(){
  if (song == 0) song = 2;
  else if (song == 1) song = 0;
  else if (song == 2) song = 1;
}
void song_sel(){
  mode_selection = false;
  if (mode == 3){
    t = false;
    mode = 33;
  } 
  else if (mode == 4){
  //   //mode = 33;
     t = true;
  }
  else if (mode == 33) {
    t = false;
    mode = -33;
    //uLCD.printf("\n song number : %d\n", song);
  }
}



void loadSignal()
{
  greenLED = 0;
  int i = 0;
  int j = 0;
  serialCount = 0;
  audio.spk.pause();
  for (j = 0; j < 3; j++){
      i = 0;
      while(i < signalLength)
      {
        if(pc.readable())
        {
          serialInBuffer[serialCount] = pc.getc();
          serialCount++;
          if(serialCount == 5)
          {
            serialInBuffer[serialCount] = '\0';
            song_note[j][i] = 1000 * (float) atof(serialInBuffer);
            serialCount = 0;
            i++;
            //printf("%d", i);
          }
        }
      }
    
  }
  for (j = 0; j < 3; j++) {
    i = 0;
    while(i < signalLength)
    {
      if(pc.readable()) {
        serialInBuffer[serialCount] = pc.getc();
        serialCount++;
        if(serialCount == 5) {
          serialInBuffer[serialCount] = '\0';
          noteLength[j][i] = 1000 * (float)atof(serialInBuffer);
          serialCount = 0;
          i++;
        }
      }
    }
  }
  greenLED = 1;    
}

// Return the result of the last prediction
int PredictGesture(float* output) {
  // How many times the most recent gesture has been matched in a row
  static int continuous_count = 0;
  // The result of the last prediction
  static int last_predict = -1;

  // Find whichever output has a probability > 0.8 (they sum to 1)
  int this_predict = -1;
  for (int i = 0; i < label_num; i++) {
    if (output[i] > 0.8) this_predict = i;
  }

  // No gesture was detected above the threshold
  if (this_predict == -1) {
    continuous_count = 0;
    last_predict = label_num;
    return label_num;
  }

  if (last_predict == this_predict) {
    continuous_count += 1;
  } else {
    continuous_count = 0;
  }
  last_predict = this_predict;

  // If we haven't yet had enough consecutive matches for this gesture,
  // report a negative result
  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
    return label_num;
  }
  // Otherwise, we've seen a positive result, so clear all our variables
  // and report it
  continuous_count = 0;
  last_predict = -1;

  return this_predict;

}
