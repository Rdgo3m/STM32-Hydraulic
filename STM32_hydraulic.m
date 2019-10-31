delete(instrfind({'Port'},{'COM7'}));
arm = serial('COM7'); % comunicação serial via COM7
arm.BaudRate = 115200; % baud rate
warning('off', 'MATLAB:serial:fscanf:unsuccessfulRead');
fopen(arm);
fgets(arm);
ts = 1; % periodo de amostragem
tempo = input('Digite por quantos segundos serao obtidas amostras: ');
total_amostras = tempo / ts;
tplot = (0:1:tempo);
for i = 1:total_amostras
amostras(i)=fscanf(arm,'%f');
fprintf('vazão = %-3.2fL/min\n', amostras(i));
stem(amostras)
drawnow
end
figure(2)
plot(amostras,'b')
legend('Vazao moto-bomba')
xlabel('Amostras')
ylabel('Vazao [L/min]')
fclose(arm);
disp('Fim da amostragem.');
disp('Pressione qualquer tecla para encerrar o programa.');
pause(0.1);
delete(arm);
clear all;