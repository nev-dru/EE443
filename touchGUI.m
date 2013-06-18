function varargout = touchGUI(varargin)
% TOUCHGUI M-file for touchGUI.fig
%      TOUCHGUI, by itself, creates a new TOUCHGUI or raises the existing
%      singleton*.
%
%      H = TOUCHGUI returns the handle to a new TOUCHGUI or the handle to
%      the existing singleton*.
%
%      TOUCHGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in TOUCHGUI.M with the given input arguments.
%
%      TOUCHGUI('Property','Value',...) creates a new TOUCHGUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before touchGUI_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to touchGUI_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help touchGUI

% Last Modified by GUIDE v2.5 06-Jun-2013 21:20:55

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @touchGUI_OpeningFcn, ...
                   'gui_OutputFcn',  @touchGUI_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before touchGUI is made visible.
function touchGUI_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to touchGUI (see VARARGIN)
% -------------------------------------------------------------- RTDX SETUP
handles.cc = [];
ccsboardinfo
handles.cc = ccsdsp('boardnum',0);
r=handles.cc.rtdx;
visible(handles.cc,1);
handles.cc.reset
pause(1);
r.disable;
enable(r);
if ~isenabled(r)
    error('RTDX is not enabled')
end
r.set('timeout',20);
% open(handles.cc,'C:\CCStudio_v3.1\CD_Example\CD_Example\C6713\rtdx_matlabFIR\RTDX_MATLABFIR.pjt');
% load(handles.cc,'C:\CCStudio_v3.1\CD_Example\CD_Example\C6713\rtdx_matlabFIR\Debug\RTDX_MATLABFIR.out');
open(handles.cc,'C:\CCStudio_v3.1\CD_Example\CD_Example\C6713\MASTER\MASTER.pjt');
load(handles.cc,'C:\CCStudio_v3.1\CD_Example\CD_Example\C6713\MASTER\Debug\MASTER.out');
handles.cc.run;
r.open('ichan1','w');
r.open('ichan2','w');
r.open('ichan3','w');
r.enable('ichan1');
r.enable('ichan2');
r.enable('ichan3');
r.configure(1024,4,'continuous');
r.enable;

%------------------------------------------------ VARIABLES & ARDUINO SETUP
clc;
handles.numFunctions = 6;

% set limits of plot
xlim(handles.xy_axis,[10 990]);
ylim(handles.xy_axis,[10 990]);

% random color vector for axis
handles.bg_rgb = [rand rand rand];
set(handles.xy_axis, 'Color', [0 0 0], 'XTick', [], 'YTick', []);

% vector to store function button states
handles.buttons = zeros(1,handles.numFunctions);
handles.buttonHandles = [handles.tone, handles.tremolo, handles.gate, ...
    handles.autopan handles.delay handles.bpf];

% vector to store hold button states
handles.holdButtons = zeros(1,handles.numFunctions);
handles.holdButtonHandles = [handles.toneHold, handles.tremoloHold, handles.gateHold, ...
    handles.autopanHold handles.delayHold handles.bpfHold];

% vector to hold xy pairs when holding for each function
handles.holdPairs = zeros(handles.numFunctions, 2);
handles.markerSizes = [3:2:24, 24:-2:3];
handles.holdCounters = zeros(1,handles.numFunctions);
handles.holdColors = rand(handles.numFunctions, 3);

% setup serial connection to arduino
handles.s = serial('COM3', 'baudrate', 9600);
fopen(handles.s);
clear handles.data;
handles.i = 1;
handles.N = 25;
handles.x = zeros(1,handles.N);
handles.y = zeros(1,handles.N);

% Choose default command line output for touchGUI
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes touchGUI wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = touchGUI_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in exit.
function exit_Callback(hObject, eventdata, handles)
% hObject    handle to exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%----------------------------------------------------- EXIT BUTTON CALLBACK
fclose(handles.s)
disp('Serial connection to Arduino closed.');
close all;


