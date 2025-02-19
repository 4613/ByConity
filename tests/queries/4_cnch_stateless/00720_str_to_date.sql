SELECT STR_TO_DATE('2023-01-01 12:00:00', '%Y-%m-%d %H:%i:%s'); 
SELECT STR_TO_DATE('2024-02-29 12:00:00', '%Y-%m-%d %H:%i:%s'); 
SELECT STR_TO_DATE('2023-02-29 12:00:00', '%Y-%m-%d %H:%i:%s');
SELECT STR_TO_DATE('2023-01-01', '%Y-%m-%d');
SELECT STR_TO_DATE('12:00:00', '%H:%i:%s');
SELECT STR_TO_DATE('2023-02-30 12:00:00', '%Y-%m-%d %H:%i:%s');
SELECT STR_TO_DATE('2023-01-01 24:00:00', '%Y-%m-%d %H:%i:%s');
SELECT STR_TO_DATE('2023-01-01 12:00 PM', '%Y-%m-%d %r');
set dialect_type='MYSQL';
SELECT STR_TO_DATE('2023-01-01 12:00:00 PM', '%Y-%m-%d %r');
SELECT STR_TO_DATE('01-01-2023', '%d-%m-%Y');
set formatdatetime_parsedatetime_m_is_month_name = 1;
SELECT STR_TO_DATE('01-January-2023', '%d-%M-%Y');
SELECT STR_TO_DATE('01-Jan-2023', '%d-%b-%Y');
SELECT STR_TO_DATE('01/01/2023', '%d-%m-%Y');
SELECT STR_TO_DATE('2023-01-', '%Y-%m-%d');
SELECT STR_TO_DATE('2023-01-01-01', '%Y-%m-%d');
SELECT STR_TO_DATE('2023-01-01', '%d-%m-%Y');

-- year
select parseDateTime('2020', '%Y', 'UTC') = toDateTime('2020-01-01', 'UTC');

