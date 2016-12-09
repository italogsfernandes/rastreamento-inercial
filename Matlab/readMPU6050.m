%--------------------------------------------------------------------------
% Federal University of Uberlandia
% Biomedical Engineering Lab
% Author: Andrei Nakagawa, MSc
% contact: nakagawa.andrei@gmail.com
%--------------------------------------------------------------------------
% Description: Step-by-step offline path reconstruction
% Based on Stephen Madgwick's algorithm for gait tracking and the work from
% Wang et al. (2010) for tracking the movements of an instrumented pen
%--------------------------------------------------------------------------
clear;
close all;
clc;
addpath('Quaternions');
addpath('ximu_matlab_library');
%--------------------------------------------------------------------------
%Parameters
g =  9.81; %Standard gravity value
sampfreq = 100;
samplePeriod = 1/sampfreq;
s=load('mpu6050data.txt');
nbits = 16;
accelFactor = (2^nbits) / (4*g);
gyrFactor = (2^nbits) / (250*2);
accX = s(:,1) ./ accelFactor ./ g;
accY = s(:,2) ./ accelFactor ./ g;
accZ = s(:,3) ./ accelFactor ./ g;
gyrX = s(:,5) ./ gyrFactor;  
gyrY = s(:,6) ./ gyrFactor;
gyrZ = s(:,7) ./ gyrFactor; 
time = 1:length(accX);
time = time .* samplePeriod;
%--------------------------------------------------------------------------
%1st step: Pre-processing the acceloremeter data
%High-pass
highCutOff = 0.001;
[b, a] = butter(1, highCutOff/(sampfreq/2), 'high');
fax = filtfilt(b,a,accX);
fay = filtfilt(b,a,accY);
faz = filtfilt(b,a,accZ);
%Low-pass filter
lowCutOff = 5;
[b, a] = butter(1, lowCutOff/(sampfreq/2), 'low');
fax = filtfilt(b,a,fax);
fay = filtfilt(b,a,fay);
faz = filtfilt(b,a,faz);
%--------------------------------------------------------------------------
%thresholds
res = sqrt(fax.^2 + fay.^2 + faz.^2);
res = abs(res);
b0 = 1; b1=50; %baseline
th = mean(res(b0:b1)) + 5*std(res(b0:b1));
stationary = res<th;
figure(); plot(time,res);
hold on;
plot([min(time),max(time)],[th,th],'r');
xlabel('Time (s)');
ylabel('Accelerometer magnitude');
%--------------------------------------------------------------------------
%2nd step: Computing the orientation of the sensor
quat = zeros(length(time), 4); %quaternion vector
%Creates a new object
AHRSalgorithm = AHRS('SamplePeriod', 1/sampfreq, 'Kp', 1, 'KpInit', 1);
% Initial convergence
initPeriod = 2;
indexSel = 1 : find(sign(time-(time(1)+initPeriod))+1, 1);
for i = 1:2000
    AHRSalgorithm.UpdateIMU([0 0 0], [mean(accX(indexSel)) mean(accY(indexSel)) mean(accZ(indexSel))]);
end
% For all data
for t = 1:length(time)
    if(stationary(t))
        AHRSalgorithm.Kp = 0.5;
    else
        AHRSalgorithm.Kp = 0;
    end
    AHRSalgorithm.UpdateIMU(deg2rad([gyrX(t) gyrY(t) gyrZ(t)]), [accX(t) accY(t) accZ(t)]);
    quat(t,:) = AHRSalgorithm.Quaternion;
end
%--------------------------------------------------------------------------
%3rd step: Compute translational accelerations
% Rotate body accelerations to Earth frame
acc = quaternRotate([accX accY accZ], quaternConj(quat));
% % Remove gravity from measurements
acc = acc - [zeros(length(time), 2) ones(length(time), 1)];     % unnecessary due to velocity integral drift compensation
% Convert acceleration measurements to m/s/s
acc = acc * g;
figure();
hold on;
plot(time, acc(:,1), 'r');
plot(time, acc(:,2), 'g');
plot(time, acc(:,3), 'b');
title('Acceleration');
xlabel('Time (s)');
ylabel('Acceleration (m/s/s)');
legend('X', 'Y', 'Z');
hold off;
%--------------------------------------------------------------------------
%4th step: Estimating velocity
% Compute translational velocities
acc(:,3) = acc(:,3) - 9.81;
% Integrate acceleration to yield velocity
vel = zeros(size(acc));
velaux = zeros(size(acc));
for t = 2:length(vel)
    vel(t,:) = vel(t-1,:) + acc(t,:) * samplePeriod;
    velaux(t,:) = vel(t-1,:) + acc(t,:) * samplePeriod;
    if(stationary(t) == 1)
        vel(t,:) = [0 0 0];     % force zero velocity when foot stationary
    end