% --- Executes on button press in run.
function run_Callback(hObject, eventdata, handles)
% hObject    handle to run (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of run
%------------------------------------------------------ RUN BUTTON CALLBACK

% reinitialize x and y vectors
handles.x = zeros(1,handles.N);
handles.y = zeros(1,handles.N);
clear handles.data;

% disable exit button
set(handles.exit,'enable','off','BackgroundColor',[0.18 0.18 0.18]);

% get ccs handle
handles.cc;
r=handles.cc.rtdx;    

% while run button is pressed...
while get(hObject,'Value')
    
    handles.data = fscanf(handles.s); % get string from arduino
    xystr = strsplit(handles.data,' '); % split string
    if (length(xystr) < 2)
        continue; % should never happen
    end
    handles.x(handles.i) = str2double(xystr(1)); % get x value
    handles.y(handles.i) = str2double(xystr(2)); % get y value

    % get value of function and hold buttons
    for b = 1:handles.numFunctions
        handles.buttons(b) = get(handles.buttonHandles(b), 'Value'); % get function button state
        handles.holdButtons(b) = get(handles.holdButtonHandles(b), 'Value'); % get hold button state
        if handles.buttons(b)
            % if function button state = ON
            set(handles.buttonHandles(b),'BackgroundColor',[.07 .2 .43]); % set color to blue
            if handles.holdButtons(b)
                % if hold button state = ON
                set(handles.holdButtonHandles(b),'BackgroundColor',handles.holdColors(b,:)); % set color to red
                handles.holdCounters(b) = handles.holdCounters(b) + 1;
                if ~handles.holdPairs(b,:)
                    % if hold values are empty, capture new values
                    handles.holdPairs(b,:) = [handles.x(handles.i), handles.y(handles.i)];
                    handles.holdCounters(b) = 0;
                end
            else
                set(handles.holdButtonHandles(b),'BackgroundColor',[.18 .18 .18]); % set color to grey
                handles.holdPairs(b,:) = [0, 0]; % set pair to zero
                handles.holdCounters(b) = 0;
            end    
        else
            % if function button state = OFF
            set(handles.buttonHandles(b),'BackgroundColor',[.18 .18 .18]); % set color to grey
            set(handles.holdButtonHandles(b),'BackgroundColor',[.18 .18 .18]); % set color to grey
            set(handles.holdButtonHandles(b),'Value',0.0); % turn off hold button
            handles.holdPairs(b,:) = [0, 0]; % set pair to zero
            handles.holdCounters(b) = 0;
        end
    end            

    % make plot
    plot(handles.x, handles.y, '.', ...
        'MarkerEdgeColor', 1-handles.bg_rgb, ...
        'MarkerFaceColor', 1-handles.bg_rgb, ...
        'MarkerSize',15, ...
        'parent', handles.xy_axis);
    hold on;
    for b = 1:handles.numFunctions
        plot(handles.holdPairs(b,1), handles.holdPairs(b,2), 'o', ...
            'MarkerEdgeColor', handles.holdColors(b,:), ...
            'MarkerFaceColor', 'None', ...
            'LineWidth', 5, ...
            'MarkerSize', handles.markerSizes(mod(handles.holdCounters(b),length(handles.markerSizes))+1), ...
            'parent', handles.xy_axis);
    end
    hold off;
    xlim(handles.xy_axis, [10 990]);
    ylim(handles.xy_axis, [10 990]);
    set(handles.xy_axis, 'Color', handles.bg_rgb,...
        'XTick',[], 'YTick', []);
    set(handles.functionPanel, 'HighlightColor', handles.bg_rgb);
    set(handles.runexitPanel, 'HighlightColor', handles.bg_rgb);
    drawnow;
    
    % increment colors for bg
    handles.bg_rgb = handles.bg_rgb + 0.005;
    for c = 1:3
        if handles.bg_rgb(c) >= 1
            handles.bg_rgb(c) = rand;
        end
    end
    
    % send x & y values over rtdx
    xy = [handles.x(handles.i), handles.y(handles.i)];
    if isenabled(r,'ichan1')
        writemsg(r,'ichan1',int16(xy))
    else
        error('Channel ''ichan1'' is not enabled')
    end
    
    % send function and hold button values over rtdx
    buttonValues = [handles.buttons, handles.holdButtons];
    if isenabled(r,'ichan2')
        writemsg(r,'ichan2',int16(buttonValues))
    else
        error('Channel ''ichan2'' is not enabled')
    end

    % send filter coefficients for BPF, if needed
    if handles.buttons(6)
        % compute coeffs here
        f = (handles.x(handles.i)-100)/(800);
        BW = (handles.y(handles.i)-100)/(800);
        Fs=44100;
        f0=floor(f*20000);       
        w0 = 2*pi*f0/Fs;
        alpha = sin(w0)*sinh( log(2)/2 * BW * w0/sin(w0));
        b0 =   alpha;
        b1 =   0;
        b2 =  -alpha;
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha; 

        cxn=b0/a0;
        cxn1=b1/a0;
        cxn2=b2/a0;
        cyn1=a1/a0;
        cyn2=a2/a0;
        
        % k = handles.i;
        out = [cxn cxn1 cxn2 cyn1 cyn2];
        if isenabled(r, 'ichan3')
            writemsg(r,'ichan3', single(out));
        else
            error('Channel ''ichan3'' is not enabled')
        end
    end
    
    % erase tail end of plot as it goes
    j = mod(handles.i, handles.N) + 1;
    handles.x(j) = 0;
    handles.y(j) = 0;
    
    % increment i index
    handles.i = handles.i + 1;
    if (handles.i > handles.N)
        handles.i = 1;
    end
end

% reset plot and colors
handles.bg_rgb = [rand rand rand];
plot(0,0,'parent',handles.xy_axis);
set(handles.xy_axis, 'Color', [0 0 0], 'XTick',[], 'YTick', []);
set(handles.functionPanel, 'HighlightColor', [.5 .5 .5]);
set(handles.runexitPanel, 'HighlightColor', [.5 .5 .5]);
handles.holdColors = rand(handles.numFunctions, 3);

% reset buttons
for b = 1:handles.numFunctions
    set(handles.buttonHandles(b),'BackgroundColor',[.18 .18 .18]);
    set(handles.buttonHandles(b),'Value',0);
    set(handles.holdButtonHandles(b),'BackgroundColor',[.18 .18 .18]);
    set(handles.holdButtonHandles(b),'Value',0);
end

% enable exit button
set(handles.exit, 'BackgroundColor', [.5 0 0], 'enable', 'on');

guidata(hObject,handles);
    




% --- Executes on button press in tone.
function tone_Callback(hObject, eventdata, handles)
% hObject    handle to tone (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tone
% handles.tone = get(hObject,'Value');
guidata(hObject,handles);

% --- Executes on button press in tremolo.
function tremolo_Callback(hObject, eventdata, handles)
% hObject    handle to tremolo (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tremolo
% handles.tremolo = get(hObject,'Value');
guidata(hObject,handles);

% --- Executes on button press in gate.
function gate_Callback(hObject, eventdata, handles)
% hObject    handle to gate (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of gate
% handles.gate = get(hObject,'Value');
guidata(hObject,handles);

% --- Executes on button press in autopan.
function autopan_Callback(hObject, eventdata, handles)
% hObject    handle to autopan (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of autopan
% handles.autopan = get(hObject,'Value');
guidata(hObject,handles);

% --- Executes on button press in delay.
function delay_Callback(hObject, eventdata, handles)
% hObject    handle to delay (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of delay
handles.delay = get(hObject,'Value');
guidata(hObject,handles);

% --- Executes on button press in bpf.
function bpf_Callback(hObject, eventdata, handles)
% hObject    handle to bpf (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of bpf
% handles.bpf = get(hObject,'Value');
guidata(hObject,handles);



% --- Executes on button press in toneHold.
function toneHold_Callback(hObject, eventdata, handles)
% hObject    handle to toneHold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toneHold


% --- Executes on button press in autopanHold.
function autopanHold_Callback(hObject, eventdata, handles)
% hObject    handle to autopanHold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of autopanHold


% --- Executes on button press in tremoloHold.
function tremoloHold_Callback(hObject, eventdata, handles)
% hObject    handle to tremoloHold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tremoloHold


% --- Executes on button press in delayHold.
function delayHold_Callback(hObject, eventdata, handles)
% hObject    handle to delayHold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of delayHold


% --- Executes on button press in gateHold.
function gateHold_Callback(hObject, eventdata, handles)
% hObject    handle to gateHold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of gateHold


% --- Executes on button press in bpfHold.
function bpfHold_Callback(hObject, eventdata, handles)
% hObject    handle to bpfHold (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of bpfHold


