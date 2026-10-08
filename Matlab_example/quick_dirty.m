clear
clc

%load moment arm data inside functions
tmp = mfilename;
tmp = erase(which(tmp),[tmp '.m']);
addpath([tmp '/Functions']);
%%


 [fname,fpath] = (uigetfile('*.xlsx; *.csv'));


 %
 data = readtable([fpath fname]);

 %col 1 = angle (indepent var)
 %col 2 = rest Votlage (torque)
 %col 3 = peak voltage (torque)

 %Would be good to make a table with 3 columns
 % where the use insert the values of the tested angles
 % then the peak to peak is shown in column 4 (not interactive)
 % and on the right there is a panel with the points plotted (real data)
 % and the fitted line (3rd polynomial)
 %get data in array
 Angle = data.Angle;
 activeT = data.Peak_V_-data.Rest_V_;


 %normalize 0-1
 activeT = activeT./max(activeT);
 polyp = polyfit(Angle,activeT,3); %get polyfit

 % create a "continue function" for checking

  x = floor(min(Angle)*5)/5 : 1 :  ceil(max(Angle)*5)/5; %round to 5

 out = polyval(polyp,x);

 %show the output of the function for visual inspection
 plot(Angle,activeT,'o');
 hold on
 plot(x,out,'-')


 %% Now normalize to peak

 %outNorm = out./max(out);
% plot over angle
%  plot(x,outNorm,'-')
% now simply ask what force % the person (Andrew) needs to match and return
% the corresponding angle value to reach that %


%% Find angle(s) for a target force percentage (e.g., 75%)
target_pct = 0.75; % 75% target

% 1. Compute target value in raw units (matching polyp scale)
max_val = max(out); 
y_target = target_pct * max_val;

% 2. Shift polynomial down: P(x) - y_target = 0
polyp_shifted = polyp;
polyp_shifted(end) = polyp_shifted(end) - y_target;

% 3. Find all roots (candidate angles)
all_roots = roots(polyp_shifted);

% 4. Filter out complex numbers and keep roots inside the tested range
real_roots = all_roots(imag(all_roots) == 0);
valid_angles = real_roots(real_roots >= min(Angle) & real_roots <= max(Angle));

% Display results
fprintf('Angle(s) at %.0f%% force:\n', target_pct * 100);
disp(valid_angles);

% Plot target line and identified angle(s) on the curve
plot([min(x) max(x)], [target_pct target_pct], 'r--');
plot(valid_angles, (polyval(polyp, valid_angles) / max_val), 'ro', 'MarkerFaceColor', 'r');