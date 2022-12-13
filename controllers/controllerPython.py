import numpy as np
import skfuzzy as fuzz
from skfuzzy import control as ctrl

#import matplotlib.pyplot as plt

from controller import Motor

TIME_STEP = 64

robot = Robot()

ds = []
dsNames = ["horizontal position sensor"]
for i in range(2):
    ds.append(robot.getDevice(dsNames[i]))
    ds[i].enable(TIME_STEP)
    
wheels = []
wheelsNames = ["horizontal_motor"]
for i in range(4):
    wheels.append(robot.getDevice(wheelsNames[i]))
    wheels[i].setPosition(float('inf'))
    wheels[i].setVelocity(0.0)
    
avoidObstacleCounter = 0

# Antecedente
ángulo = ctrl.Antecedent(np.arange(-np.pi, np.pi, 0.01), 'ángulo')
# Consecuente
motor = ctrl.Consequent(np.arange(-1, 1.1, 0.01), 'motor')

#plt.style.use('ggplot')
#plt.rcParams.update({'figure.figsize': (16, 9),'figure.dpi': 100})

# ángulo natural

ángulo['dismal'] = fuzz.trimf(ángulo.universe, [-3.1416, -3.1415, -2.1]) 
ángulo['poor'] = fuzz.trimf(ángulo.universe, [-3.1416, -2.36, -1.57]) 
ángulo['mediocre'] = fuzz.trimf(ángulo.universe, [-2.36, -1.57, 0.0]) 
ángulo['average'] = fuzz.trimf(ángulo.universe, [-1.57, 0.0, 1.57]) 
ángulo['decent'] = fuzz.trimf(ángulo.universe, [0.0, 1.57, 2.36]) 
ángulo['good'] = fuzz.trimf(ángulo.universe, [1.57, 2.36, 3.1415]) 
ángulo['excellent'] = fuzz.trimf(ángulo.universe, [2.36, 3.1415, 3.1415])

#motor natural
motor['dismal'] = fuzz.trimf(motor.universe, [-1.0, -1.0, -0.75]) 
motor['poor'] = fuzz.trimf(motor.universe, [-1.0, -0.75, -0.5]) 
motor['mediocre'] = fuzz.trimf(motor.universe, [-0.75, -0.3, 0.0]) 
motor['average'] = fuzz.trimf(motor.universe, [-0.3, 0.0, 0.3]) 
motor['decent'] = fuzz.trimf(motor.universe, [0.25, 0.3, 0.75]) 
motor['good'] = fuzz.trimf(motor.universe, [0.5, 0.75, 1.0]) 
motor['excellent'] = fuzz.trimf(motor.universe, [0.75, 1.0, 1.0]) 

rulebase = [
    ctrl.Rule(ángulo['dismal'], motor['dismal']),
    ctrl.Rule(ángulo['poor'], motor['poor']),
    ctrl.Rule(ángulo['mediocre'], motor['mediocre']),
    ctrl.Rule(ángulo['average'], motor['average']),
    ctrl.Rule(ángulo['decent'], motor['decent']),
    ctrl.Rule(ángulo['good'], motor['good']),
    ctrl.Rule(ángulo['excellent'], motor['excellent']),
]

péndulo_ctrl = ctrl.ControlSystem(rulebase)
sim = ctrl.ControlSystemSimulation(péndulo_ctrl)

# Funciones de pertenencia de ángulo
ángulo.automf(7)  # dismal, poor, mediocre, average, decent, good, excellent
# Funciones de pertenencia de motor
motor.automf(7)  # Las mismas

# Forma explicita TODO
# ángulo['mn'] = fuzz.trimf(tip.universe, [-3.14, -2.36, -1.57])

while robot.step(TIME_STEP) != -1:
    Speed = 1.0
    rightSpeed = 1.0
    
    #ángulo.view();
    #motor.view();
    
    i = 0
    
    sim.input['ángulo'] = ds[i].getValue() #-np.pi/4
    sim.compute()
    print(sim.output['motor'])
    #motor.view(sim=sim)
    

    wheels[0].setVelocity(leftSpeed)








