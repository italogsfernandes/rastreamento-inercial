horarios = times_sem_fio_teste_28_09;
periodos = seconds(horarios(2:length(horarios))-horarios(1:length(horarios)-1));
periodos(2748) = periodos(2748)./2;
plot(1./periodos);
hold on;grid on;
plot(1./mean(periodos)*ones(length(periodos),1));
plot(1./median(periodos)*ones(length(periodos),1));
plot(1./max(periodos)*ones(length(periodos),1));
plot(1./min(periodos)*ones(length(periodos),1));
legend('frequencias', 'media', 'mediana', 'minima', 'maxima');

