/*
  Birabot Program Syntax
  
  statement := ([1-9][0-9]{,2})([AB])([0-9]{,2})[#]
  statement[0] -> duration in minutes
  statement[1] -> sweep type (A-constant, B-linear)
  statement[2] -> temperature 
                  (for B sweeps, ending temperature)
  
  program := statement+
  
  example:
    30A50#15B80#25A99#
    
     0 -> 50 // 30A50#
     1 -> 50
    ...
    29 -> 50 
    30 -> 52 // 15B80#
    31 -> 54
    ...
    43 -> 78
    44 -> 80
    45 -> 99 // 25B99#
    46 -> 99
    ...
    68 -> 99
    69 -> 99
    70 ->  0 // program end
    71 ->  0
*/

struct statement {
  byte duration;
  boolean constant:1;
  byte temperature:7;
};

struct program {
  statement statements[];
  byte num_statements;
  char name[9];
};

int program_duration(struct program *p) {
  int duration = 0;
  for (int i=0; i<p->num_statements; i++) {
    duration += p->statements[i].duration;
  }
  return duration;
}

char* program_name(struct program *p) {
  return p->name;
}

byte temperature_at(struct program *p, int time) {
  for (int i=0, t=0; i<p->num_statements; i++) {
    if (time < t + p->statements[i].duration) {
      return p->statements[i].temperature;
    }
  }
  return 0;
}



