function out_dat=flight_convert(in_name,out_name)

    f=fopen(in_name,'rt');


    if(~is_valid_file_id(f))
        error('Faied to open input file');
    end

    %read header line
    line=fgetl(f);

    %first line should be #
    if(line(1)~='#')
        fclose(f);
        error('Could not find header line');
    end

    %get column names from header names
    columns=strsplit(line(1:end),',');

    %use %s for these columns
    string_col_names={'call','state_name'};

    %preallocate logical array of columns that are strings
    string_cols=zeros(size(columns));

    for k=1:length(string_col_names)
        string_cols=string_cols|strcmp(columns,string_col_names{k});
    end

    %column format specifier
    format=cell(size(columns));

    format(:)='%f';

    format(string_cols)='%s';

    format=strcat(format{:});
    
    %read data from file
    dat=textscan(f,format,'Delimiter',',');

    %close file
    fclose(f);

    %columns to use in output and scale factor
    output_cols={'state','tick','serial','flight','acceleration','baro_speed','height','motor_number'};
    output_scales=[1    ,     1,       1,       1,            16,          16,       1,             1];

    %name of time column
    time_name='time';

    %get index of time column
    time_col=strcmp(columns,time_name);

    t_num=nnz(time_col);

    if(t_num==0)
        error('Time column not found');
    elseif(t_num>1)
        error('Multiple time columns found');
    end

    %get time data
    time=dat{time_col};

    %only sample data every second
    sample_times=floor(min(time)):ceil(max(time));

    %preallocate for output
    out_dat=zeros(length(sample_times),length(output_cols));

    for k=1:length(output_cols)
        %find matching column
        col_idx=strcmp(columns,output_cols{k});
        %see if match was found
        if(any(col_idx))
            out_dat(:,k)=interp1(time,dat{col_idx},sample_times);
            %check if the first point is NAN
            if(isnan(out_dat(1,k)))
                %fill in start points with first value
                out_dat(1,k)=dat{col_idx}(1);
            end
            %check if the last point is NAN
            if(isnan(out_dat(end,k)))
                %fill in end points with first value
                out_dat(end,k)=dat{col_idx}(end);
            end
            %scale to companion value and round
            out_dat(:,k)=round(out_dat(:,k)*output_scales(k));
        %check for tick column
        elseif(strcmp('tick',output_cols{k}))
            %tick rate is 100 hz
            tick_rate=100;
            %generate tick vector add a few minuts so it's not negative
            out_dat(:,k)=uint16(round(tick_rate*(500+sample_times)));
        %check for motor number column
        elseif(strcmp('motor_number',output_cols{k}))
           %assume that there is only one motor fired 
           out_dat(:,k)=1;
        else
            error('No column matching ''%s''\n',output_cols{k});
        end
    end

    %open output file
    f=fopen(out_name,'wt');

    if(is_valid_file_id(f))
        %print file header
        fprintf(f,['//This file is auto generated from flight data\n\n',...
                   '#include "Companion.h"\n',...
                   '#include "flightPattern.h"\n\n']);
        %data declaration
        fprintf(f,'const struct ao_companion_command flight_dat[]={\n');
        %print data, command is added as it is fixed
        fprintf(f,'\t\t{AO_COMPANION_NOTIFY,%i,%i,%i,%i,%i,%i,%i,%i},\n',out_dat')
        %write a terminator line with zeros
        fprintf(f,'\t\t{0,%i,%i,%i,%i,%i,%i,%i,%i},\n',zeros(1,size(out_dat,2)))
        %close data structure
        fprintf(f,'\t};\n\n');

        %close file
        fclose(f);
    else
        error('Faied to open output file');
    end


end