-- month
select parseDateTime('02', '%m', 'UTC') = toDateTime('2000-02-01', 'UTC');
select parseDateTime('07', '%m', 'UTC') = toDateTime('2000-07-01', 'UTC');
select parseDateTime('11-', '%m-', 'UTC') = toDateTime('2000-11-01', 'UTC');
select parseDateTime('02', '%c', 'UTC') = toDateTime('2000-02-01', 'UTC');
select parseDateTime('07', '%c', 'UTC') = toDateTime('2000-07-01', 'UTC');
select parseDateTime('11-', '%c-', 'UTC') = toDateTime('2000-11-01', 'UTC');
select parseDateTime('jun', '%b', 'UTC') = toDateTime('2000-06-01', 'UTC');
select parseDateTime('JUN', '%b', 'UTC') = toDateTime('2000-06-01', 'UTC');
set formatdatetime_parsedatetime_m_is_month_name = 1;
select parseDateTime('may', '%M', 'UTC') = toDateTime('2000-05-01', 'UTC');
select parseDateTime('MAY', '%M', 'UTC') = toDateTime('2000-05-01', 'UTC');
select parseDateTime('september', '%M', 'UTC') = toDateTime('2000-09-01', 'UTC');
set formatdatetime_parsedatetime_m_is_month_name = 0;
select parseDateTime('08', '%M', 'UTC') = toDateTime('1970-01-01 00:08:00', 'UTC');
select parseDateTime('59', '%M', 'UTC') = toDateTime('1970-01-01 00:59:00', 'UTC');
select parseDateTime('00/', '%M/', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
set formatdatetime_parsedatetime_m_is_month_name = 1;

-- day of month
select parseDateTime('07', '%d', 'UTC') = toDateTime('2000-01-07', 'UTC');
select parseDateTime('01', '%d', 'UTC') = toDateTime('2000-01-01', 'UTC');
select parseDateTime('/11', '/%d', 'UTC') = toDateTime('2000-01-11', 'UTC');

select parseDateTime('01 31 20 02', '%m %d %d %m', 'UTC') = toDateTime('2000-02-20', 'UTC');
select parseDateTime('02 31 20 04', '%m %d %d %m', 'UTC') = toDateTime('2000-04-20', 'UTC');
select parseDateTime('02 31 01', '%m %d %m', 'UTC') = toDateTime('2000-01-31', 'UTC');
select parseDateTime('2000-02-29', '%Y-%m-%d', 'UTC') = toDateTime('2000-02-29', 'UTC');

-- day of year
select parseDateTime('001', '%j', 'UTC') = toDateTime('2000-01-01', 'UTC');
select parseDateTime('007', '%j', 'UTC') = toDateTime('2000-01-07', 'UTC');
select parseDateTime('/031/', '/%j/', 'UTC') = toDateTime('2000-01-31', 'UTC');
select parseDateTime('032', '%j', 'UTC') = toDateTime('2000-02-01', 'UTC');
select parseDateTime('060', '%j', 'UTC') = toDateTime('2000-02-29', 'UTC');
select parseDateTime('365', '%j', 'UTC') = toDateTime('2000-12-30', 'UTC');
select parseDateTime('366', '%j', 'UTC') = toDateTime('2000-12-31', 'UTC');
select parseDateTime('1980 001', '%Y %j', 'UTC') = toDateTime('1980-01-01', 'UTC');
select parseDateTime('1980 007', '%Y %j', 'UTC') = toDateTime('1980-01-07', 'UTC');
select parseDateTime('1980 /007', '%Y /%j', 'UTC') = toDateTime('1980-01-07', 'UTC');
select parseDateTime('1980 /031/', '%Y /%j/', 'UTC') = toDateTime('1980-01-31', 'UTC');
select parseDateTime('1980 032', '%Y %j', 'UTC') = toDateTime('1980-02-01', 'UTC');
select parseDateTime('1980 060', '%Y %j', 'UTC') = toDateTime('1980-02-29', 'UTC');
select parseDateTime('1980 366', '%Y %j', 'UTC') = toDateTime('1980-12-31', 'UTC');

select parseDateTime('2001 366 2000', '%Y %j %Y', 'UTC') = toDateTime('2000-12-31', 'UTC');

-- hour of day
select parseDateTime('07', '%H', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('23', '%H', 'UTC') = toDateTime('1970-01-01 23:00:00', 'UTC');
select parseDateTime('00', '%H', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('10', '%H', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');

select parseDateTime('07', '%k', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('23', '%k', 'UTC') = toDateTime('1970-01-01 23:00:00', 'UTC');
select parseDateTime('00', '%k', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('10', '%k', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');


-- hour of half day
select parseDateTime('07', '%h', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('12', '%h', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01', '%h', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('10', '%h', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');

select parseDateTime('07', '%I', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('12', '%I', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01', '%I', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('10', '%I', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');

select parseDateTime('07', '%l', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('12', '%l', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01', '%l', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('10', '%l', 'UTC') = toDateTime('1970-01-01 10:00:00', 'UTC');

-- half of day
select parseDateTime('07 PM', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('07 AM', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('07 pm', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('07 am', '%H %p', 'UTC') = toDateTime('1970-01-01 07:00:00', 'UTC');
select parseDateTime('00 AM', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('00 PM', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('00 am', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('00 pm', '%H %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('01 PM', '%h %p', 'UTC') = toDateTime('1970-01-01 13:00:00', 'UTC');
select parseDateTime('01 AM', '%h %p', 'UTC') = toDateTime('1970-01-01 01:00:00', 'UTC');
select parseDateTime('06 PM', '%h %p', 'UTC') = toDateTime('1970-01-01 18:00:00', 'UTC');
select parseDateTime('06 AM', '%h %p', 'UTC') = toDateTime('1970-01-01 06:00:00', 'UTC');
select parseDateTime('12 PM', '%h %p', 'UTC') = toDateTime('1970-01-01 12:00:00', 'UTC');
select parseDateTime('12 AM', '%h %p', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');

-- minute
select parseDateTime('08', '%i', 'UTC') = toDateTime('1970-01-01 00:08:00', 'UTC');
select parseDateTime('59', '%i', 'UTC') = toDateTime('1970-01-01 00:59:00', 'UTC');
select parseDateTime('00/', '%i/', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');


-- second
select parseDateTime('09', '%s', 'UTC') = toDateTime('1970-01-01 00:00:09', 'UTC');
select parseDateTime('58', '%s', 'UTC') = toDateTime('1970-01-01 00:00:58', 'UTC');
select parseDateTime('00/', '%s/', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');


-- microsecond
select parseDateTime('000000', '%f', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTime('456789', '%f', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');


-- mixed YMD format
select parseDateTime('2021-01-04+23:00:00.654321', '%Y-%m-%d+%H:%i:%s.%f', 'UTC') = toDateTime('2021-01-04 23:00:00', 'UTC');
select parseDateTime('2019-07-03 11:04:10.975319', '%Y-%m-%d %H:%i:%s.%f', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select parseDateTime('10:04:11 03-07-2019.242424', '%s:%i:%H %d-%m-%Y.%f', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');

-- *OrZero, *OrNull, str_to_date
select parseDateTimeOrZero('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select parseDateTimeOrZero('10:04:11 invalid 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('1970-01-01 00:00:00', 'UTC');
select parseDateTimeOrNull('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select parseDateTimeOrNull('10:04:11 invalid 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') IS NULL;
select str_to_date('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select sTr_To_DaTe('10:04:11 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') = toDateTime('2019-07-03 11:04:10', 'UTC');
select str_to_date('10:04:11 invalid 03-07-2019', '%s:%i:%H %d-%m-%Y', 'UTC') IS NULL;

