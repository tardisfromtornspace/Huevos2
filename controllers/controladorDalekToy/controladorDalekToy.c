/*
 * File:          controladorRObot4rueda.c
 * Date:
 * Description:
 * Author:
 * Modifications:
 */

/*
 * You may need to add include files like <webots/distance_sensor.h> or
 * <webots/motor.h>, etc.
 */
#include <webots/robot.h>

#include <stdio.h>

#include <webots/led.h>
#include <webots/motor.h>
#include <webots/emitter.h> 
// TO-DO Arreglar el de arriba, no se usa pero era el del globo?

#include <webots/gps.h>
#include <webots/distance_sensor.h>
#include <webots/compass.h>
#include <webots/camera.h>
#include <webots/camera_recognition_object.h>

#include <webots/keyboard.h>

//#include "js.h"
#define KEY_INCREMENT 0.05

/*
 * You may want to add macros here.
 */
#define TIME_STEP 64

/*
 * This is the main program.
 * The arguments of the main function can be specified by the
 * "controllerArgs" field of the Robot node
 */
int main(int argc, char **argv) {
  /* necessary to initialize webots stuff */
  double command[5] = {0.0, 0.0, 0.0, 0.0, 0.0}; // X, Y, Z, Z2, Rotacion X. 2.1 es lo suficiente para volar
  // 5.0 ascenso suave 4.9 descenso suave.
  wb_robot_init();
  // ENU es East-North-Up, el segundo valor es el Norte
  WbDeviceTag compas = wb_robot_get_device("compass");
  wb_compass_enable(compas, TIME_STEP);
  
  const double * coordIniciales = wb_compass_get_values(compas); // Valores iniciales de orientación, nos da 3 float
  printf("Compas: %lf, %lf, %lf\n", coordIniciales[0], coordIniciales[1], coordIniciales[2]);

  double compasInicialVerdad[3];
  int i, j;
  
  bool avoid_obstacle_counter = 0;
  bool avoid_obstacle_counter2 = 0;
  int onTheAir = 0;
  bool firstTime = 1;
  int alturaMaximaPermitida = 800; // TO-DO ajustar al plano adecuado
  int maxTiempoEnElAire = 100;
  int counterBrujulear = 0;
  int sentidoGiro = 1;
  
  WbDeviceTag ds[5];
  char ds_names[5][10] = {"ds_left", "ds_right", "ds_ple", "ds_pri", "ds_sue"};
  
  for (i = 0; i < 5; i++) {
    ds[i] = wb_robot_get_device(ds_names[i]);
    wb_distance_sensor_enable(ds[i], TIME_STEP);
  }
  
  WbDeviceTag camera;
  
  camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, TIME_STEP);
  wb_camera_recognition_enable(camera, TIME_STEP);
  
  WbDeviceTag gps;
  gps = wb_robot_get_device("gps");
  wb_gps_enable(gps, TIME_STEP);
  const double * coordGPS = wb_gps_get_values(gps); // = wb_gps_get_speed_vector(gps);
  
  WbDeviceTag wheels[2];
  char wheels_names[2][8] = {"orugaD", "orugaI"};
  for (i = 0; i < 2; i++) {
    wheels[i] = wb_robot_get_device(wheels_names[i]);
    wb_motor_set_position(wheels[i], INFINITY);
  }
  
  WbDeviceTag lights[4];
  char light_names[4][8] = {"led1", "led2", "led3", "engiGlo"};
  for (i = 0; i < 4; i++) {
    lights[i] = wb_robot_get_device(light_names[i]);
    wb_led_set(lights[i], 0);
  }
  wb_led_set(lights[2], 1);
  
  // TO-DO Cambiar a control de robot
  printf("Using keyboard control (click in the 3D windows before pressing keys).\n");
  printf("Available control keys: up, down, right, left, page up, page down and space (reset).\n");
  wb_keyboard_enable(TIME_STEP);
  
  // Handle emitter
  WbDeviceTag gEmitter[5];
  char engines_names[5][9] = {"emitterX", "emitterY", "emitterZ", "emiterZ2", "emiterR"};
  for (i = 0; i < 5; i++) {
    gEmitter[i] = wb_robot_get_device(engines_names[i]);
    wb_motor_set_position(gEmitter[i], INFINITY);
  }
  
