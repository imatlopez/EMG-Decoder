clear; global s

dt = 1/1e4;
f  = 100;

s = daq.createSession('ni');
s.addAnalogInputChannel('Dev2', [0 1 2 3], 'Voltage');
s.Channels(1).Range = [-5 5];
s.addAnalogOutputChannel('Dev2',0,'Voltage');
s.Rate = 1/dt;

A = logspace(-3,0,200)';

raw = cell(length(A),3);

if s.IsLogging; error('DAQ is running'); end

for i = 1:length(A)
    raw{i,1} = A(i);
    t = (0:dt:(0.2-dt))';
    s.queueOutputData(A(i)*sin(2*pi*f*t));
    [raw{i,3}, raw{i,2}] = s.startForeground;
    if mod(i,round(length(A)/10))==0; fprintf('%0.4f V\n',A(i)); end
end

clear s i t f A

save(['amp' datestr(now,'-mmdd-hhMMss')])