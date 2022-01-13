/* File : example.i */
%module flexric
%{
/* Put headers and other declarations here */
extern void init(void);
extern void stop(void);
extern void report_service(int);
extern void remove_service(int);
extern void control_service(int);
extern void load_sm(const char*);
%}

extern void init(void);
extern void stop(void);
extern void report_service(int);
extern void remove_service(int);
extern void control_service(int);
extern void load_sm(const char*);