//TO-DO BORRA
    wb_motor_set_velocity(wheels[0], 0.0);
    wb_motor_set_velocity(wheels[1], 0.0);
    wb_motor_set_velocity(gEmitter[0], command[0]);
    wb_motor_set_velocity(gEmitter[1], command[1]);
    wb_motor_set_velocity(gEmitter[2], command[2]);
    wb_motor_set_velocity(gEmitter[3], command[3]);
    wb_motor_set_velocity(gEmitter[4], command[4]);
  
  while (wb_robot_step(TIME_STEP) != -1) {
    if (firstTime) {
      firstTime = 0;
      compasInicialVerdad[0] = coordIniciales[0]; // East. East 0 y North 1 es de frente
      compasInicialVerdad[1] = coordIniciales[1]; // North East 1 y North 0 es para la izq, East -1 y North 0 es para la dcha
      compasInicialVerdad[2] = coordIniciales[2]; // Up
      printf("Compas Inicial: %lf, %lf, %lf\n", compasInicialVerdad[0], compasInicialVerdad[1], compasInicialVerdad[2]);
    }
    printf("Compas: %lf, %lf, %lf\n", coordIniciales[0], coordIniciales[1], coordIniciales[2]);
    printf("GPS: %lf, %lf, %lf\n", coordGPS[0], coordGPS[1], coordGPS[2]);
    // Siento 
    int number_of_objects = wb_camera_recognition_get_number_of_objects(camera);
    printf("\nReconocidos %d objetos.\n", number_of_objects);
    
    const WbCameraRecognitionObject *objects = wb_camera_recognition_get_objects(camera);
    /*for (i = 0; i < number_of_objects; ++i) {
      printf("Modelo de objeto %d: %s\n", i, objects[i].model);
      printf("Id del objeto %d: %d\n", i, objects[i].id);
      printf("Posicion relativa del objeto %d: %lf %lf %lf\n", i, objects[i].position[0], objects[i].position[1],
             objects[i].position[2]);
      printf("Orientacion relativa del objeto %d: %lf %lf %lf %lf\n", i, objects[i].orientation[0], objects[i].orientation[1],
             objects[i].orientation[2], objects[i].orientation[3]);
      printf("Espacio del objeto %d: %lf %lf\n", i, objects[i].size[0], objects[i].size[1]);
      printf("Posicion del objeto t %d en la imagen de la camara: %d %d\n", i, objects[i].position_on_image[0],
             objects[i].position_on_image[1]);
      printf("Espacio del objeto %d en la imagen de la camara %d %d\n", i, objects[i].size_on_image[0], objects[i].size_on_image[1]);
      for (j = 0; j < objects[i].number_of_colors; ++j)
        printf("- Color %d/%d: %lf %lf %lf\n", j + 1, objects[i].number_of_colors, objects[i].colors[3 * j],
               objects[i].colors[3 * j + 1], objects[i].colors[3 * j + 2]);
    }*/
    // Lógica interna
    
    // Actúo
    double left_speed = 0.4;
    double right_speed = 0.4;
    
    double ds_values[5];
    for (i = 0; i < 5; i++) {
      ds_values[i] = wb_distance_sensor_get_value(ds[i]);
      printf("Control ds %d: %lf", i, ds_values[i]);
    }
    if (avoid_obstacle_counter > 0) {
      avoid_obstacle_counter--;
      if (sentidoGiro == 1){
        left_speed = 0.005;
        right_speed = -0.005;
      }else if (sentidoGiro == 2){
        left_speed = -0.005;
        right_speed = 0.005;
      } else if (sentidoGiro == 3){
        printf("Despegue");
        left_speed = 0.0;
        right_speed = 0.0;
        command[0] = 0.6;
        if (ds_values[4] > alturaMaximaPermitida){ // TO-DO quizá aniadir un sensor de distancia a la cabeza?
          command[2] = 4.9363; // TO-DO avisar al profe que usamos una lookup table mejor HECHO
          command[3] = 4.9363;
        } else {
          command[2] = 4.96; //5.0;
          command[3] = 4.96; //5.0;
        }
        wb_led_set(lights[3], 1);
      } 

      
      wb_led_set(lights[0], 1);
      wb_led_set(lights[1], 1);
      
    } else { // read sensors
      if (onTheAir > 0) {
      printf("ON THE AIR");
      left_speed = 0.0;
      right_speed = 0.0;
      command[0] = 1.8;//0.3; // TO-DO ajustar
        if (onTheAir == 1) {
          command[2] = 4.9; //4.9; TO-DO usar 4.9563 para descenso y 4.9564 para ligero ascenso?
          command[3] = 4.9; //4.9; 4.95 logra un muy lento descenso
          counterBrujulear = 10;
        } else { // Control altitud
          if (ds_values[4] > alturaMaximaPermitida){ 
            command[2] = 4.9363; 
            command[3] = 4.9363;
          } else {
            command[2] = 4.96; //6.9;
            command[3] = 4.96; //6.9;
          }

        if (ds_values[0] < 970.0 || ds_values[1] < 970.0 || avoid_obstacle_counter2 > 0){ // TO-DO aniadir estabilidad, fuzzy logic?
            if (ds_values[0] < 970.0) command[4] = 0.0101; // TO-DO modificar
            else if (ds_values[1] < 970.0) command[4] = -0.0101;
        } else {
              if (coordIniciales[1] < compasInicialVerdad[1] && coordIniciales[0] > compasInicialVerdad[0]){
                printf("Giro izda");
                command[4] = 0.0051;
              }else if (coordIniciales[1] < compasInicialVerdad[1] && coordIniciales[0] < compasInicialVerdad[0]){
                printf("Giro dcha");
                command[4] = -0.0051;
              }else {
                printf("No giro, voy de frente");
                command[4] = 0.0;
              }
              // TO-DO Fuzzy logic?
        
        }
        
        }
        onTheAir--;
      } // else command[0] = 0.0; // TO-DO Detener o manejar el giro con PropellerR?
      if (ds_values[4] < 480) {
          if (onTheAir == 0) {
          command[0] = 0.0; //6.9;
          command[2] = 0.0; //6.9;
          command[3] = 0.0; //6.9;
          } else {
            //command[0] = -1.0;
            if (onTheAir > 15) onTheAir = 15;
          }
          wb_led_set(lights[3], 0);
      } //else if (onTheAir == 0) command[0] = 0.06;
      
      wb_led_set(lights[0], 0);
      wb_led_set(lights[1], 0);
      if ((coordGPS[1] < -0.4 || coordGPS[1] > 1.6) && counterBrujulear <= 0) {
        left_speed *= 0.01;
        right_speed *= 0.01;
      }
      if (ds_values[0] < 970.0 || ds_values[1] < 970.0){ // TO-DO aniadir estabilidad, fuzzy logic?
        printf("Detecto obstaculo, a evitar ahora");
        if (ds_values[0] < 970.0 && ds_values[1] < 970.0 && !(coordGPS[1] < -0.4 || coordGPS[1] > 1.6)){
          printf("A VOLAR");
          left_speed = 0.0;
          right_speed = 0.0;
          command[0] = 0.4;
          command[2] = 5.2901;
          command[3] = 5.2901;
          sentidoGiro = 3;
          onTheAir = maxTiempoEnElAire;
        } else if (ds_values[0] < 970.0 && ds_values[1] > 970.0) sentidoGiro = 1;
        else if (ds_values[0] > 970.0 && ds_values[1] < 970.0) sentidoGiro = 2;
        else sentidoGiro = 1;
        
        avoid_obstacle_counter = 8;
        counterBrujulear = 10;
      } else {
        if (counterBrujulear <= 0) {
              if (!(coordGPS[1] < -0.4 || coordGPS[1] > 1.6)) counterBrujulear = 1;
              if (coordIniciales[1] < compasInicialVerdad[1] && coordIniciales[0] > compasInicialVerdad[0]){
                if (coordGPS[1] < -0.4 || coordGPS[1] > 1.6) {
                  left_speed *= -0.04;
                  right_speed *= 0.04;
                } else {
                  left_speed *= (0.8 * (1 - 0.1 * coordIniciales[0]));
                  right_speed *= (0.8 * (1 + 0.1 * coordIniciales[0]));
                }

              }else if (coordIniciales[1] < compasInicialVerdad[1] && coordIniciales[0] < compasInicialVerdad[0]){
                 if (coordGPS[1] < -0.4 || coordGPS[1] > 1.6) {
                   left_speed *= 0.04;
                   right_speed *= -0.04;
                 } else {
                   left_speed *= (0.8 * (1 - 0.1 * coordIniciales[0])); //0.8
                   right_speed *= (0.8 * (1 + 0.1 * coordIniciales[0]));  
                 }
              }
       } else counterBrujulear--;       
      }
    }
    if (ds_values[2] < 97.0 || coordIniciales[2] > 0.5) { // TO-DO arreglo con fuzzy logic
        command[1] = -0.01; // Laterales
      } else if (ds_values[3] < 97.0 || coordIniciales[2] < -0.5) {
        command[1] = 0.01;
      } else {
        if (onTheAir > 0) {
            if (coordIniciales[1] < compasInicialVerdad[1] && coordIniciales[0] > compasInicialVerdad[0]){
                command[1] = 0.151;
              }else if (coordIniciales[1] < compasInicialVerdad[1] && coordIniciales[0] < compasInicialVerdad[0]){
                
                command[1] = -0.151;
              }else {
                
                command[4] = 0.0;
              }
        } else command[1] = 0.0;
      }
      wb_motor_set_velocity(wheels[0], left_speed);
      wb_motor_set_velocity(wheels[1], right_speed);

      if (command[0] || command[1] || command[2] || command[3])
        printf(" command = ( %g , %g , %g , %g, %g)", command[0], command[1], command[2], command[3], command[4]);
      printf(" tracks ( left = %g , right = %g)\n", left_speed, right_speed);
      //wb_emitter_send(gEmitter, command, sizeof(command));
      wb_motor_set_velocity(gEmitter[0], command[0]);
      //wb_motor_set_force(gEmitter[0], command[0]);
      wb_motor_set_velocity(gEmitter[1], command[1]);
      wb_motor_set_velocity(gEmitter[2], command[2]);
      wb_motor_set_velocity(gEmitter[3], command[3]);
      wb_motor_set_velocity(gEmitter[4], command[4]);
  }
  wb_robot_cleanup();
  return 0;  // EXIT_SUCCESS
}
