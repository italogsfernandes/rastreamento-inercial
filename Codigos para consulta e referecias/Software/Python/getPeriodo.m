function [ periodos ] = getPeriodo( horarios )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
       periodos = seconds(horarios(2:length(horarios))-horarios(1:length(horarios)-1));
       %periodo = mean(periodos);
end

