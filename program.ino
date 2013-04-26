/*
  Birabot Program Syntax
  
  statement := ([1-9][0-9]{,2})([AB])([0-9]{,2})[#]
  statement[0] -> duration in minutes
  statement[1] -> sweep type (A-constant, B-linear)
  statement[2] -> temperature 
                  (for B sweeps, ending temperature)
  
  program := statement+
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