end
% Compute integral drift during non-stationary periods
velDrift = zeros(size(vel));
stationaryStart = find([0; diff(stationary)] == -1);
stationaryEnd = find([0; diff(stationary)] == 1);
for i = 1:numel(stationaryEnd)
    driftRate = vel(stationaryEnd(i)-1, :) / (stationaryEnd(i) - stationaryStart(i));
    enum = 1:(stationaryEnd(i) - stationaryStart(i));
    drift = [enum'*driftRate(1) enum'*driftRate(2) enum'*driftRate(3)];
    velDrift(stationaryStart(i):stationaryEnd(i)-1, :) = drift;
end
% Remove integral drift
vel = vel - velDrift;
% Plot translational velocity
%'Position', [9 39 900 300], 'Number', 'off', 'Name', 'Velocity'
figure();
hold on;
plot(time, vel(:,1), 'r');
plot(time, vel(:,2), 'g');
plot(time, vel(:,3), 'b');
title('Velocity');
xlabel('Time (s)');
ylabel('Velocity (m/s)');
legend('X', 'Y', 'Z');
hold off;
%--------------------------------------------------------------------------
%5th: Estimating position
% Integrate velocity to yield position
pos = zeros(size(vel));
posaux = zeros(size(vel));
for t = 2:length(pos)
    pos(t,:) = pos(t-1,:) + vel(t,:) * samplePeriod;    % integrate velocity to yield position
    posaux(t,:) = pos(t-1,:) + velaux(t,:) * samplePeriod;    % integrate velocity to yield position
end
% Plot translational position
%'Position', [9 39 900 600], 'Number', 'off', 'Name', 'Position'
figure();
hold on;
plot(time, pos(:,1), 'r');
plot(time, pos(:,2), 'g');
plot(time, pos(:,3), 'b');
title('Position');
xlabel('Time (s)');
ylabel('Position (m)');
%%
figure();
plot(pos(:,1),pos(:,2));
title('Position in XY');
xlabel('X');
ylabel('Y');
%%
figure();
plot(time,fax,'r');
hold on;
plot(time,fay,'g');
plot(time,faz,'b');
title('Raw Acceloremeter data');
%--------------------------------------------------------------------------
figure();
plot(time,gyrX,'r');
hold on;
plot(time,gyrY,'g');
plot(time,gyrZ,'b');
title('Raw Gyroscope data');
%%
SixDofAnimation(pos,quatern2rotMat(quat),'SamplePlotFreq', 15, 'Trail', 'All');
Spin = 120;
SamplePlotFreq = 4;
% SixDofAnimation(pos, quatern2rotMat(quat), ...
%                 'SamplePlotFreq', SamplePlotFreq, 'Trail', 'All', ...
%                 'Position', [9 39 1280 768], 'View', [(100:(Spin/(length(pos)-1)):(100+Spin))', 10*ones(length(pos), 1)], ...
%                 'AxisLength', 0.1, 'ShowArrowHead', false, ...
%                 'Xlabel', 'X (m)', 'Ylabel', 'Y (m)', 'Zlabel', 'Z (m)', 'ShowLegend', false, ...
%                 'CreateAVI', false, 'AVIfileNameEnum', false, 'AVIfps', ((1/samplePeriod) / SamplePlotFreq));
% 
% %--------------------------------------------------------------------------
% R = quatern2rotMat(quat);
% ox = pos(:,1)';
% oy = pos(:,2)';
% oz = pos(:,3)';
% ux = R(1,1,1:end);
% vx = R(2,1,1:end);
% wx = R(3,1,1:end);
% uy = R(1,2,1:end);
% vy = R(2,2,1:end);
% wy = R(3,2,1:end);
% uz = R(1,3,1:end);
% vz = R(2,3,1:end);
% wz = R(3,3,1:end);
% figure();
% %plot3(pos(:,1),pos(:,2),pos(:,3)); 
% quiver3(ox,oy,oz,ux,vx,wx);
% hold on;
% quiver3(ox,oy,oz,uy,vy,vz);
% quiver3(ox,oy,oz,uz,vz,wz);